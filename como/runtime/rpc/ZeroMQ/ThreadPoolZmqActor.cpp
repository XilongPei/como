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
#include "zmq.h"
#include "util/comolog.h"
#include "ComoConfig.h"
#include "ThreadPoolZmqActor.h"
#include "CZMQUtils.h"
#include "CZMQParcel.h"

namespace como {

static struct timespec lastCheckConnExpireTime = {0, 0};

//-------------------------------------------------------------------------
// TPZA : ThreadPoolZmqActor
TPZA_Executor::Worker::Worker(CZMQChannel *channel, AutoPtr<IStub> stub)
    : mStub(stub)
    , mWorkerStatus(WORKER_TASK_READY)
{
    clock_gettime(CLOCK_REALTIME, &lastAccessTime);

    mSocket = channel->GetSocket();

    HANDLE pid = getpid();
    mChannel = ((pid & 0xFFFF) << 48) | (reinterpret_cast<HANDLE>(channel) & 0xFFFFFFFFFFFF);
    //                    1 0                                                   5 4 3 2 1 0
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

TPZA_Executor::Worker *TPZA_Executor::Worker::HandleMessage()
{
    HANDLE hChannel;
    Integer eventCode;
    zmq_msg_t msg;
    ECode ec;
    int numberOfBytes;

    if (CZMQUtils::CzmqRecvMsg(hChannel, eventCode, mSocket, msg, ZMQ_DONTWAIT) != 0) {
        clock_gettime(CLOCK_REALTIME, &lastAccessTime);
        switch (eventCode) {
            case ZmqFunCode::Method_Invoke: {
                AutoPtr<IParcel> argParcel = new CZMQParcel();
                if (nullptr == argParcel) {
                    Logger::E("TPZA_Executor::Worker::HandleMessage",
                                               "new CZMQParcel return nullptr");
                    break;
                }

                argParcel->SetData(reinterpret_cast<HANDLE>(zmq_msg_data(&msg)),
                                                            zmq_msg_size(&msg));
                zmq_msg_close(&msg);
                AutoPtr<IParcel> resParcel = new CZMQParcel();
                if (nullptr == resParcel) {
                    Logger::E("TPZA_Executor::Worker::HandleMessage",
                                               "new CZMQParcel return nullptr");
                    break;
                }

                if (hChannel == mChannel) {
                    ec = mStub->Invoke(argParcel, resParcel);
                }
                else {
                    Worker *w = ThreadPoolZmqActor::findWorkerByChannelHandle(hChannel);
                    if (nullptr != w) {
                        ec = mStub->Invoke(argParcel, resParcel);
                    }
                    else {
                        if (nullptr != TPZA_Executor::defaultHandleMessage) {
                            if (TPZA_Executor::defaultHandleMessage(
                                reinterpret_cast<HANDLE>(nullptr), eventCode,
                                                           mSocket, msg) != 0) {
                                Logger::E("TPZA_Executor::Worker::HandleMessage",
                                                  "defaultHandleMessage error");
                            }
                        }
                    }
                }

                HANDLE resData;
                Long resSize;
                resParcel->GetData(resData);
                resParcel->GetDataSize(resSize);
                numberOfBytes = CZMQUtils::CzmqSendBuf(mChannel, ec,
                                       mSocket, (const void *)resData, resSize);
                break;
            }

            case ZmqFunCode::GetComponentMetadata: {
                AutoPtr<IParcel> argParcel = new CZMQParcel();
                if (nullptr == argParcel) {
                    Logger::E("TPZA_Executor::Worker::HandleMessage",
                                               "new CZMQParcel return nullptr");
                    break;
                }

                argParcel->SetData(reinterpret_cast<HANDLE>(zmq_msg_data(&msg)),
                                                            zmq_msg_size(&msg));
                CoclassID cid;
                argParcel->ReadCoclassID(cid);
                AutoPtr<IMetaComponent> mc;
                ec = CoGetComponentMetadata(*cid.mCid, nullptr, mc);
                if (FAILED(ec)) {
                    Logger::E("TPZA_Executor::Worker::HandleMessage",
                               "CoGetComponentMetadata error, ECode: 0x%X", ec);
                    ReleaseCoclassID(cid);
                    break;
                }

                Array<Byte> metadata;
                ec = mc->GetSerializedMetadata(metadata);

                ReleaseCoclassID(cid);
                if (SUCCEEDED(ec))
                    numberOfBytes = zmq_send(mSocket, metadata.GetPayload(),
                                                       metadata.GetLength(), 0);
                else
                    numberOfBytes = zmq_send(mSocket, "", 1, 0);

                break;
            }

            case ZmqFunCode::Actor_IsPeerAlive: {
                Boolean b = true;
                numberOfBytes = CZMQUtils::CzmqSendBuf(mChannel, ec, mSocket,
                                             (const void *)&b, sizeof(Boolean));
                break;
            }

            case ZmqFunCode::Object_Release: {
                if (hChannel == mChannel)
                        return this;

                Worker *w = ThreadPoolZmqActor::findWorkerByChannelHandle(hChannel);
                if (nullptr == w) {
                    Logger::E("TPZA_Executor::Worker::Invoke", "bad Channel");
                }
                return w;
            }

            default:
                if (nullptr != TPZA_Executor::defaultHandleMessage) {
                    if (TPZA_Executor::defaultHandleMessage(
                        reinterpret_cast<HANDLE>(nullptr), eventCode, mSocket, msg) != 0) {
                        Logger::E("TPZA_Executor::Worker::Invoke", "bad eventCode");
                    }
                }
        }

        mWorkerStatus = WORKER_TASK_FINISH;
    }

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
    return threadPool->addTask(task);
}

int TPZA_Executor::CleanTask(int posWorkerList)
{
    return threadPool->cleanTask(posWorkerList);
}

int TPZA_Executor::SetDefaultHandleMessage(HANDLE_MESSAGE_FUNCTION func)
{
    defaultHandleMessage = func;
    return 0;
}

void *ThreadPoolZmqActor::threadFunc(void *threadData)
{
    ECode ec;
    int i;

    while (true) {
        struct timespec currentTime;
        clock_gettime(CLOCK_REALTIME, &currentTime);

        // check for time out connection
        // ns accuracy is not required
        // TODO, mWorkerList.size() is not the really num in mWorkerList
        if (mWorkerList.size() > ComoConfig::DBUS_CONNECTION_MAX_NUM ||
                         (currentTime.tv_sec - lastCheckConnExpireTime.tv_sec) >
                                    ComoConfig::DBUS_BUS_CHECK_EXPIRES_PERIOD) {
            clock_gettime(CLOCK_REALTIME, &lastCheckConnExpireTime);

            pthread_mutex_lock(&pthreadMutex);

            for (i = 0;  (i < mWorkerList.size()) &&
                                   (WORKER_TASK_READY != mWorkerList[i]->mWorkerStatus);  i++) {
                if (nullptr == mWorkerList[i])
                    continue;

                if (1000000000L * (currentTime.tv_sec - mWorkerList[i]->lastAccessTime.tv_sec) +
                   /*987654321*/(currentTime.tv_nsec - mWorkerList[i]->lastAccessTime.tv_nsec) >
                                                         ComoConfig::DBUS_BUS_SESSION_EXPIRES) {
                    delete mWorkerList[i];
                    mWorkerList[i]->mWorkerStatus = WORKER_IDLE;
                }
            }
            pthread_mutex_unlock(&pthreadMutex);
        }

        pthread_mutex_lock(&pthreadMutex);

        for (i = 0;  i < mWorkerList.size();  i++) {
            if ((nullptr == mWorkerList[i]) ||
                         (WORKER_TASK_READY == mWorkerList[i]->mWorkerStatus)) {
                break;
            }
        }

        while ((i >= mWorkerList.size()) && !shutdown) {
            if (nullptr != TPZA_Executor::defaultHandleMessage) {
                Integer eventCode;
                zmq_msg_t msg;
                TPZA_Executor::defaultHandleMessage(
                    reinterpret_cast<HANDLE>(nullptr), eventCode, nullptr, msg);
            }

            /* wait 100ns, a short time, CPU is too tired.
               Theoretically, it should be calculated as follows, but the delay
               accuracy is not required here, so it is important to reduce the
               amount of calculation.

               timeout_ms: the ms we want to wait

            long nsec = curTime.tv_nsec + (timeout_ms % 1000) * 1000000;
            curTime.tv_sec = curTime.tv_sec + nsec / 1000000000 + timeout_ms / 1000;
            curTime.tv_nsec = nsec % 1000000000;
            */
            struct timespec curTime;
            clock_gettime(CLOCK_REALTIME, &curTime);
            curTime.tv_nsec += 100;
            pthread_cond_timedwait(&pthreadCond, &pthreadMutex, &curTime);

            for (i = 0;  i < mWorkerList.size();  i++) {
                if (nullptr == mWorkerList[i])
                    continue;

                if (WORKER_TASK_READY == mWorkerList[i]->mWorkerStatus)
                    break;
            }
        }

        if (shutdown) {
            pthread_mutex_unlock(&pthreadMutex);
            pthread_exit(nullptr);
            return nullptr;
        }

        if (i < mWorkerList.size()) {
            AutoPtr<TPZA_Executor::Worker> worker = mWorkerList[i];
            if (nullptr != worker) {
                worker->mWorkerStatus = WORKER_TASK_RUNNING;
                pthread_mutex_unlock(&pthreadMutex);
                if (worker->HandleMessage() != nullptr) {
                    delete worker;
                    mWorkerList[i] = nullptr;
                }
            }
            else {
                pthread_mutex_unlock(&pthreadMutex);
            }
        }
        else {
            pthread_mutex_unlock(&pthreadMutex);
        }
    }

    return reinterpret_cast<void*>(ec);
}

//-------------------------------------------------------------------------
// ThreadPoolZmqActor
//

bool ThreadPoolZmqActor::shutdown = false;
std::vector<TPZA_Executor::Worker*> ThreadPoolZmqActor::mWorkerList;   // task list

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

    for (int i = 0;  i < mThreadNum;  i++) {
        pthread_attr_t threadAddr;
        pthread_attr_init(&threadAddr);
        pthread_attr_setdetachstate(&threadAddr, PTHREAD_CREATE_DETACHED);
        if (pthread_create(&pthread_id[i], nullptr,
                                ThreadPoolZmqActor::threadFunc, nullptr) != 0) {
            Logger::E("ThreadPoolZmqActor", "pthread_create() error");
        }
    }
}

/*
 * return position in mWorkerList
 */
int ThreadPoolZmqActor::addTask(
    /* [in] */ AutoPtr<TPZA_Executor::Worker> task)
{
    int i;
    struct timespec currentTime;
    clock_gettime(CLOCK_REALTIME, &currentTime);

    pthread_mutex_lock(&pthreadMutex);

    for (i = 0;  i < mWorkerList.size();  i++) {
        if (nullptr == mWorkerList[i])
            break;

        // COMO Runtime could set mWorkerStatus as WORKER_IDLE
        if (WORKER_IDLE == mWorkerList[i]->mWorkerStatus)
            break;

        if (WORKER_TASK_RUNNING == mWorkerList[i]->mWorkerStatus)
            continue;

        if ((currentTime.tv_sec - mWorkerList[i]->lastAccessTime.tv_sec) +
                    1000000000L * (currentTime.tv_nsec - mWorkerList[i]->lastAccessTime.tv_nsec) >
                   /*987654321*/                                  ComoConfig::TPZA_TASK_EXPIRES) {
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

    pthread_mutex_unlock(&pthreadMutex);
    pthread_cond_signal(&pthreadCond);

    return i;
}

/*
 * Find Worker by ChannelHandle, `mSocket + mChannel` is unique id of an IStub
 */
TPZA_Executor::Worker *ThreadPoolZmqActor::findWorkerByChannelHandle(HANDLE hChannel)
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
    }
    else {
        w = nullptr;
    }

    pthread_mutex_unlock(&pthreadMutex);
    pthread_cond_signal(&pthreadCond);

    return w;
}

/*
 * Count Worker by socket
 */
int ThreadPoolZmqActor::countWorkerBySocket(void *socket)
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


int ThreadPoolZmqActor::cleanTask(int posWorkerList)
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

int ThreadPoolZmqActor::stopAll()
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

int ThreadPoolZmqActor::getTaskListSize()
{
    return mWorkerList.size();
}

} // namespace como
