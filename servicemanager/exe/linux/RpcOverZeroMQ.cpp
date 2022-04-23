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
static void *socket = nullptr;

void RpcOverZeroMQ::startTPZA_Executor()
{
    Logger::D("ServiceManager", "starting daemon for RPC over ZeroMQ...");

    // prepare ComoConfig
    std::string ret = ComoConfig::AddZeroMQEndpoint(std::string("localhost"),
                                            std::string("tcp://127.0.0.1:8081"));
    if (std::string("localhost") != ret) {
        Logger::E("startTPZA_Executor", "Failed to AddZeroMQEndpoint\n");
    }

    ret = ComoConfig::AddZeroMQEndpoint(std::string("ServiceManager"),
                                            std::string("tcp://127.0.0.1:8088"));
    if (std::string("ServiceManager") != ret) {
        Logger::E("startTPZA_Executor", "Failed to AddZeroMQEndpoint\n");
    }

    // end of prepare ComoConfig

    std::string strep = ComoConfig::GetZeroMQEndpoint("ServiceManager");

    socket = CZMQUtils::CzmqGetSocket(nullptr,
                                ComoConfig::ComoRuntimeInstanceIdentity.c_str(),
                                ComoConfig::ComoRuntimeInstanceIdentity.size(),
                                "ServiceManager", strep.c_str(),
                                ZMQ_REP);
    if (nullptr == socket) {
        Logger::E("startTPZA_Executor",
                  "CzmqGetSocket error, Server: 'ServiceManager', Endpoint: %s",
                  strep.c_str());
    }

    int timeout = -1;  // milliseconds
    zmq_setsockopt(socket, ZMQ_RCVTIMEO, &timeout, sizeof(int));

    Logger::D("startTPZA_Executor", "Server: 'ServiceManager', Endpoint: %s",
                                                                 strep.c_str());
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
    ECode ec;
    HANDLE hChannel;
    Integer eventCode;
    zmq_msg_t msg;

    while (true) {
        ec = NOERROR;
        if (CZMQUtils::CzmqRecvMsg(hChannel, eventCode, socket, msg, 0) > 0) {
            ec = HandleMessage(hChannel, eventCode, socket, msg);
        }

        // In the service program, the complete recv() and send() must appear in
        // pairs. Similarly, in the client program, after send(), there should
        // also be recv(). This rule is guaranteed by HandleMessage()

        if (FAILED(ec)) {
            Logger::E("RpcOverZeroMQ::threadFunc", "ECode: 0x%X", ec);
            usleep(200);
        }
    }

    return reinterpret_cast<void*>(ec);
}

ECode RpcOverZeroMQ::HandleMessage(HANDLE hChannel, Integer eventCode,
                                                   void *socket, zmq_msg_t& msg)
{
    ECode ec = NOERROR;
    Integer rc;
    void *data = zmq_msg_data(&msg);
    size_t size = zmq_msg_size(&msg);

    switch (eventCode) {
        case ZmqFunCode::AddService: {      // 0x0201
            AutoPtr<IParcel> parcel;
            ServiceManager::InterfacePack ipack;

            CoCreateParcel(RPCType::Remote, parcel);
            parcel->SetData(reinterpret_cast<HANDLE>(data), size);

            // Keep the same order with InterfacePack::WriteToParcel() in
            // como/runtime/rpc/ZeroMQ/InterfacePack.cpp
            parcel->ReadCoclassID(ipack.mCid);
            parcel->ReadInterfaceID(ipack.mIid);
            parcel->ReadBoolean(ipack.mIsParcelable);
            parcel->ReadLong(ipack.mServerObjectId);

            String serverName, name;
            parcel->ReadString(serverName);
            Integer pos = serverName.IndexOf('\n', 0);
            if (pos < 0) {
                Logger::E("RpcOverZeroMQ",
                  "HandleMessage() AddService, bad serverName: %s", serverName);
                zmq_msg_close(&msg);
                break;
            }

            ipack.mServerName = serverName.Substring(0, pos);

            // 4096, any big number, the max length of serverName
            // String::GetLength() cost time
            name = serverName.Substring(pos+1, 4096);

            ec = ServiceManager::GetInstance()->AddService(name, ipack);

            rc = CZMQUtils::CzmqSendBuf(reinterpret_cast<HANDLE>(nullptr),
                                         ZmqFunCode::AnswerECode,
                                         socket, (const void *)&ec, sizeof(ec));
            if (rc <= 0)
                usleep(100);

            zmq_msg_close(&msg);
            break;
        }
        case ZmqFunCode::GetService: {      // 0x0202
            ServiceManager::InterfacePack* ipack = nullptr;
            ECode ec = NOERROR;
            HANDLE resData = reinterpret_cast<HANDLE>("");
            Long resSize = 0;
            AutoPtr<IParcel> parcel;

            ec = ServiceManager::GetInstance()->GetService((char*)data, &ipack);

            if (FAILED(ec))
                goto HandleMessage_Exit;

            if (nullptr != ipack) {
                if (! ipack->mServerName.IsEmpty()) {
                    // Keep the same order with InterfacePack::ReadFromParcel() in
                    // como/runtime/rpc/ZeroMQ/InterfacePack.cpp
                    ec = CoCreateParcel(RPCType::Remote, parcel);
                    if (FAILED(ec))
                        goto HandleMessage_Exit;

                    parcel->WriteCoclassID(ipack->mCid);
                    parcel->WriteInterfaceID(ipack->mIid);
                    parcel->WriteBoolean(ipack->mIsParcelable);
                    parcel->WriteLong(ipack->mServerObjectId);
                    parcel->WriteString(ipack->mServerName);
                }
                parcel->GetData(resData);
                parcel->GetDataSize(resSize);
            }

        HandleMessage_Exit:
            rc = CZMQUtils::CzmqSendBuf(reinterpret_cast<HANDLE>(nullptr),
                                        ZmqFunCode::AnswerECode,
                                        socket, (const void *)resData, resSize);
            if (rc <= 0)
                usleep(100);

            break;
        }
        case ZmqFunCode::RemoveService: {   // 0x0203
            ec = ServiceManager::GetInstance()->RemoveService((char*)data);

            rc = CZMQUtils::CzmqSendBuf(reinterpret_cast<HANDLE>(nullptr),
                                         ZmqFunCode::AnswerECode,
                                         socket, (const void *)&ec, sizeof(ec));
            if (rc <= 0)
                usleep(100);

            break;
        }
        default:
            ec = E_ZMQ_FUN_CODE_ERROR;
            rc = CZMQUtils::CzmqSendBuf(reinterpret_cast<HANDLE>(nullptr),
                                         ZmqFunCode::AnswerECode,
                                         socket, (const void *)&ec, sizeof(ec));
            if (rc <= 0)
                usleep(100);

            Logger::E("RpcOverZeroMQ", "HandleMessage() unknown eventCode");
    }

    return NOERROR;
}

} // namespace jing
