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

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "comolog.h"
#include "ThreadPoolZmqActor.h"
#include "RpcOverZeroMQ.h"

namespace jing {

void RpcOverZeroMQ::startTPZA_Executor()
{
    Logger_D("ServiceManager", "starting daemon for RPC over ZeroMQ...");

    AutoPtr<TPZA_Executor> tPZA_Executor = TPZA_Executor::GetInstance();
    tPZA_Executor->SetDefaultHandleMessage(HandleMessage);
}

ECode RpcOverZeroMQ::HandleMessage()
{
    static int i = 0;
    printf("------ %d --\n", i++);
    usleep(100000);

    //pthread_cond_signal(&ThreadPoolZmqActor::pthreadCond);
    //pthread_cond_broadcast(&ThreadPoolZmqActor::pthreadCond);

    return NOERROR;
}

} // namespace jing
