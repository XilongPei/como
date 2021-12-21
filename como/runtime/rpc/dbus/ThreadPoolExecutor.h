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

#ifndef __COMO_THREADPOOLEXECUTOR_H__
#define __COMO_THREADPOOLEXECUTOR_H__

#include "comotypes.h"
#include "comoref.h"
#include "util/arraylist.h"
#include "util/mutex.h"

namespace como {

class ThreadPool;

class ThreadPoolExecutor
    : public LightRefBase
{
public:
    class Runnable
        : public LightRefBase
    {
    public:
        virtual ECode Run() = 0;
    };

public:
    class Worker
        : public LightRefBase
    {
    public:
        Worker(
            /* [in] */ Runnable* task,
            /* [in] */ ThreadPoolExecutor* owner);

        ECode Run();

    public:
        AutoPtr<Runnable> mTask;
        ThreadPoolExecutor* mOwner;
        Mutex mLock;
    };

public:
    static AutoPtr<ThreadPoolExecutor> GetInstance();

    ECode RunTask(
        /* [in] */ Runnable* task);

private:
    static AutoPtr<ThreadPoolExecutor> sInstance;
    static AutoPtr<ThreadPool> threadPool;
    static Mutex sInstanceLock;
};

class ThreadPool
    : public LightRefBase
{
private:
    static ArrayList<ThreadPoolExecutor::Worker*> mWorkerList;      // task list
    static bool shutdown;
    int mThreadNum;                                                 // most thread number
    pthread_t *pthread_ids;

    static pthread_mutex_t m_pthreadMutex;
    static pthread_cond_t m_pthreadCond;

protected:
    static void *threadFunc(void *threadData);

    int create();

public:
    ThreadPool(int threadNum = 10);
    static int addTask(ThreadPoolExecutor::Worker *task);
    int stopAll();
    int getTaskSize();
};

} // namespace como

#endif // __COMO_THREADPOOLEXECUTOR_H__
