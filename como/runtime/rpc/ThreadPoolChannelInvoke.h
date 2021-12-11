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

#include "comotypes.h"
#include "comoref.h"
#include "util/arraylist.h"
#include "util/mutex.h"

namespace como {

// ns, 30s
#define TPCI_TASK_EXPIRES   (30*1.0e9)
#define FUNCTION_SAFETY_CALL_TIMEOUT    1

class ThreadPoolChannelInvoke;

// TPCI: Thread Pool Channel Invoke
class TPCI_Executor
    : public LightRefBase
{
public:
    class Worker
        : public LightRefBase
    {
    public:
        Worker(AutoPtr<IRPCChannel> channel, mChannel, AutoPtr<IMetaMethod> method,
                              AutoPtr<IParcel> inParcel, AutoPtr<IParcel> outParcel);

        ECode Invoke();

    public:
        AutoPtr<IRPCChannel> mChannel,
        AutoPtr<IMetaMethod> mMethod;
        AutoPtr<IParcel> mInParcel;
        AutoPtr<IParcel> mOutParcel;
        TPCI_Executor* mOwner;
        Mutex mLock;
        struct timespec mCreateTime;
        ECode ec;
    };

public:
    static AutoPtr<TPCI_Executor> GetInstance();

    int RunTask(AutoPtr<IMetaMethod> method, AutoPtr<IParcel> inParcel, AutoPtr<IParcel> outParcel);

private:
    static AutoPtr<TPCI_Executor> sInstance;
    static AutoPtr<ThreadPoolChannelInvoke> threadPool;
    static Mutex sInstanceLock;
};

class ThreadPoolChannelInvoke
    : public LightRefBase
{
private:
    static ArrayList<TPCI_Executor::Worker*> mWorkerList;      // task list
    static bool shutdown;
    int mThreadNum;                                            // most thread number
    pthread_t *pthread_id;

    static pthread_mutex_t m_pthreadMutex;
    static pthread_cond_t m_pthreadCond;

protected:
    static void *threadFunc(void *threadData);

    int create();

public:
    ThreadPoolChannelInvoke(int threadNum = 10);
    static int addTask(TPCI_Executor::Worker *task);
    int stopAll();
    int getTaskSize();
};

} // namespace como

#endif // __COMO_THREADPOOLCHANNELINVOKE_H__
