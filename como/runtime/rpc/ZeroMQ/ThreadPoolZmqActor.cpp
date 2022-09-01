//=========================================================================
// Copyright (C) 2021 The C++ Component Model(COMO) Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//=========================================================================

#include <unistd.h>
#include <assert.h>
#include <cerrno>
#include <csignal>
#include <pthread.h>
#include "mutex.h"
#include "zmq.h"
#include "util/comolog.h"
#include "ComoConfig.h"
#include "registry.h"
#include "ThreadPoolZmqActor.h"
#include "CZMQUtils.h"
#include "CZMQParcel.h"
#include "RuntimeMonitor.h"

namespace como {

static struct timespec lastCheckConnExpireTime = {0, 0};
static bool LivingWorker(TPZA_Executor::Worker *worker);

//-------------------------------------------------------------------------
// TPZA : ThreadPoolZmqActor
TPZA_Executor::Worker::Worker(CZMQChannel *channel, AutoPtr<IStub> stub,
                                                          std::string& endpoint)
    : mStub(stub)
    , mWorkerStatus(WORKER_TASK_READY)
    , mEndpoint(endpoint)
    , state(0)
{
    clock_gettime(CLOCK_REALTIME, &lastAccessTime);
    channel->GetServerObjectId(mChannel);
}

TPZA_Executor::Worker::~Worker()
{
/**
 * The ZeroMQ socket managed by ZeroMQ/zmq_sockets are always open. In order to
 * improve the efficiency of use, always maintain the connection with all servers.
 */
}

static Integer SendECode(HANDLE hChannel, void *socket, ECode ec)
{
    return CZMQUtils::CzmqSendBuf(hChannel, ec, socket,
                                              (const void *)&ec, sizeof(ECode));
}

void *ThreadPoolZmqActor::threadHandleMessage(void *threadData)
{
    void     *socket;
    HANDLE    hChannel;
    Integer   eventCode;
    zmq_msg_t msg;
    ECode     ec;
    HANDLE    resData;
    Long      resSize;
    Integer   iRet;
    TPZA_Executor::Worker *worker;

    socket = CZMQUtils::CzmqGetPollitemsSocket((int)(Long)threadData);
    if (nullptr == socket) {
        Logger::E("threadHandleMessage", "socket is nullptr");
        return nullptr;
    }

  while (true) {
    iRet = CZMQUtils::CzmqRecvMsg(hChannel, eventCode, socket, msg, 0);

    if (iRet > 0) {
        switch (eventCode) {
            case ZmqFunCode::Method_Invoke: {
                AutoPtr<IParcel> resParcel;

                worker = PickWorkerByChannelHandle(hChannel, true);
                if (nullptr == worker) {
                    zmq_msg_close(&msg);
                    Logger::E("threadHandleMessage",
                                            "Method_Invoke, Bad channel value");
                    SendECode(0, socket, ZMQ_BAD_PACKET);
                    break;
                }

                AutoPtr<IParcel> argParcel = new CZMQParcel();
                if (nullptr == argParcel) {
                    Logger::E("threadHandleMessage",
                                "Method_Invoke, new CZMQParcel return nullptr");
                    resData = reinterpret_cast<HANDLE>((char*)"");
                    resSize = 1;
                    ec = E_OUT_OF_MEMORY_ERROR;
                    goto HandleMessage_Method_Invoke;
                }

                ec = argParcel->SetData(reinterpret_cast<HANDLE>(
                                       zmq_msg_data(&msg)), zmq_msg_size(&msg));
                if (FAILED(ec)) {
                    Logger::E("threadHandleMessage",
                                     "Method_Invoke, argParcel->SetData error");
                    resData = reinterpret_cast<HANDLE>((char*)"");
                    resSize = 1;
                    goto HandleMessage_Method_Invoke;
                }

                resParcel = new CZMQParcel();
                if (nullptr == resParcel) {
                    Logger::E("threadHandleMessage",
                                "Method_Invoke, new CZMQParcel return nullptr");
                    resData = reinterpret_cast<HANDLE>((char*)"");
                    resSize = 1;
                    ec = E_OUT_OF_MEMORY_ERROR;
                    goto HandleMessage_Method_Invoke;
                }

                ec = worker->mStub->Invoke(argParcel, resParcel);

                // It shouldn't be WORKER_TASK_FINISH.
                // A request ends, but the channel still needs to be.
                worker->mWorkerStatus = WORKER_TASK_DAEMON_RUNNING;

                // Even if worker->mStub->Invoke fails, the following two
                // behaviors of taking the return value of Invoke work normally.
                resParcel->GetData(resData);
                resParcel->GetDataSize(resSize);

HandleMessage_Method_Invoke:
                zmq_msg_close(&msg);
                CZMQUtils::CzmqSendBuf(worker->mChannel, ec,
                                        socket, (const void *)resData, resSize);

                // `ReleaseWorker`, This Worker is a daemon
                clock_gettime(CLOCK_REALTIME, &(worker->lastAccessTime));
                worker->mWorkerStatus = WORKER_TASK_DAEMON_RUNNING;
                break;
            }

            case ZmqFunCode::GetComponentMetadata: {
                Array<Byte> metadata;
                AutoPtr<IMetaComponent> mc;

                worker = PickWorkerByChannelHandle(hChannel, true);
                if (nullptr == worker) {
                    zmq_msg_close(&msg);
                    Logger::E("threadHandleMessage",
                                     "GetComponentMetadata, Bad channel value");
                    SendECode(0, socket, E_NOT_FOUND_EXCEPTION);
                    break;
                }

                AutoPtr<IParcel> argParcel = new CZMQParcel();
                if (nullptr == argParcel) {
                    Logger::E("threadHandleMessage",
                         "GetComponentMetadata, new CZMQParcel return nullptr");
                    // `ReleaseWorker`, This Worker is a daemon
                    worker->mWorkerStatus = WORKER_TASK_DAEMON_RUNNING;
                    ec = E_OUT_OF_MEMORY_ERROR;
                    goto HandleMessage_GetComponentMetadata;
                }

                ec = argParcel->SetData(reinterpret_cast<HANDLE>(
                                       zmq_msg_data(&msg)), zmq_msg_size(&msg));

                if (FAILED(ec)) {
                    Logger::E("threadHandleMessage",
                              "GetComponentMetadata, argParcel->SetData error");
                    goto HandleMessage_GetComponentMetadata;
                }

                CoclassID cid;
                argParcel->ReadCoclassID(cid);
                ec = CoGetComponentMetadata(*cid.mCid, nullptr, mc);
                if (FAILED(ec)) {
                    Logger::E("threadHandleMessage",
                               "CoGetComponentMetadata error, ECode: 0x%X", ec);
                    ReleaseCoclassID(cid);
                    worker->mWorkerStatus = WORKER_TASK_DAEMON_RUNNING;
                    goto HandleMessage_GetComponentMetadata;
                }

                ec = mc->GetSerializedMetadata(metadata);

                ReleaseCoclassID(cid);
                if (SUCCEEDED(ec)) {
                    CZMQUtils::CzmqSendBuf(worker->mChannel, ec,
                           socket, metadata.GetPayload(), metadata.GetLength());
                }
                else {
                    CZMQUtils::CzmqSendBuf(worker->mChannel, ec,
                                                          socket, (char*)"", 1);
                }

                // `ReleaseWorker`, This Worker is a daemon
                worker->mWorkerStatus = WORKER_TASK_DAEMON_RUNNING;
                clock_gettime(CLOCK_REALTIME, &(worker->lastAccessTime));
                break;

HandleMessage_GetComponentMetadata:
                zmq_msg_close(&msg);
                CZMQUtils::CzmqSendBuf(worker->mChannel, ec,
                                                          socket, (char*)"", 1);

                // `ReleaseWorker`, This Worker is a daemon
                worker->mWorkerStatus = WORKER_TASK_DAEMON_RUNNING;
                break;
            }

            case ZmqFunCode::Actor_IsPeerAlive: {
                #pragma pack(1)
                struct PeerState {
                    Long state;
                    Boolean alive;
                };
                #pragma pack()
                PeerState peerState;
                peerState.alive = true;
                peerState.state = worker->state;

                worker = PickWorkerByChannelHandle(hChannel, true);
                if ((nullptr == worker) || (zmq_msg_size(&msg) < sizeof(Long))) {
                    peerState.alive = false;
                }

                CZMQUtils::CzmqSendBuf(hChannel, NOERROR, socket,
                                   (const void *)&peerState, sizeof(peerState));

                if (nullptr != worker) {
                    // Remember the Identifier (a Long type data) passed in the
                    // client request
                    worker->state = *(Long*)zmq_msg_data(&msg);

                    worker->mWorkerStatus = WORKER_TASK_DAEMON_RUNNING;
                    clock_gettime(CLOCK_REALTIME, &(worker->lastAccessTime));
                }

                zmq_msg_close(&msg);

                break;
            }

            case ZmqFunCode::Object_Release: {
                worker = PickWorkerByChannelHandle(hChannel, true);
                if ((nullptr == worker) || (zmq_msg_size(&msg) < sizeof(Long))) {
                    Logger::E("threadHandleMessage",
                                            "Object_Release, Bad channel value");
                    ec = ZMQ_BAD_PACKET;
                    goto HandleMessage_Object_Release;
                }

                Long hash = *(Long*)zmq_msg_data(&msg);

                // At this time, the Channel object may be released itself.
                if (0 != hash) {
                    ec = UnregisterExportObjectByHash(RPCType::Remote, hash);
                    if (FAILED(ec)) {
                        Logger::E("threadHandleMessage",
                                       "Object_Release error, ECode: 0x%X", ec);
                    }
                }

                // `ReleaseWorker` will NOT delete this work
                worker->mWorkerStatus = WORKER_TASK_DAEMON_RUNNING;
                clock_gettime(CLOCK_REALTIME, &(worker->lastAccessTime));

HandleMessage_Object_Release:
                zmq_msg_close(&msg);
                SendECode(hChannel, socket, ec);

                break;
            }

            case ZmqFunCode::ReleasePeer: {

                // `ReleaseWorker` will DELETE this work
                // PickWorkerByChannelHandle(hChannel, false); The 'false' here
                // means it is not a daemon, life ends here.

                worker = PickWorkerByChannelHandle(hChannel, false);
                if ((nullptr == worker) || (zmq_msg_size(&msg) < sizeof(Long))) {
                    Logger::E("threadHandleMessage",
                                                "ReleasePeer, Bad channel value");
                    ec = ZMQ_BAD_PACKET;
                    goto HandleMessage_Object_ReleasePeer;
                }

                ec = UnregisterExportObjectByChannel(RPCType::Remote, hChannel);
                if (FAILED(ec)) {
                    Logger::E("threadHandleMessage",
                                          "ReleasePeer error, ECode: 0x%X", ec);
                }

HandleMessage_Object_ReleasePeer:
                zmq_msg_close(&msg);
                SendECode(hChannel, socket, ec);

                break;
            }

            case ZmqFunCode::RuntimeMonitor: {
                Array<Byte> resBuffer;
                RTM_Command *rtmCommand = (RTM_Command *)zmq_msg_data(&msg);

                worker = PickWorkerByChannelHandle(hChannel, true);
                if ((nullptr == worker) || (zmq_msg_size(&msg) < 1)) {
                    Logger::E("threadHandleMessage",
                                           "RuntimeMonitor, Bad channel value");
                    ec = ZMQ_BAD_PACKET;
                    goto HandleMessage_RuntimeMonitor;
                }

                switch (rtmCommand->command) {
                    case RTM_CommandType::CMD_Server_Activate_InvokeMethod: {
                        CStub::From(worker->mStub)->mMonitorInvokeMethod = true;
                        goto HandleMessage_RuntimeMonitor;
                    }

                    case RTM_CommandType::CMD_Server_Deactivate_InvokeMethod: {
                        CStub::From(worker->mStub)->mMonitorInvokeMethod = false;
                        goto HandleMessage_RuntimeMonitor;
                    }
                }

                ec = RuntimeMonitor::RuntimeMonitorMsgProcessor(msg, resBuffer);
                if (SUCCEEDED(ec)) {
                    if (nullptr == resBuffer.GetPayload()) {
                        ec = E_OUT_OF_MEMORY_ERROR;
                        goto HandleMessage_RuntimeMonitor;
                    }

                    resData = reinterpret_cast<HANDLE>(resBuffer.GetPayload());
                    resSize = resBuffer.GetLength();
                    CZMQUtils::CzmqSendBuf(worker->mChannel, ec,
                                        socket, (const void *)resData, resSize);
                    zmq_msg_close(&msg);
                    // `ReleaseWorker` will NOT delete this work
                    worker->mWorkerStatus = WORKER_TASK_DAEMON_RUNNING;
                    break;
                }

HandleMessage_RuntimeMonitor:
                zmq_msg_close(&msg);
                SendECode(worker->mChannel, socket, ec);

                // `ReleaseWorker` will NOT delete this work
                worker->mWorkerStatus = WORKER_TASK_DAEMON_RUNNING;

                break;
            }

            default:
                worker = PickWorkerByChannelHandle(hChannel, true);
                if (nullptr == worker) {
                    SendECode(0, socket, ZMQ_BAD_PACKET);
                    goto HandleMessage_Default;
                }

                if (nullptr != TPZA_Executor::defaultHandleMessage) {
                    if (TPZA_Executor::defaultHandleMessage(
                                   reinterpret_cast<HANDLE>(nullptr), eventCode,
                                                            socket, msg) != 0) {
                        Logger::E("threadHandleMessage",
                                                  "defaultHandleMessage error");
                    }
                    else {
                        // defaultHandleMessage will call CzmqSendBuf
                        worker->mWorkerStatus = WORKER_TASK_DAEMON_RUNNING;
                        goto HandleMessage_Default;
                    }
                }
                SendECode(worker->mChannel, socket, NOERROR);
HandleMessage_Default:
                zmq_msg_close(&msg);
        }
    }
    else if (iRet < 0) {
        if (iRet < -1)
            zmq_msg_close(&msg);

        SendECode(hChannel, socket, NOERROR);
    }
  } // while (true)
}

//-------------------------------------------------------------------------

AutoPtr<TPZA_Executor> TPZA_Executor::sInstance = nullptr;
Mutex TPZA_Executor::sInstanceLock;
AutoPtr<ThreadPoolZmqActor> TPZA_Executor::threadPool = nullptr;
HANDLE_MESSAGE_FUNCTION TPZA_Executor::defaultHandleMessage = nullptr;

AutoPtr<TPZA_Executor> TPZA_Executor::GetInstance()
{
    Mutex::AutoLock lock(sInstanceLock);

    if (nullptr == sInstance) {
        sInstance = new TPZA_Executor();
        if (nullptr != sInstance)
            threadPool = new ThreadPoolZmqActor(ComoConfig::ThreadPoolZmqActor_MAX_THREAD_NUM);

        if (nullptr == threadPool) {
            delete sInstance;
            return nullptr;
        }
    }
    return sInstance;
}

int TPZA_Executor::RunTask(
    /* [in] */ AutoPtr<TPZA_Executor::Worker> task)
{
    return threadPool->AddTask(task);
}

int TPZA_Executor::CleanTask(int posWorkerList)
{
    return threadPool->CleanTask(posWorkerList);
}

int TPZA_Executor::SetDefaultHandleMessage(HANDLE_MESSAGE_FUNCTION func)
{
    defaultHandleMessage = func;
    return 0;
}

static void CalWaitTime(struct timespec& curTime, int timeout_ms) {
    clock_gettime(CLOCK_REALTIME, &curTime);
    long nsec = curTime.tv_nsec + (timeout_ms % 1000) * 1000000;
                                                      // 654321
    curTime.tv_sec = curTime.tv_sec + nsec / 1000000000 + timeout_ms / 1000;
                                           // 987654321
    curTime.tv_nsec = nsec % 1000000000;
                           // 987654321
}

static bool LivingWorker(TPZA_Executor::Worker *worker)
{
    if (nullptr == worker)
        return false;

    switch (worker->mWorkerStatus) {
        case WORKER_TASK_READY:
        case WORKER_TASK_RUNNING:
        case WORKER_TASK_DAEMON_RUNNING:
            return true;
    }
    return false;
}

void *ThreadPoolZmqActor::threadManager(void *threadData)
{
    int i;
    int iWorkerInQueue;

    while (true) {
        pthread_mutex_lock(&pthreadMutex);

        struct timespec curTime;
        CalWaitTime(curTime, 1000 * 60 * 5);    // 60*5 seconds
        while ((! signal_) && (! shutdown)) {
            // The work of cleaning up the mWorkerList later may
            // also need to be done regularly
            pthread_cond_timedwait(&pthreadCond, &pthreadMutex, &curTime);
        }
        signal_ = false;

        pthread_mutex_unlock(&pthreadMutex);

        struct timespec currentTime;
        clock_gettime(CLOCK_REALTIME, &currentTime);

        // check for time out connection
        // ns accuracy is not required
        if (mWorkerList.size() > ComoConfig::DBUS_CONNECTION_MAX_NUM ||
                         (currentTime.tv_sec - lastCheckConnExpireTime.tv_sec) >
                                    ComoConfig::DBUS_BUS_CHECK_EXPIRES_PERIOD) {
            clock_gettime(CLOCK_REALTIME, &lastCheckConnExpireTime);

            pthread_mutex_lock(&pthreadMutex);

            for (i = 0;  (i < mWorkerList.size()) &&
                                         !LivingWorker(mWorkerList[i]);  i++) {
                if (nullptr == mWorkerList[i])
                    continue;

                if (1000000000L * (currentTime.tv_sec - mWorkerList[i]->lastAccessTime.tv_sec) +
                   /*987654321*/(currentTime.tv_nsec - mWorkerList[i]->lastAccessTime.tv_nsec) >
                                                         ComoConfig::DBUS_BUS_SESSION_EXPIRES) {
                    //@ `ReleaseWorker`
                    // If a Worker has not been accessed for a long time or has
                    // been actively released, it will be REFCOUNT_RELEASE
                    REFCOUNT_RELEASE(mWorkerList[i]);
                    mWorkerList[i] = nullptr;
                }
            }
            pthread_mutex_unlock(&pthreadMutex);
        }

        if (shutdown) {
            pthread_exit(nullptr);
            return nullptr;
        }
    }

    return reinterpret_cast<void*>(NOERROR);
}

//-------------------------------------------------------------------------
// ThreadPoolZmqActor
//

bool ThreadPoolZmqActor::shutdown = false;
std::vector<TPZA_Executor::Worker*> ThreadPoolZmqActor::mWorkerList;   // task list
bool ThreadPoolZmqActor::signal_;

pthread_mutex_t ThreadPoolZmqActor::pthreadMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ThreadPoolZmqActor::pthreadCond = PTHREAD_COND_INITIALIZER;

ThreadPoolZmqActor::ThreadPoolZmqActor(int threadNum)
{
    if (CZMQUtils::CzmqGetSockets(nullptr, nullptr) < 0) {
        Logger::E("ThreadPoolZmqActor", "CzmqGetSockets error");
    }

    mThreadNum = threadNum;
    pthread_id_HandleMessage = (pthread_t*)calloc(mThreadNum, sizeof(pthread_t));
    if (nullptr == pthread_id_HandleMessage) {
        Logger::E("ThreadPoolChannelInvoke", "calloc() error");
        return;
    }

    pthread_attr_t threadAddr;
    pthread_attr_init(&threadAddr);
    pthread_attr_setdetachstate(&threadAddr, PTHREAD_CREATE_DETACHED);
    if (pthread_create(&pthread_id_Manager, nullptr,
                             ThreadPoolZmqActor::threadManager, nullptr) != 0) {
        Logger::E("ThreadPoolZmqActor", "pthread_create() error");
    }

    for (size_t i = 0;  i < mThreadNum;  i++) {
        if (pthread_create(&pthread_id_HandleMessage[i], nullptr,
                                         threadHandleMessage, (void *)i) != 0) {
            Logger::E("ThreadPoolZmqActor", "pthread_create() error");
        }
    }

    signal_ = false;
}

/*
 * return position in mWorkerList
 */
int ThreadPoolZmqActor::AddTask(
    /* [in] */ AutoPtr<TPZA_Executor::Worker> task)
{
    int i;
    struct timespec currentTime;
    clock_gettime(CLOCK_REALTIME, &currentTime);

    pthread_mutex_lock(&pthreadMutex);

    for (i = 0;  i < mWorkerList.size();  i++) {
        if (nullptr == mWorkerList[i])
            break;

        if (LivingWorker(mWorkerList[i]))
            continue;

        if (1000000000L * (currentTime.tv_sec - mWorkerList[i]->lastAccessTime.tv_sec) +
           /*987654321*/(currentTime.tv_nsec - mWorkerList[i]->lastAccessTime.tv_nsec) >
                                                        ComoConfig::TPZA_TASK_EXPIRES) {
            break;
        }
    }
    if (i < mWorkerList.size()) {
        if (nullptr != mWorkerList[i])
            REFCOUNT_RELEASE(mWorkerList[i]);
        mWorkerList[i] = task;
    }
    else {
        mWorkerList.push_back(task);
    }
    REFCOUNT_ADD(task);

    signal_ = true;
    pthread_cond_signal(&pthreadCond);
    pthread_mutex_unlock(&pthreadMutex);

    return i;
}

/*
 * Find Worker by ChannelHandle, `mSocket + mChannel` is unique id of an IStub
 */
TPZA_Executor::Worker *ThreadPoolZmqActor::PickWorkerByChannelHandle(
                                                 HANDLE hChannel, bool isDaemon)
{
    int i;
    TPZA_Executor::Worker *w;

    pthread_mutex_lock(&pthreadMutex);

    for (i = 0;  i < mWorkerList.size();  i++) {
        if (nullptr == mWorkerList[i])
            continue;

        if (mWorkerList[i]->mChannel == hChannel)
            break;
    }
    if (i < mWorkerList.size()) {
        w = mWorkerList[i];

        // `ReleaseWorker`
        if (! isDaemon) {
            REFCOUNT_RELEASE(mWorkerList[i]);
            mWorkerList[i] = nullptr;
        }
    }
    else {
        w = nullptr;
    }

    pthread_mutex_unlock(&pthreadMutex);

    return w;
}

int ThreadPoolZmqActor::CleanTask(int posWorkerList)
{
    if (posWorkerList < 0 || (posWorkerList >= mWorkerList.size()))
        return -1;

    pthread_mutex_lock(&pthreadMutex);

    if (nullptr != mWorkerList[posWorkerList])
        REFCOUNT_RELEASE(mWorkerList[posWorkerList]);

    mWorkerList[posWorkerList] = nullptr;
    pthread_mutex_unlock(&pthreadMutex);

    return posWorkerList;
}

int ThreadPoolZmqActor::StopAll()
{
    if (shutdown) {
        return -1;
    }

    shutdown = true;
    pthread_cond_broadcast(&pthreadCond);

    for (int i = 0; i < mThreadNum; i++) {
        pthread_join(pthread_id_HandleMessage[i], nullptr);
    }
    free(pthread_id_HandleMessage);
    pthread_id_HandleMessage = nullptr;

    pthread_join(pthread_id_Manager, nullptr);

    pthread_mutex_destroy(&pthreadMutex);
    pthread_cond_destroy(&pthreadCond);

    return 0;
}

int ThreadPoolZmqActor::GetTaskListSize()
{
    return mWorkerList.size();
}

/*
 * Clean Worker by ChannelHandle, `mSocket + mChannel` is unique id of an IStub
 */
ECode ThreadPoolZmqActor::CleanWorkerByChannelHandle(HANDLE hChannel)
{
    int i;
    ECode ec = NOERROR;

    pthread_mutex_lock(&pthreadMutex);

    for (i = 0;  i < mWorkerList.size();  i++) {
        if (nullptr == mWorkerList[i])
            continue;

        if (mWorkerList[i]->mChannel == hChannel)
            break;
    }
    if (i < mWorkerList.size()) {
        //@ `ReleaseWorkerWhenPickIt`
        REFCOUNT_RELEASE(mWorkerList[i]);
        mWorkerList[i] = nullptr;
    }
    else {
        ec = E_NOT_FOUND_EXCEPTION;
    }

    pthread_mutex_unlock(&pthreadMutex);

    return ec;
}

} // namespace como
