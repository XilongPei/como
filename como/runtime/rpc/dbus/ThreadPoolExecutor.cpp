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
#include "ComoContext.h"
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
    // This run will not return, but wait in the processing mechanism of the message queue
    // CDBusChannel::ServiceRunnable::Run()
    // The number ComoConfig::ThreadPool_MAX_THREAD_NUM must be greater than the
    // number ComoConfig::ThreadPool_MAX_DBUS_DISPATCHER
    mEc = mTask->Run();

    /**
     * about mEc:
     * `mEc` records the execution of the COMO method and does not carry the
     * return value of the method. The value mEc is used to check whether the
     * calling process is correct.
     */

    return mEc;
}

//-------------------------------------------------------------------------

AutoPtr<ThreadPoolExecutor> ThreadPoolExecutor::sInstance = nullptr;
Mutex ThreadPoolExecutor::sInstanceLock;
AutoPtr<ThreadPool> ThreadPoolExecutor::threadPool = nullptr;

AutoPtr<ThreadPoolExecutor> ThreadPoolExecutor::GetInstance()
{
    Mutex::AutoLock lock(sInstanceLock);
    if (nullptr == sInstance) {
        sInstance = new ThreadPoolExecutor();
        if (nullptr != sInstance) {
            threadPool = new ThreadPool(ComoConfig::ThreadPool_MAX_THREAD_NUM);
        }

        if (nullptr == threadPool) {
            delete sInstance;
            return nullptr;
        }
    }
    return sInstance;
}

int ThreadPoolExecutor::RunTask(
    /* [in] */ Runnable* task)
{
    /**
     * Although the argument `task` is bare pointer, but the object `worker`
     * stores `task` in a smart pointer (AutoPtr): attribute mTask of the
     * object `worker`.
     */
    AutoPtr<Worker> worker = new Worker(task, this);
    if (nullptr == worker) {
        return -1;
    }

    return threadPool->addTask(worker);
}

/**
 * `ThreadPoolExecutor::Worker` w implements the Runnable interface, it is a task,
 * this thread will be waiting for such a task.
 * This thread gets `w` and executes it: w->Run().
 */
void *ThreadPool::threadFunc(void *threadData)
{
    ECode ec;

    while (true) {
        pthread_mutex_lock(&m_pthreadMutex);

        while ((mWorkerList.GetSize() == 0) && (! mShutdown)) {
            pthread_cond_wait(&m_pthreadCond, &m_pthreadMutex);
        }

        if (mShutdown) {
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
            Logger::D("ThreadPool", "pid:%u tid:%u (0x%X) mWorkerList.GetSize()=%d",
                      (unsigned int)pid, (unsigned int)tid, (unsigned int)tid, i);
        }
#endif
        mRuningWorkerSize++;
        pthread_mutex_unlock(&m_pthreadMutex);
        ec = w->Run();
    }

    return reinterpret_cast<void*>(ec);
}

//-------------------------------------------------------------------------
// ThreadPool
//

bool ThreadPool::mShutdown = false;
ArrayList<ThreadPoolExecutor::Worker*> ThreadPool::mWorkerList;      // task list
int ThreadPool::mRuningWorkerSize = 0;

pthread_mutex_t ThreadPool::m_pthreadMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ThreadPool::m_pthreadCond = PTHREAD_COND_INITIALIZER;

ThreadPool::ThreadPool(int threadNum)
{
#ifdef COMO_FUNCTION_SAFETY_RTOS
    if (mThreadNum >= sizeof(ComoContext::gThreadsWorking)) {
        Logger::E("ThreadPoolExecutor", "ComoContext::gThreadsWorking too small");
        threadNum = -1;
        return;
    }
#else
    mPthreadIds = (pthread_t*)calloc(mThreadNum, sizeof(pthread_t));
    if (nullptr == mPthreadIds) {
        Logger::E("ThreadPool", "create thread error");
        threadNum = -1;
        return;
    }
#endif

    // The number of threads in the thread pool.
    mThreadNum = threadNum;

    for (int i = 0;  i < mThreadNum;  i++) {
        pthread_attr_t threadAddr;
        pthread_attr_init(&threadAddr);
        pthread_attr_setdetachstate(&threadAddr, PTHREAD_CREATE_DETACHED);

        pthread_t thread;
        int ret = pthread_create(&mPthreadIds[i], nullptr, ThreadPool::threadFunc, nullptr);
        if (0 != ret) {
            Logger::E("ThreadPoolExecutor", "ThreadPool create thread error");
            threadNum = -1;
            return;
        }
    }

    /**
     * Put the thread handle in Context for determining if all threads have exited
     */
#ifndef COMO_FUNCTION_SAFETY_RTOS
    ComoContext::gThreadsWorking = (pthread_t*)realloc(ComoContext::gThreadsWorking,
                      sizeof(pthread_t*) * (ComoContext::gThreadsWorkingNum + mThreadNum));
    if (nullptr == ComoContext::gThreadsWorking) {
        Logger::E("ThreadPool", "calloc ComoContext::gThreadsWorking error");
        threadNum = -1;
        return;
    }
#endif
    /**
     * When functional safety calculating, ComoContext::gThreadsWorking is an array.
     */
    for (int i = 0;  i < mThreadNum;  i++) {
        ComoContext::gThreadsWorking[ComoContext::gThreadsWorkingNum++] = mPthreadIds[i];
    }
}

int ThreadPool::addTask(ThreadPoolExecutor::Worker *task)
{
    pthread_mutex_lock(&m_pthreadMutex);

    if (! mWorkerList.Add(task)) {
        return -1;
    }

    pthread_mutex_unlock(&m_pthreadMutex);
    pthread_cond_signal(&m_pthreadCond);
    return 0;
}

int ThreadPool::stopAll()
{
    if (mShutdown) {
        return -1;
    }

    mShutdown = true;
    pthread_cond_broadcast(&m_pthreadCond);

    for (int i = 0; i < mThreadNum; i++) {
        pthread_join(mPthreadIds[i], nullptr);
    }

    free(mPthreadIds);
    mPthreadIds = nullptr;

    pthread_mutex_destroy(&m_pthreadMutex);
    pthread_cond_destroy(&m_pthreadCond);

    return 0;
}

} // namespace como
