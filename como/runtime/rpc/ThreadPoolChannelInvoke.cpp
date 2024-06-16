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

#include <assert.h>
#include <cerrno>
#include <csignal>
#include <pthread.h>
#include "util/comolog.h"
#include "ComoConfig.h"
#include "ComoerrorHelper.h"
#include "ThreadPoolChannelInvoke.h"
#if defined(RPC_OVER_ZeroMQ_SUPPORT)
    #include "ZeroMQ/CZMQUtils.h"
#endif

namespace como {

static ComoEcError g_ecErrors[] = {
    {FUNCTION_SAFETY_CALL_TIMEOUT, "FUNCTION_SAFETY_CALL_TIMEOUT"},                                 // (0, 0x01)
    {FUNCTION_SAFETY_CALL_OUT_OF_MEMORY, "FUNCTION_SAFETY_CALL_OUT_OF_MEMORY"},
#if defined(RPC_OVER_ZeroMQ_SUPPORT)
    {E_ZMQ_FUN_CODE_ERROR, "E_ZMQ_FUN_CODE_ERROR"},
    {ZMQ_BAD_REPLY_DATA, "ZMQ_BAD_REPLY_DATA"},
    {ZMQ_BAD_PACKET, "ZMQ_BAD_PACKET"}
#endif
};

//-------------------------------------------------------------------------
// TPCI : ThreadPoolChannelInvoke
TPCI_Executor::Worker::Worker(AutoPtr<IRPCChannel> channel, AutoPtr<IMetaMethod> method,
                              AutoPtr<IParcel> inParcel, AutoPtr<IParcel> outParcel)
    : mChannel(channel)
    , mMethod(method)
    , mInParcel(inParcel)
    , mOutParcel(outParcel)
    , mWorkerStatus(TPCI_WORKER_TASK_READY)
    , mCond(PTHREAD_COND_INITIALIZER)
    , ec(NOERROR)
{
    pthread_mutex_init(&mMutex, NULL);
    clock_gettime(CLOCK_REALTIME, &mCreateTime);
}

ECode TPCI_Executor::Worker::Invoke()
{
    return mChannel->Invoke(mMethod, mInParcel, mOutParcel);
}

//-------------------------------------------------------------------------

AutoPtr<TPCI_Executor> TPCI_Executor::sInstance = nullptr;
Mutex TPCI_Executor::sInstanceLock;
AutoPtr<ThreadPoolChannelInvoke> TPCI_Executor::threadPool = nullptr;
bool ThreadPoolChannelInvoke::signal_;

AutoPtr<TPCI_Executor> TPCI_Executor::GetInstance()
{
    Mutex::AutoLock lock(sInstanceLock);
    if (nullptr == sInstance) {
        sInstance = new TPCI_Executor();
        if (nullptr != sInstance) {
            threadPool = new ThreadPoolChannelInvoke(
                        ComoConfig::ThreadPoolChannelInvoke_MAX_THREAD_NUM);
        }

        if (nullptr == threadPool) {
            delete sInstance;
            return nullptr;
        }

        ComoerrorHelper::RegisterEcErrors(g_ecErrors,
                                        sizeof(g_ecErrors)/sizeof(ComoEcError));
    }
    return sInstance;
}

int TPCI_Executor::RunTask(AutoPtr<IRPCChannel> channel, AutoPtr<IMetaMethod> method,
                               AutoPtr<IParcel> inParcel, AutoPtr<IParcel> outParcel)
{
    // addTask(w) and subsequent operations are bare pointer to record
    Worker *w = new Worker(channel, method, inParcel, outParcel);
    if (nullptr == w) {
        return -1;
    }
    return threadPool->addTask(w);
}

int TPCI_Executor::CleanTask(int pos)
{
    return threadPool->cleanTask(pos);
}

int ThreadPoolChannelInvoke::LookingForReadyWorker()
{
    for (int i = 0;  i < mWorkerList.size();  i++) {
        if (nullptr == mWorkerList[i]) {
            return i;
        }
        if (TPCI_WORKER_TASK_READY == mWorkerList[i]->mWorkerStatus) {
            return i;
        }
    }
    return -1;
}

void *ThreadPoolChannelInvoke::threadFunc(void *threadData)
{
    int i;

    while (true) {
        pthread_mutex_lock(&m_pthreadMutex);

        i = LookingForReadyWorker();
        if (i < 0) {
            while ((! signal_) && (! shutdown)) {
                pthread_cond_wait(&m_pthreadCond, &m_pthreadMutex);
            }
            signal_ = false;

            i = LookingForReadyWorker();
            if (i < 0) {
                // When signal_ is true, this shouldn't happen
                Logger::E("ThreadPoolChannelInvoke", "LookingForReadyWorker error");
                shutdown = true;
            }
        }

        if (shutdown) {
            pthread_mutex_unlock(&m_pthreadMutex);
            pthread_exit(nullptr);
            return nullptr;
        }

        AutoPtr<TPCI_Executor::Worker> w = mWorkerList[i];
        w->mWorkerStatus = TPCI_WORKER_TASK_RUNNING;

        pthread_mutex_unlock(&m_pthreadMutex);

        // the w->Invoke() will block until the Channel (dbus, binder, ZeroMQ)
        // finish this Worker. such as: CDBusChannel::Invoke()
        w->ec = w->Invoke();

        w->mWorkerStatus = TPCI_WORKER_TASK_FINISH;
        pthread_cond_signal(&(w->mCond));
    }

    return reinterpret_cast<void*>(NOERROR);
}

//-------------------------------------------------------------------------
// ThreadPoolChannelInvoke
//

bool ThreadPoolChannelInvoke::shutdown = false;
std::vector<TPCI_Executor::Worker*> ThreadPoolChannelInvoke::mWorkerList;   // task list

pthread_mutex_t ThreadPoolChannelInvoke::m_pthreadMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ThreadPoolChannelInvoke::m_pthreadCond = PTHREAD_COND_INITIALIZER;

/**
 * constructor of class ThreadPoolChannelInvoke
 *
 * One thread corresponds to an Channel invoke.
 */
ThreadPoolChannelInvoke::ThreadPoolChannelInvoke(int threadNum)
{
    mThreadNum = threadNum;

    /**
     * The single instance is implemented by TPCI_Executor::GetInstance(), and
     * this object is constructed only when COMO_Runtime is started, so dynamic
     * memory can be used.
     */
    mPthreadIds = (pthread_t*)calloc(mThreadNum, sizeof(pthread_t));
    if (nullptr == mPthreadIds) {
        Logger::E("ThreadPoolChannelInvoke", "calloc() error");
        return;
    }

    for (int i = 0;  i < mThreadNum;  i++) {
        pthread_attr_t threadAddr;
        pthread_attr_init(&threadAddr);
        pthread_attr_setdetachstate(&threadAddr, PTHREAD_CREATE_DETACHED);
        if (pthread_create(&mPthreadIds[i], &threadAddr,
                           ThreadPoolChannelInvoke::threadFunc, nullptr) != 0) {
            Logger::E("ThreadPoolChannelInvoke", "pthread_create() error");
            mThreadNum = i;
            break;
        }
    }

    signal_ = false;
}

/*
 * return position in mWorkerList
 */
int ThreadPoolChannelInvoke::addTask(TPCI_Executor::Worker *task)
{
    int i;
    struct timespec currentTime;
    clock_gettime(CLOCK_REALTIME, &currentTime);

    pthread_mutex_lock(&m_pthreadMutex);

    for (i = 0;  i < mWorkerList.size();  i++) {
        if (nullptr == mWorkerList[i]) {
            break;
        }

        // COMO Runtime could set mWorkerStatus as TPCI_WORKER_IDLE
        if (TPCI_WORKER_IDLE == mWorkerList[i]->mWorkerStatus) {
            break;
        }

        if (TPCI_WORKER_TASK_RUNNING == mWorkerList[i]->mWorkerStatus) {
            continue;
        }

        if (1000000000L * (currentTime.tv_sec - mWorkerList[i]->mCreateTime.tv_sec) +
           /*987654321*/(currentTime.tv_nsec - mWorkerList[i]->mCreateTime.tv_nsec) >
                                                     ComoConfig::TPCI_TASK_EXPIRES) {
            break;
        }
    }
    if (i < mWorkerList.size()) {
        if (nullptr != mWorkerList[i]) {
            delete mWorkerList[i];
        }
        mWorkerList[i] = task;
    }
    else {
        mWorkerList.push_back(task);
    }

    signal_ = true;
    pthread_mutex_unlock(&m_pthreadMutex);
    pthread_cond_signal(&m_pthreadCond);

    return i;
}

int ThreadPoolChannelInvoke::cleanTask(int pos)
{
    if ((pos < 0) || (pos >= mWorkerList.size())) {
        return -1;
    }

    pthread_mutex_lock(&m_pthreadMutex);
    delete mWorkerList[pos];
    mWorkerList[pos] = nullptr;
    pthread_mutex_unlock(&m_pthreadMutex);

    return pos;
}

int ThreadPoolChannelInvoke::stopAll()
{
    if (shutdown) {
        return -1;
    }

    shutdown = true;
    pthread_cond_broadcast(&m_pthreadCond);

    for (int i = 0;  i < mThreadNum;  i++) {
        pthread_join(mPthreadIds[i], nullptr);
    }

    free(mPthreadIds);
    mPthreadIds = nullptr;

    pthread_mutex_destroy(&m_pthreadMutex);
    pthread_cond_destroy(&m_pthreadCond);

    return 0;
}

int ThreadPoolChannelInvoke::getTaskListSize()
{
    return mWorkerList.size();
}

} // namespace como
