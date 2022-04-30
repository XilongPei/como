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
{
    clock_gettime(CLOCK_REALTIME, &lastAccessTime);

    mSocket = channel->GetSocket();
    Long serverObjectId;
    channel->GetServerObjectId(mChannel);
}

TPZA_Executor::Worker::~Worker()
{
/**
 * The ZeroMQ socket managed by ZeroMQ/zmq_sockets are always open. In order to
 * improve the efficiency of use, always maintain the connection with all servers.
 */
#if 0
    int num = ThreadPoolZmqActor::countWorkerBySocket(mSocket);
    if (1 == num) {
        if (zmq_close(mSocket) != 0)
            Logger::E("TPZA_Executor::Worker::~Worker", "zmq_close() errno %d", zmq_errno());
    }
    else if (0 == num) {
        Logger::E("TPZA_Executor::Worker::~Worker",
                                "The Worker can't be found in ThreadPoolZmqActor::mWorkerList");
    }
#endif
}

static Integer SendECode(HANDLE hChannel, void *socket, ECode ec)
{
    HANDLE resData;

    resData = reinterpret_cast<HANDLE>((char*)"");
    return CZMQUtils::CzmqSendBuf(hChannel, ec, socket,
                                              (const void *)&ec, sizeof(ECode));
}

TPZA_Executor::Worker *TPZA_Executor::Worker::HandleMessage()
{
    HANDLE hChannel;
    Integer eventCode;
    zmq_msg_t msg;
    ECode ec;
    int numberOfBytes;
    HANDLE resData;
    Long resSize;

#if 0
    Logger::D("TPZA_Executor::Worker::HandleMessage",
              "Try to CzmqRecvMsg from endpoint %s", mEndpoint.c_str());
#endif

    Integer iRet = CZMQUtils::CzmqRecvMsg(hChannel, eventCode, mSocket, msg, 0);

    if (iRet > 0) {
        clock_gettime(CLOCK_REALTIME, &lastAccessTime);
        switch (eventCode) {
            case ZmqFunCode::Method_Invoke: {
                Worker *w;
                AutoPtr<IParcel> resParcel;

                AutoPtr<IParcel> argParcel = new CZMQParcel();
                if (nullptr == argParcel) {
                    Logger::E("TPZA_Executor::Worker::HandleMessage",
                                               "new CZMQParcel return nullptr");
                    resData = reinterpret_cast<HANDLE>((char*)"");
                    resSize = 1;
                    goto HandleMessage_Method_Invoke;
                }

                ec = argParcel->SetData(reinterpret_cast<HANDLE>(
                                       zmq_msg_data(&msg)), zmq_msg_size(&msg));
                if (FAILED(ec)) {
                    Logger::E("TPZA_Executor::Worker::HandleMessage",
                                                    "argParcel->SetData error");
                    resData = reinterpret_cast<HANDLE>((char*)"");
                    resSize = 1;
                    goto HandleMessage_Method_Invoke;
                }

                zmq_msg_close(&msg);
                resParcel = new CZMQParcel();
                if (nullptr == resParcel) {
                    Logger::E("TPZA_Executor::Worker::HandleMessage",
                                               "new CZMQParcel return nullptr");
                    resData = reinterpret_cast<HANDLE>((char*)"");
                    resSize = 1;
                    goto HandleMessage_Method_Invoke;
                }

                if (hChannel == mChannel) {
                    ec = mStub->Invoke(argParcel, resParcel);

                    resParcel->GetData(resData);
                    resParcel->GetDataSize(resSize);
                }
                else {
                    w = ThreadPoolZmqActor::PickWorkerByChannelHandle(hChannel, true);
                    if (nullptr != w) {
                        ec = w->mStub->Invoke(argParcel, resParcel);

                        // It shouldn't be WORKER_TASK_FINISH.
                        // A request ends, but the channel still needs to be.
                        w->mWorkerStatus = WORKER_TASK_DAEMON_RUNNING;

                        resParcel->GetData(resData);
                        resParcel->GetDataSize(resSize);
                    }
                    else {
                        if (nullptr != TPZA_Executor::defaultHandleMessage) {
                            if (TPZA_Executor::defaultHandleMessage(
                                reinterpret_cast<HANDLE>(nullptr), eventCode,
                                                           mSocket, msg) != 0) {
                                Logger::E("TPZA_Executor::Worker::HandleMessage",
                                                  "defaultHandleMessage error");
                            }
                            else {
                                // defaultHandleMessage will call CzmqSendBuf
                                mWorkerStatus = WORKER_TASK_DAEMON_RUNNING;
                                return nullptr;
                            }
                        }
                        resData = reinterpret_cast<HANDLE>((char*)"");
                        resSize = 1;
                        resParcel->GetData(resData);
                        resParcel->GetDataSize(resSize);
                    }
                }
HandleMessage_Method_Invoke:
                numberOfBytes = CZMQUtils::CzmqSendBuf(mChannel, ec,
                                       mSocket, (const void *)resData, resSize);

                // `ReleaseWorker`, This Worker is a daemon
                mWorkerStatus = WORKER_TASK_DAEMON_RUNNING;
                return nullptr;
            }

            case ZmqFunCode::GetComponentMetadata: {
                Array<Byte> metadata;
                AutoPtr<IMetaComponent> mc;

                AutoPtr<IParcel> argParcel = new CZMQParcel();
                if (nullptr == argParcel) {
                    Logger::E("TPZA_Executor::Worker::HandleMessage",
                                               "new CZMQParcel return nullptr");
                    // `ReleaseWorker`, This Worker is a daemon
                    mWorkerStatus = WORKER_TASK_DAEMON_RUNNING;
                    ec = E_OUT_OF_MEMORY_ERROR;
                    goto HandleMessage_GetComponentMetadata;
                }

                argParcel->SetData(reinterpret_cast<HANDLE>(zmq_msg_data(&msg)),
                                                            zmq_msg_size(&msg));
                CoclassID cid;
                argParcel->ReadCoclassID(cid);
                ec = CoGetComponentMetadata(*cid.mCid, nullptr, mc);
                if (FAILED(ec)) {
                    Logger::E("TPZA_Executor::Worker::HandleMessage",
                               "CoGetComponentMetadata error, ECode: 0x%X", ec);
                    ReleaseCoclassID(cid);
                    mWorkerStatus = WORKER_TASK_DAEMON_RUNNING;
                    goto HandleMessage_GetComponentMetadata;
                }

                ec = mc->GetSerializedMetadata(metadata);

                ReleaseCoclassID(cid);
                if (SUCCEEDED(ec)) {
                    numberOfBytes = CZMQUtils::CzmqSendBuf(mChannel, ec, mSocket,
                                    metadata.GetPayload(), metadata.GetLength());
                }
                else {
                    numberOfBytes = CZMQUtils::CzmqSendBuf(mChannel, ec, mSocket,
                                                                   (char*)"", 1);
                }

                // `ReleaseWorker`, This Worker is a daemon
                mWorkerStatus = WORKER_TASK_DAEMON_RUNNING;
                return nullptr;

HandleMessage_GetComponentMetadata:
                resData = reinterpret_cast<HANDLE>((char*)"");
                resSize = 1;
                numberOfBytes = CZMQUtils::CzmqSendBuf(mChannel, ec,
                                       mSocket, (const void *)resData, resSize);

                // `ReleaseWorker`, This Worker is a daemon
                mWorkerStatus = WORKER_TASK_DAEMON_RUNNING;
                return nullptr;
            }

            case ZmqFunCode::Actor_IsPeerAlive: {
                Boolean b = true;
                numberOfBytes = CZMQUtils::CzmqSendBuf(mChannel, ec, mSocket,
                                             (const void *)&b, sizeof(Boolean));
                // `ReleaseWorker` will NOT delete this work
                mWorkerStatus = WORKER_TASK_DAEMON_RUNNING;
                return this;
            }

            case ZmqFunCode::Object_Release: {
                if (zmq_msg_size(&msg) < sizeof(Long)) {
                    Logger::E("TPZA_Executor::Worker::HandleMessage",
                                          "Object_Release, Bad request packet");
                    ec = ZMQ_BAD_PACKET;
                    goto HandleMessage_Object_Release;
                }

                ec = UnregisterExportObjectByHash(RPCType::Remote,
                                                    *(Long*)zmq_msg_data(&msg));
                if (FAILED(ec)) {
                    Logger::E("TPZA_Executor::Worker::HandleMessage",
                                       "Object_Release error, ECode: 0x%X", ec);
                }

HandleMessage_Object_Release:
                SendECode(mChannel, mSocket, ec);

                // `ReleaseWorker` will NOT delete this work
                mWorkerStatus = WORKER_TASK_DAEMON_RUNNING;

                return this;
            }

            default:
                if (nullptr != TPZA_Executor::defaultHandleMessage) {
                    if (TPZA_Executor::defaultHandleMessage(
                        reinterpret_cast<HANDLE>(nullptr), eventCode, mSocket, msg) != 0) {
                        Logger::E("TPZA_Executor::Worker::Invoke", "bad eventCode");
                    }
                    else {
                        // defaultHandleMessage will call CzmqSendBuf
                        mWorkerStatus = WORKER_TASK_DAEMON_RUNNING;
                        return nullptr;
                    }
                }
                SendECode(mChannel, mSocket, NOERROR);
        }
    }
    else if (iRet < 0) {
        SendECode(mChannel, mSocket, NOERROR);
    }

    mWorkerStatus = WORKER_TASK_DAEMON_RUNNING;
    return nullptr;
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

void *ThreadPoolZmqActor::threadFunc(void *threadData)
{
    ECode ec;
    int i;
    int iWorkerInQueue;

    while (true) {
        iWorkerInQueue = -1;
        pthread_mutex_lock(&pthreadMutex);
        for (i = 0;  i < mWorkerList.size();  i++) {
            if (LivingWorker(mWorkerList[i])) {
                iWorkerInQueue = i;
                break;
            }
        }
        if (iWorkerInQueue < 0) {
            if ((size_t)threadData % 2 == 0) {
                struct timespec curTime;
                CalWaitTime(curTime, 1000 * 60 * 5);    // 60*5 seconds
                while ((! signal_) && (! shutdown)) {
                    // The work of cleaning up the mWorkerList later may
                    // also need to be done regularly
                    pthread_cond_timedwait(&pthreadCond, &pthreadMutex, &curTime);
                }
                signal_ = false;
                pthread_mutex_unlock(&pthreadMutex);
            }
            else {
                pthread_mutex_unlock(&pthreadMutex);
printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
                CZMQUtils::CzmqPoll();
            }
        }
        else {
            pthread_mutex_unlock(&pthreadMutex);
        }

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
                    REFCOUNT_RELEASE(mWorkerList[i]);
                    mWorkerList[i] = nullptr;
                }
            }
            pthread_mutex_unlock(&pthreadMutex);
        }

        if (iWorkerInQueue < 0)
            continue;

        pthread_mutex_lock(&pthreadMutex);
        AutoPtr<TPZA_Executor::Worker> worker;
        i = iWorkerInQueue;
        iWorkerInQueue = -1;
        for (;  i < mWorkerList.size();  i++) {
            if (LivingWorker(mWorkerList[i])) {
                iWorkerInQueue = i;
                worker = mWorkerList[i];
                worker->mWorkerStatus = WORKER_TASK_RUNNING;
                break;
            }
        }
        pthread_mutex_unlock(&pthreadMutex);

        // There are Workers in the queue. Try to do all the Workers
        AutoPtr<TPZA_Executor::Worker> workerRet;
        while ((iWorkerInQueue >= 0) && (! shutdown)) {

            Logger::D("ThreadPoolZmqActor::threadFunc",
                      "HandleMessage, endpoint: %s", worker->mEndpoint.c_str());

            EndpointSocket *eps = CZMQUtils::CzmqFindSocket(worker->mEndpoint);
            assert(eps->socket == worker->mSocket);

            if (nullptr != eps) {
            // Do not use the same ZeroMQ socket in multiple threads, that is,
            // socket can only be used in one thread.

                pthread_mutex_lock(&(eps->mutex));
                workerRet = worker->HandleMessage();
                pthread_mutex_unlock(&(eps->mutex));
            }

            if (workerRet != mWorkerList[iWorkerInQueue]) {
            // Current Worker has been processed
                if (WORKER_TASK_DAEMON_RUNNING != worker->mWorkerStatus) {
                    //@ `ReleaseWorker`
                    REFCOUNT_RELEASE(worker);
                    mWorkerList[iWorkerInQueue] = nullptr;
                }
            }
            else {
                //refer to `ReleaseWorkerWhenPickIt`
                if (nullptr != workerRet) {
                    // A Worker, not current Worker, has been processed
                    if (WORKER_TASK_DAEMON_RUNNING != workerRet->mWorkerStatus)
                        REFCOUNT_RELEASE(workerRet);
                }
                // else, (nullptr == workerRet),
                // This Worker is a daemon, do nothing.
            }

            // look for another Worker
            iWorkerInQueue++;
            pthread_mutex_lock(&pthreadMutex);
            if (iWorkerInQueue < mWorkerList.size()) {
                for (i = iWorkerInQueue;  i < mWorkerList.size();  i++) {
                    if (LivingWorker(mWorkerList[i])) {
                        iWorkerInQueue = i;
                        worker = mWorkerList[i];
                        worker->mWorkerStatus = WORKER_TASK_RUNNING;
                        break;
                    }
                }

                if (i >= mWorkerList.size()) {
                    iWorkerInQueue = -1;
                }
            }
            else {
                iWorkerInQueue = -1;
            }
            pthread_mutex_unlock(&pthreadMutex);
        }

        if (shutdown) {
            pthread_mutex_unlock(&pthreadMutex);
            pthread_exit(nullptr);
            return nullptr;
        }
    }

    return reinterpret_cast<void*>(ec);
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
    mThreadNum = threadNum;
    pthread_id = (pthread_t*)calloc(mThreadNum, sizeof(pthread_t));
    if (nullptr == pthread_id) {
        Logger::E("ThreadPoolChannelInvoke", "calloc() error");
        return;
    }

    for (size_t i = 0;  i < mThreadNum;  i++) {
        pthread_attr_t threadAddr;
        pthread_attr_init(&threadAddr);
        pthread_attr_setdetachstate(&threadAddr, PTHREAD_CREATE_DETACHED);
        if (pthread_create(&pthread_id[i], nullptr,
                                ThreadPoolZmqActor::threadFunc, (void *)i) != 0) {
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

        //@ `ReleaseWorkerWhenPickIt`
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

/*
 * Count Worker by socket
 */
int ThreadPoolZmqActor::CountWorkerBySocket(void *socket)
{
    int num = 0;

    pthread_mutex_lock(&pthreadMutex);

    for (int i = 0;  i < mWorkerList.size();  i++) {
        if (nullptr == mWorkerList[i])
            continue;

        if (mWorkerList[i]->mSocket == socket)
            num++;
    }

    pthread_mutex_unlock(&pthreadMutex);

    return num;
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
        pthread_join(pthread_id[i], nullptr);
    }

    free(pthread_id);
    pthread_id = nullptr;

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
