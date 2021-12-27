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

namespace como {

COMO_INTERFACE_IMPL_1(CZMQChannel, Object, IRPCChannel);

COMO_OBJECT_IMPL(CZMQChannel);

CZMQChannel::CZMQChannel(
    /* [in] */ RPCType type,
    /* [in] */ RPCPeer peer)
    : mType(type)
    , mPeer(peer)
    , mStarted(false)
    , mCond(mLock)
{
    std::string endpoint = ComoConfig::ServerNameEndpointMap.find(mServerName);
    mSocket = CzmqGetSocket(nullptr, ComoConfig::ComoRuntimeInstanceIdentity,
                                    strlen(ComoConfig::ComoRuntimeInstanceIdentity),
                                    mServerName, (const char *)endpoint, ZMQ_REQ);
}

ECode CZMQChannel::Apply(
    /* [in] */ IInterfacePack* ipack)
{
    mName = InterfacePack::From(ipack)->GetDBusName();
    return NOERROR;
}

ECode CZMQChannel::GetRPCType(
    /* [out] */ RPCType& type)
{
    type = mType;
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

    int rc = CzmqSendBuf(GetComponentMetadata, mSocket, data, size);

    Integer eventCode;
    zmq_msg_t msg;
    rc = CzmqRecvMsg(eventCode, mSocket, msg, 0);

    if (-1 != rc) {
        if (ZmqFunCode::GetComponentMetadata != eventCode) {

        }
        else {
            void* replyData = zmq_msg_data(msg);
            Integer replySize = zmq_msg_size(msg);
            metadata = Array<Byte>::Allocate(replySize);
            if (metadata.IsNull()) {
                Logger::E("CZMQChannel", "Malloc %d size metadata failed.", replySize);
                ec = E_OUT_OF_MEMORY_ERROR;
            }
            else {
                memcpy(metadata.GetPayload(), replyData, replySize);
            }
        }
    }
    else {
        Logger::E("CZMQChannel", "Remote call failed with ec = 0x%x.", ec);
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

    CzmqSendBuf(Method_Invoke, socket, data, size);

    Integer eventCode;
    String serverName;
    GetServerName(serverName);
    zmq_msg_t msg;
    void *socket = CZMQUtils::CzmqFindSocket(serverName.string());
    int replySize = CZMQUtils::CzmqRecvMsg(eventCode, mSocket, msg, 0);

    if (SUCCEEDED(ec)) {
        resParcel = new CDBusParcel();

        Integer hasOutArgs;
        method->GetOutArgumentsNumber(hasOutArgs);
        if (hasOutArgs) {
            if (replySize > 0) {
                resParcel->SetData(reinterpret_cast<HANDLE>(zmq_msg_data(msg)), zmq_msg_size(msg));
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
        ret = ThreadPoolExecutor::GetInstance()->RunTask(w);
    }

    if (0 == ret) {
        Mutex::AutoLock lock(mLock);
        while (!mStarted) {
            mCond.Wait();
        }
    }
    return NOERROR;
}

} // namespace como
