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
#include "ServiceManager.h"
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
    ECode ec = NOERROR;
    switch (eventCode) {
        case ZmqFunCode::AddService: {      // 0x0201
            void *data = zmq_msg_data(&msg);
            size_t size = zmq_msg_size(&msg);

            AutoPtr<IParcel> parcel;
            ServiceManager::InterfacePack ipack;

            CoCreateParcel(RPCType::Remote, parcel);
            parcel->SetData(reinterpret_cast<HANDLE>(data), size);

            // Keep the same order with InterfacePack::WriteToParcel() in
            // como/runtime/rpc/ZeroMQ/InterfacePack.cpp
            parcel->ReadCoclassID(ipack.mCid);
            parcel->ReadInterfaceID(ipack.mIid);
            parcel->ReadBoolean(ipack.mIsParcelable);

            String serverName, name;
            parcel->ReadString(serverName);
            Integer pos = serverName.IndexOf('\n', 0);
            if (pos < 0) {
                Logger::E("RpcOverZeroMQ", "HandleMessage() AddService, bad serverName: %s", serverName);
                zmq_msg_close (&msg);
                break;
            }

            ipack.mServerName = serverName.Substring(0, pos);
            name = serverName.Substring(pos+1, 4096);   // 4096, any big number, the max length of serverName
                                                        // String::GetLength() cost time

            ec = ServiceManager::GetInstance()->AddService(name, ipack);

            zmq_msg_close (&msg);
            break;
        }
        case ZmqFunCode::GetService: {      // 0x0202
            break;
        }
        case ZmqFunCode::RemoveService: {   // 0x0203
            break;
        }
        default:
            Logger::E("RpcOverZeroMQ", "HandleMessage() error");
    }

    return NOERROR;
}

} // namespace jing
