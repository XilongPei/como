//=========================================================================
// Copyright (C) 2018 The C++ Component Model(COMO) Open Source Project
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

//=========================================================================
// Copyright (C) 2012 The Elastos Open Source Project
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

#include "ThreadPoolExecutor.h"
#include "util/comolog.h"
#include "ComoConfig.h"
#include <assert.h>
#include <cerrno>
#include <csignal>
#include <unistd.h>
#include <pthread.h>

namespace como {

//-------------------------------------------------------------------------

ThreadPoolExecutor::Worker::Worker(
    /* [in] */ Runnable* task,
    /* [in] */ ThreadPoolExecutor* owner)
    : mTask(task)
    , mOwner(owner)
{}

ECode ThreadPoolExecutor::Worker::Run()
{
    return mTask->Run();
}

//-------------------------------------------------------------------------

AutoPtr<ThreadPoolExecutor> ThreadPoolExecutor::sInstance = nullptr;
Mutex ThreadPoolExecutor::sInstanceLock;
AutoPtr<ThreadPool> ThreadPoolExecutor::threadPool = nullptr;

AutoPtr<ThreadPoolExecutor> ThreadPoolExecutor::GetInstance()
{
    {
        Mutex::AutoLock lock(sInstanceLock);
        if (sInstance == nullptr) {
            sInstance = new ThreadPoolExecutor();
            threadPool = new ThreadPool(ComoConfig::ThreadPool_MAX_THREAD_NUM);
        }
    }
    return sInstance;
}

ECode ThreadPoolExecutor::RunTask(
    /* [in] */ Runnable* task)
{
    AutoPtr<Worker> w = new Worker(task, this);
    threadPool->addTask(w);
    return NOERROR;
}

void *ThreadPool::threadFunc(void *threadData)
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
        AutoPtr<ThreadPoolExecutor::Worker> w = mWorkerList.Get(i);
        mWorkerList.Remove(i);
#ifdef __DEBUG__
        {
            pid_t pid;
            pthread_t tid;

            pid = getpid();
            tid = pthread_self();
            Logger::D("ThreadPool", "pid:%u tid:%u (0x%x) mWorkerList.GetSize()=%d",
                      (unsigned int)pid, (unsigned int)tid, (unsigned int)tid, i);
        }
#endif
        pthread_mutex_unlock(&m_pthreadMutex);

        ec = w->Run();
    }

    return reinterpret_cast<void*>(ec);
}

//-------------------------------------------------------------------------
// ThreadPool
//

bool ThreadPool::shutdown = false;
ArrayList<ThreadPoolExecutor::Worker*> ThreadPool::mWorkerList;      // task list

pthread_mutex_t ThreadPool::m_pthreadMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ThreadPool::m_pthreadCond = PTHREAD_COND_INITIALIZER;

ThreadPool::ThreadPool(int threadNum)
{
    mThreadNum = threadNum;
    if (create() != 0) {
        Logger::E("ThreadPool", "create thread error");
    }
}

int ThreadPool::addTask(ThreadPoolExecutor::Worker *task)
{
    pthread_mutex_lock(&m_pthreadMutex);
    mWorkerList.Add(task);
    pthread_mutex_unlock(&m_pthreadMutex);
    pthread_cond_signal(&m_pthreadCond);
    return 0;
}

int ThreadPool::create()
{
    pthread_ids = (pthread_t*)calloc(mThreadNum, sizeof(pthread_t));
    if (nullptr == pthread_ids) {
        return E_RUNTIME_EXCEPTION;
    }

    for (int i = 0; i < mThreadNum; i++) {
        pthread_attr_t threadAddr;
        pthread_attr_init(&threadAddr);
        pthread_attr_setdetachstate(&threadAddr, PTHREAD_CREATE_DETACHED);

        pthread_t thread;
        int ret = pthread_create(&pthread_ids[i], nullptr, ThreadPool::threadFunc, nullptr);
        if (ret != 0) {
            return E_RUNTIME_EXCEPTION;
        }
    }
    return 0;
}

int ThreadPool::stopAll()
{
    if (shutdown) {
        return -1;
    }

    shutdown = true;
    pthread_cond_broadcast(&m_pthreadCond);

    for (int i = 0; i < mThreadNum; i++) {
        pthread_join(pthread_ids[i], nullptr);
    }

    free(pthread_ids);
    pthread_ids = nullptr;

    pthread_mutex_destroy(&m_pthreadMutex);
    pthread_cond_destroy(&m_pthreadCond);

    return 0;
}

int ThreadPool::getTaskSize()
{
    return mWorkerList.GetSize();
}

} // namespace como
