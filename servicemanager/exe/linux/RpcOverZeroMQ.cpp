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
#include "ComoConfig.h"
#include "CZMQUtils.h"
#include "ThreadPoolZmqActor.h"
#include "RpcOverZeroMQ.h"

namespace jing {

void *threadFunc(void *threadData);

void RpcOverZeroMQ::startTPZA_Executor()
{
    Logger_D("ServiceManager", "starting daemon for RPC over ZeroMQ...");

    AutoPtr<TPZA_Executor> tPZA_Executor = TPZA_Executor::GetInstance();
    tPZA_Executor->SetDefaultHandleMessage(HandleMessage);

    pthread_attr_t threadAddr;
    pthread_attr_init(&threadAddr);
    pthread_attr_setdetachstate(&threadAddr, PTHREAD_CREATE_DETACHED);
    pthread_t pthread_id;
    if (pthread_create(&pthread_id, nullptr, threadFunc, nullptr) != 0) {
        Logger::E("ThreadPoolZmqActor", "pthread_create() error");
    }

}

void *RpcOverZeroMQ::threadFunc(void *threadData)
{
    ECode ec = NOERROR;
    int i;

    while (true) {
        HANDLE hChannel;
        Integer eventCode;
        zmq_msg_t msg;
        int numberOfBytes;
        void *socket = nullptr;

        std::unordered_map<std::string, ServerNodeInfo*>::iterator iter =
                                                ComoConfig::ServerNameEndpointMap.begin();
        while (iter != ComoConfig::ServerNameEndpointMap.end()) {
            if (iter->second->inChannel <= 0) {
                socket = iter->second->socket;
                if (nullptr == socket) {
                    socket = CZMQUtils::CzmqGetSocket(nullptr,
                                            ComoConfig::ComoRuntimeInstanceIdentity.c_str(),
                                            ComoConfig::ComoRuntimeInstanceIdentity.size(),
                                            iter->first.c_str(), iter->second->endpoint.c_str(),
                                            ZMQ_REQ);
                    iter->second->socket = socket;
                }
            }
            else
                continue;

            if (nullptr == socket)
                continue;

            if (CZMQUtils::CzmqRecvMsg(hChannel, eventCode, socket, msg, ZMQ_DONTWAIT) != 0) {
                ec = HandleMessage(hChannel, eventCode, socket, msg);
            }
        }
   }

    return reinterpret_cast<void*>(ec);
}

ECode RpcOverZeroMQ::HandleMessage(HANDLE hChannel, Integer eventCode, void *socket, zmq_msg_t& msg)
{
    static int i = 0;
    printf("------ %d --\n", i++);
    usleep(100000);

    //pthread_cond_signal(&ThreadPoolZmqActor::pthreadCond);
    //pthread_cond_broadcast(&ThreadPoolZmqActor::pthreadCond);

    return NOERROR;
}

} // namespace jing
