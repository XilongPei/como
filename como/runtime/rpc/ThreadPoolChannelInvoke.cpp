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

#include "TPCI_Executor.h"
#include "util/comolog.h"
#include "ComoConfig.h"
#include <assert.h>
#include <cerrno>
#include <csignal>
#include <pthread.h>

namespace como {

//-------------------------------------------------------------------------
// TPCI : ThreadPoolChannelInvoke
TPCI_Executor::Worker::Worker(AutoPtr<IRPCChannel> channel, mChannel, AutoPtr<IMetaMethod> method,
                              AutoPtr<IParcel> inParcel, AutoPtr<IParcel> outParcel)
    : mChannel(channel)
    , mMethod(method)
    , mInParcel(inParcel)
    , mOutParcel(outParcel)
    , mOwner(owner)
{}

ECode TPCI_Executor::Worker::Invoke()
{
    return mChannel->Invoke(method, inParcel, outParcel);
}

//-------------------------------------------------------------------------

AutoPtr<TPCI_Executor> TPCI_Executor::sInstance = nullptr;
Mutex TPCI_Executor::sInstanceLock;
AutoPtr<ThreadPoolChannelInvoke> TPCI_Executor::threadPool = nullptr;
AutoPtr<ThreadPoolChannelInvokeECode> TPCI_Executor::threadPoolECode = nullptr;
pthread_mutex_t *TPCI_Executor::locksThreadPoolECode = nullptr;
23     pthread_mutex_init(&lock, NULL);

AutoPtr<TPCI_Executor> TPCI_Executor::GetInstance()
{
    {
        Mutex::AutoLock lock(sInstanceLock);
        if (sInstance == nullptr) {
            sInstance = new TPCI_Executor();
            threadPool = new ThreadPoolChannelInvoke(ComoConfig::ThreadPoolChannelInvoke_MAX_THREAD_NUM);
            threadPoolECode = calloc(sizeof(ECode), ComoConfig::ThreadPoolChannelInvoke_MAX_THREAD_NUM);

            locksThreadPoolECode = calloc(sizeof(pthread_mutex_t), ComoConfig::ThreadPoolChannelInvoke_MAX_THREAD_NUM);
            for (int i = 0;  i < ThreadPoolChannelInvoke_MAX_THREAD_NUM;  i++) {
                pthread_mutex_init(&locksThreadPoolECode[i], NULL);
            }
        }
    }
    return sInstance;
}

int TPCI_Executor::RunTask(AutoPtr<IMetaMethod> method, AutoPtr<IParcel> inParcel, AutoPtr<IParcel> outParcel)
{
    AutoPtr<Worker> w = new Worker(method, inParcel, outParcel, this);
    int i = threadPool->addTask(w);
    pthread_mutex_lock(&locksThreadPoolECode[i]);
}

void *ThreadPoolChannelInvoke::threadFunc(void *threadData)
{
    ECode ec;

    while (true) {
        pthread_mutex_lock(&m_pthreadMutex);

        while ((mWorkerList.GetSize() == 0) && !shutdown) {
            pthread_cond_wait(&m_pthreadCond, &m_pthreadMutex);
        }

        if (shutdown) {
            pthread_mutex_unlock(&m_pthreadMutex);
            pthread_exit(nullptr);
        }

        Long i = mWorkerList.GetSize() - 1;
        AutoPtr<TPCI_Executor::Worker> w = mWorkerList.Get(i);
        mWorkerList.Remove(i);

        pthread_mutex_unlock(&m_pthreadMutex);

        ec = w->Invoke();
    }

    return reinterpret_cast<void*>(ec);
}

//-------------------------------------------------------------------------
// ThreadPoolChannelInvoke
//

bool ThreadPoolChannelInvoke::shutdown = false;
ArrayList<TPCI_Executor::Worker*> ThreadPoolChannelInvoke::mWorkerList;      // task list

pthread_mutex_t ThreadPoolChannelInvoke::m_pthreadMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ThreadPoolChannelInvoke::m_pthreadCond = PTHREAD_COND_INITIALIZER;

ThreadPoolChannelInvoke::ThreadPoolChannelInvoke(int threadNum)
{
    mThreadNum = threadNum;
    create();
}

int ThreadPoolChannelInvoke::addTask(TPCI_Executor::Worker *task)
{
    pthread_mutex_lock(&m_pthreadMutex);
    mWorkerList.Add(task);
    pthread_mutex_unlock(&m_pthreadMutex);
    pthread_cond_signal(&m_pthreadCond);

    // assign it an ID, return the ID
    return 0;
}

int ThreadPoolChannelInvoke::create()
{
    pthread_id = (pthread_t*)calloc(mThreadNum, sizeof(pthread_t));

    for (int i = 0; i < mThreadNum; i++) {
        pthread_attr_t threadAddr;
        pthread_attr_init(&threadAddr);
        pthread_attr_setdetachstate(&threadAddr, PTHREAD_CREATE_DETACHED);

        pthread_t thread;
        int ret = pthread_create(&pthread_id[i], nullptr, ThreadPoolChannelInvoke::threadFunc, nullptr);
        if (ret != 0) {
            return E_RUNTIME_EXCEPTION;
        }
    }
    return 0;
}

int ThreadPoolChannelInvoke::stopAll()
{
    if (shutdown) {
        return -1;
    }

    shutdown = true;
    pthread_cond_broadcast(&m_pthreadCond);

    for (int i = 0; i < mThreadNum; i++) {
        pthread_join(pthread_id[i], nullptr);
    }

    free(pthread_id);
    pthread_id = nullptr;

    pthread_mutex_destroy(&m_pthreadMutex);
    pthread_cond_destroy(&m_pthreadCond);

    return 0;
}

int ThreadPoolChannelInvoke::getTaskSize()
{
    return mWorkerList.GetSize();
}

} // namespace como
