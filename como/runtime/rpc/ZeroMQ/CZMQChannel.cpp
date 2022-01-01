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

#include <vector>
#include <unistd.h>
#include <time.h>
#include "zmq.h"
#include "comorpc.h"
#include "CZMQChannel.h"
#include "CZMQParcel.h"
#include "InterfacePack.h"
#include "util/comolog.h"
#include "ComoConfig.h"
#include "CZMQUtils.h"
#include "ThreadPoolZmqActor.h"

namespace como {

// $ ./comouuid --cpp -cityhash como::CZMQChannel
// {0x46ca0640,0x3dc3,0x1fa5,0xc63a,{0x2b,0x62,0xec,0x49,0x7c,0x2f}}
const CoclassID CID_CZMQChannel =
        {{0x46ca0640,0x3dc3,0x1fa5,0xc63a,{0x2b,0x62,0xec,0x49,0x7c,0x2f}}, &CID_COMORuntime};

COMO_INTERFACE_IMPL_1(CZMQChannel, Object, IRPCChannel);

COMO_OBJECT_IMPL(CZMQChannel);

CZMQChannel::CZMQChannel(
    /* [in] */ RPCType type,
    /* [in] */ RPCPeer peer)
    : mType(type)
    , mPeer(peer)
    , mStarted(false)
{
    Mutex::AutoLock lock(mLock);

    std::unordered_map<std::string, ServerNodeInfo*>::iterator iter =
            ComoConfig::ServerNameEndpointMap.find(std::string(mServerName.string()));
    std::string endpoint;
    if (iter != ComoConfig::ServerNameEndpointMap.end()) {
        endpoint = iter->second->endpoint;
    }
    else {
        Logger::E("CZMQChannel", "Unregistered ServerName: %s", mServerName.string());
    }

    if (nullptr != iter->second->socket) {
        mSocket = iter->second->socket;
        iter->second->inChannel++;
    }
    else {
        mSocket = CZMQUtils::CzmqGetSocket(nullptr, ComoConfig::ComoRuntimeInstanceIdentity.c_str(),
                                        ComoConfig::ComoRuntimeInstanceIdentity.size(),
                                        mServerName.string(), endpoint.c_str(), ZMQ_REQ);
        if (nullptr != mSocket) {
            iter->second->socket = mSocket;
            iter->second->inChannel++;
        }
    }
}

ECode CZMQChannel::Apply(
    /* [in] */ IInterfacePack* ipack)
{
    //mName = InterfacePack::From(ipack)->GetDBusName();
    return NOERROR;
}

ECode CZMQChannel::GetRPCType(
    /* [out] */ RPCType& type)
{
    type = mType;
    return NOERROR;
}

ECode CZMQChannel::SetServerName(
    /* [in] */ const String& serverName)
{
    mServerName = serverName;
    return NOERROR;
}

ECode CZMQChannel::GetServerName(
    /* [out] */ String& serverName)
{
    serverName = mServerName;
    return NOERROR;
}

ECode CZMQChannel::IsPeerAlive(
    /* [out] */ Boolean& alive)
{
    ECode ec = NOERROR;
    alive = true;
    return ec;
}

ECode CZMQChannel::LinkToDeath(
    /* [in] */ IProxy* proxy,
    /* [in] */ IDeathRecipient* recipient,
    /* [in] */ HANDLE cookie,
    /* [in] */ Integer flags)
{
    return E_UNSUPPORTED_OPERATION_EXCEPTION;
}

ECode CZMQChannel::UnlinkToDeath(
    /* [in] */ IProxy* proxy,
    /* [in] */ IDeathRecipient* recipient,
    /* [in] */ HANDLE cookie,
    /* [in] */ Integer flags,
    /* [out] */ AutoPtr<IDeathRecipient>* outRecipient)
{
    return E_UNSUPPORTED_OPERATION_EXCEPTION;
}

ECode CZMQChannel::GetComponentMetadata(
    /* [in] */ const CoclassID& cid,
    /* [out, callee] */ Array<Byte>& metadata)
{
    ECode ec = NOERROR;
    HANDLE data;
    Long size;
    AutoPtr<IParcel> parcel;
    CoCreateParcel(RPCType::Remote, parcel);
    parcel->WriteCoclassID(cid);
    parcel->GetData(data);
    parcel->GetDataSize(size);

    int rc = CZMQUtils::CzmqSendBuf(reinterpret_cast<HANDLE>(this),
                    ZmqFunCode::GetComponentMetadata, mSocket, (void *)data, size);
    if (-1 == rc) {
        return E_RUNTIME_EXCEPTION;
    }

    HANDLE hChannel;
    Integer eventCode;
    zmq_msg_t msg;
    rc = CZMQUtils::CzmqRecvMsg(hChannel, eventCode, mSocket, msg, 0);
    if (-1 != rc) {
        if (ZmqFunCode::GetComponentMetadata != eventCode) {
            Logger::E("GetComponentMetadata", "Bad eventCode: %d", eventCode);
            ec = E_RUNTIME_EXCEPTION;
        }
        else {
            void* replyData = zmq_msg_data(&msg);
            Integer replySize = zmq_msg_size(&msg);
            metadata = Array<Byte>::Allocate(replySize);
            if (metadata.IsNull()) {
                Logger::E("GetComponentMetadata", "Malloc %d size metadata failed.", replySize);
                ec = E_OUT_OF_MEMORY_ERROR;
            }
            else {
                memcpy(metadata.GetPayload(), replyData, replySize);
            }
        }
    }
    else {
        Logger::E("GetComponentMetadata", "RCZMQUtils::CzmqRecvMsg().");
    }

    // Release message
    zmq_msg_close(&msg);

    return ec;
}

ECode CZMQChannel::Invoke(
    /* [in] */ IMetaMethod* method,
    /* [in] */ IParcel* argParcel,
    /* [out] */ AutoPtr<IParcel>& resParcel)
{
    ECode ec = NOERROR;
    HANDLE data;
    Long size;

    argParcel->GetData(data);
    argParcel->GetDataSize(size);

    CZMQUtils::CzmqSendBuf(reinterpret_cast<HANDLE>(this), ZmqFunCode::Method_Invoke, mSocket, (void *)data, size);

    HANDLE hChannel;
    Integer eventCode;
    String serverName;
    GetServerName(serverName);
    zmq_msg_t msg;
    void *socket = CZMQUtils::CzmqFindSocket(serverName.string());
    int replySize = CZMQUtils::CzmqRecvMsg(hChannel, eventCode, mSocket, msg, 0);

    if (SUCCEEDED(ec)) {
        resParcel = new CZMQParcel();

        Integer hasOutArgs;
        method->GetOutArgumentsNumber(hasOutArgs);
        if (hasOutArgs) {
            if (replySize > 0) {
                resParcel->SetData(reinterpret_cast<HANDLE>(zmq_msg_data(&msg)), zmq_msg_size(&msg));
            }
        }
    }
    else {
        if (DEBUG) {
            Logger::D("CZMQChannel", "Remote call failed with ec = 0x%x.", ec);
        }
    }

    // Release message
    zmq_msg_close (&msg);

    return ec;
}

ECode CZMQChannel::StartListening(
    /* [in] */ IStub* stub)
{
    int ret = 0;

    if (mPeer == RPCPeer::Stub) {
        AutoPtr<TPZA_Executor::Worker> w = new TPZA_Executor::Worker(this, stub);
        ret = TPZA_Executor::GetInstance()->RunTask(w);
    }

    return NOERROR;
}

} // namespace como
