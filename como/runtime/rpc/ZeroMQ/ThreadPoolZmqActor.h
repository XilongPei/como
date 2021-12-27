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

#ifndef __COMO_THREADPOOLZMQACTOR_H__
#define __COMO_THREADPOOLZMQACTOR_H__

#include <vector>
#include "comoobj.h"
#include "comotypes.h"
#include "comoref.h"
#include "util/arraylist.h"
#include "util/mutex.h"
#include "comoerror.h"

namespace como {

constexpr ECode FUNCTION_SAFETY_CALL_TIMEOUT = MAKE_COMORT_ECODE(1, 0x1);

/*
+==ThreadPoolZmqActor==+== thread pool
|                      |
| TPZA_Executor ^^^^^^^|^^ thread
|                      |
| TPZA_Executor (1)  --+----> Worker ... Worker ... Worker ......
| TPZA_Executor (2)  --+----> Worker ... Worker ... Worker ......
|                      |
|       ......         |
|                      |
+======================+
*/

class ThreadPoolZmqActor;
enum WORKER_STATUS {
    WORKER_IDLE = 0,
    WORKER_TASK_READY,
    WORKER_TASK_RUNNING,
    WORKER_TASK_FINISH
};

// TPZA: Thread Pool Channel Invoke
class TPZA_Executor
    : public LightRefBase
{
public:
    class Worker
        : public Object
    {
    public:
        Worker(AutoPtr<CZMQChannel> channel, AutoPtr<IStub> stub);

        ECode Invoke();

    public:
        TPZA_Executor* mOwner;
        void *mSocket;
        AutoPtr<CZMQChannel> mChannel;
        AutoPtr<IStub> mStub;
        pthread_mutex_t mMutex;
        pthread_cond_t mCond;
        struct timespec lastAccessTime;
        int mWorkerStatus;
        ECode ec;
    };

public:
    static AutoPtr<TPZA_Executor> GetInstance();

    int RunTask(AutoPtr<IRPCChannel> channel, thisObj,
                                AutoPtr<IParcel> inParcel, AutoPtr<IParcel> outParcel);

    int CleanTask(int posWorkerList);

private:
    static AutoPtr<TPZA_Executor> sInstance;
    static AutoPtr<ThreadPoolZmqActor> threadPool;
    static Mutex sInstanceLock;
};

class ThreadPoolZmqActor
    : public LightRefBase
{
public:
    static std::vector<TPZA_Executor::Worker*> mWorkerList;     // task list

private:
    static bool shutdown;
    int mThreadNum;                                             // most thread number
    pthread_t *pthread_id;

    static pthread_mutex_t m_pthreadMutex;
    static pthread_cond_t m_pthreadCond;

protected:
    static void *threadFunc(void *threadData);

    int create();

public:
    ThreadPoolZmqActor(int threadNum = 10);
    static int addTask(TPZA_Executor::Worker *task);
    static int cleanTask(int posWorkerList);
    int stopAll();
    int getTaskListSize();
};

} // namespace como

#endif // __COMO_THREADPOOLZMQACTOR_H__
