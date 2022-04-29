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
#include "zmq.h"
#include "comoobj.h"
#include "comotypes.h"
#include "comoref.h"
#include "util/arraylist.h"
#include "util/mutex.h"
#include "comoerror.h"
#include "CZMQChannel.h"

namespace como {

// If the message has been processed, returned 0
using HANDLE_MESSAGE_FUNCTION = ECode(*)(HANDLE,Integer,void *,zmq_msg_t&);

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
    WORKER_TASK_READY,
    WORKER_TASK_RUNNING,
    WORKER_TASK_DAEMON_RUNNING,
    WORKER_TASK_FINISH
};

// TPZA: Thread Pool ZeroMQ Actor
class TPZA_Executor
    : public LightRefBase
{
public:
    class Worker
        : public Object
    {
    public:
        Worker(CZMQChannel *channel, AutoPtr<IStub> stub, std::string& endpoint);
        ~Worker();

        TPZA_Executor::Worker *HandleMessage();

    public:
        void *mSocket;
        std::string mEndpoint;
        Long mChannel;
        AutoPtr<IStub> mStub;
        struct timespec lastAccessTime;
        int mWorkerStatus;
    };

public:
    static AutoPtr<TPZA_Executor> GetInstance();

    int RunTask(
        /* [in] */ AutoPtr<TPZA_Executor::Worker> task);

    int CleanTask(int posWorkerList);

    int SetDefaultHandleMessage(HANDLE_MESSAGE_FUNCTION func);

    static HANDLE_MESSAGE_FUNCTION defaultHandleMessage;

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
    static bool signal_;

    static TPZA_Executor::Worker *PickWorkerByChannelHandle(
                                                HANDLE hChannel, bool isDaemon);
    static int CountWorkerBySocket(void *socket);

private:
    static bool shutdown;
    int mThreadNum;                                             // most thread number
    pthread_t *pthread_id;

public:
    static void *threadFunc(void *threadData);
    static pthread_mutex_t pthreadMutex;
    static pthread_cond_t pthreadCond;

    ThreadPoolZmqActor(int threadNum = 10);

    static int AddTask(
        /* [in] */ AutoPtr<TPZA_Executor::Worker> task);

    static int CleanTask(int posWorkerList);
    static ECode CleanWorkerByChannelHandle(HANDLE hChannel);
    int StopAll();
    int GetTaskListSize();
};

} // namespace como

#endif // __COMO_THREADPOOLZMQACTOR_H__
