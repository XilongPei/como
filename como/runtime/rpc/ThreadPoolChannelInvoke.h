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

#ifndef __COMO_THREADPOOLCHANNELINVOKE_H__
#define __COMO_THREADPOOLCHANNELINVOKE_H__

#include <vector>
#include "comoobj.h"
#include "comotypes.h"
#include "comoref.h"
#include "util/arraylist.h"
#include "util/mutex.h"
#include "comoerror.h"

namespace como {

constexpr ECode FUNCTION_SAFETY_CALL_TIMEOUT = MAKE_COMORT_ECODE(1, 0x1);

class ThreadPoolChannelInvoke;
enum WORKER_STATUS {
    WORKER_IDLE = 0,
    WORKER_TASK_READY,
    WORKER_TASK_RUNNING,
    WORKER_TASK_FINISH
};

// TPCI: Thread Pool Channel Invoke
class TPCI_Executor
    : public LightRefBase
{
public:
    class Worker
        : public Object
    {
    public:
        Worker(AutoPtr<IRPCChannel> channel, AutoPtr<IMetaMethod> method,
                              AutoPtr<IParcel> inParcel, AutoPtr<IParcel> outParcel);

        ECode Invoke();

    public:
        AutoPtr<IRPCChannel> mChannel;
        AutoPtr<IMetaMethod> mMethod;
        AutoPtr<IParcel> mInParcel;
        AutoPtr<IParcel> mOutParcel;
        TPCI_Executor* mOwner;
        pthread_mutex_t mMutex;
        pthread_cond_t mCond;
        struct timespec mCreateTime;
        int mWorkerStatus;
        ECode ec;
    };

public:
    static AutoPtr<TPCI_Executor> GetInstance();

    int RunTask(AutoPtr<IRPCChannel> channel, AutoPtr<IMetaMethod> method,
                                AutoPtr<IParcel> inParcel, AutoPtr<IParcel> outParcel);

    int CleanTask(int posWorkerList);

private:
    static AutoPtr<TPCI_Executor> sInstance;
    static AutoPtr<ThreadPoolChannelInvoke> threadPool;
    static Mutex sInstanceLock;
};

class ThreadPoolChannelInvoke
    : public LightRefBase
{
public:
    static std::vector<TPCI_Executor::Worker*> mWorkerList;     // task list

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
    ThreadPoolChannelInvoke(int threadNum = 10);
    static int addTask(TPCI_Executor::Worker *task);
    static int cleanTask(int posWorkerList);
    int stopAll();
    int getTaskListSize();
};

} // namespace como

#endif // __COMO_THREADPOOLCHANNELINVOKE_H__
