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
#include <string>
#include <unistd.h>
#include <time.h>
#include "zmq.h"
#include "ComoConfig.h"
#include "comorpc.h"
#include "CZMQChannel.h"
#include "CZMQParcel.h"
#include "CZMQInterfacePack.h"
#include "util/comolog.h"
#include "util/mistring.h"
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
    , mServerObjectId(0)
    , mPubSocket(0)
{
    mEndpoint = ComoConfig::localhostInprocEndpoint;
}

CMemPool *CZMQChannel::memPool = new CMemPool(nullptr, ComoConfig::POOL_SIZE_Channel,
                                              sizeof(CZMQChannel));
void *CZMQChannel::MemPoolAlloc(size_t ulSize)
{
    return memPool->Alloc(ulSize, MUST_USE_MEM_POOL);
}

void CZMQChannel::MemPoolFree(Short id, const void *p)
{
    memPool->Free((void *)p);
}

ECode CZMQChannel::Apply(
    /* [in] */ IInterfacePack* ipack)
{
    // DBus distinguishes channels by name, ZeroMQ channels have no name
    //mName = CZMQInterfacePack::From(ipack)->GetDBusName();
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

ECode CZMQChannel::SetServerObjectId(
        /* [out] */ Long serverObjectId)
{
    mServerObjectId = serverObjectId;
    return NOERROR;
}

ECode CZMQChannel::GetServerObjectId(
    /* [out] */ Long& serverObjectId)
{
    serverObjectId = mServerObjectId;
    return NOERROR;
}

ECode CZMQChannel::IsPeerAlive(
    /* [in, out] */ Long& lvalue,
    /* [out] */ Boolean& alive)
{
    /**
     * 1. Find out who asked me for IsPeerAlive and establish a socket connection with it.
     * 2. 'mSocket' is used to process requests, not to communicate with client
     */
    void *socket = CZMQUtils::CzmqGetSocket(nullptr, mServerName, ZMQ_REQ);
    if (nullptr == socket) {
        Logger::E("CZMQChannel::ReleasePeer",
                  "CzmqGetSocket error, endpoint: %s", mServerName);
        return E_RUNTIME_EXCEPTION;
    }

    Logger::D("CZMQChannel::ReleasePeer",
              "Try to CzmqSendBuf to endpoint %s", mServerName.string());
    int rc = CZMQUtils::CzmqSendBuf(mServerObjectId,
                                        ZmqFunCode::Actor_IsPeerAlive, socket,
                                                 (void *)&lvalue, sizeof(Long));
    if (rc <= 0) {
        lvalue = rc;
        alive = false;
        return E_RUNTIME_EXCEPTION;
    }

    ECode ec;
    HANDLE hChannel;
    zmq_msg_t msg;
    rc = CZMQUtils::CzmqRecvMsg(hChannel, ec, socket, msg, 0);
    if (rc > 0) {
        #pragma pack(1)
        struct PeerState {
            Long state;
            Boolean alive;
        };
        #pragma pack()
        PeerState* peerState;

        if (SUCCEEDED(ec) && (zmq_msg_size(&msg) == sizeof(PeerState))) {
            peerState = (PeerState*)zmq_msg_data(&msg);
            lvalue = peerState->state;
            alive = peerState->alive;
        }
        else {
            lvalue = rc;
            alive = false;
            Logger::E("CZMQChannel::ReleasePeer",
                    "ObjectId: 0x%llX, Fail, ECode: 0x%X", mServerObjectId, ec);
        }
    }
    else {
        lvalue = rc;
        alive = false;
        ec = ZMQ_BAD_REPLY_DATA;
        Logger::E("CZMQChannel::ReleasePeer", "RCZMQUtils::CzmqRecvMsg().");
    }

    if ((rc < -1) || (rc > 0)) {
        // Release message
        zmq_msg_close(&msg);
    }

    return ec;
}

ECode CZMQChannel::ReleasePeer(
    /* [out] */ Boolean& alive)
{
    /**
     * 1. Find out who asked me for ReleasePeer and establish a socket connection with it.
     * 2. 'mSocket' is used to process requests, not to communicate with client
     */
    void *socket = CZMQUtils::CzmqGetSocket(nullptr, mServerName, ZMQ_REQ);
    if (nullptr == socket) {
        Logger::E("CZMQChannel::ReleasePeer",
                  "CzmqGetSocket error, endpoint: %s", mServerName);
        return E_RUNTIME_EXCEPTION;
    }

    Logger::D("CZMQChannel::ReleasePeer",
              "Try to CzmqSendBuf to endpoint %s", mServerName.string());
    int rc = CZMQUtils::CzmqSendBuf(mServerObjectId,
                                        ZmqFunCode::ReleasePeer, socket,
                                        (void *)&mServerObjectId, sizeof(Long));
    if (rc <= 0) {
        return E_RUNTIME_EXCEPTION;
    }

    alive = true;

    ECode ec;
    HANDLE hChannel;
    zmq_msg_t msg;
    rc = CZMQUtils::CzmqRecvMsg(hChannel, ec, socket, msg, 0);
    if (rc > 0) {
        if (FAILED(ec)) {
            alive = false;
            Logger::E("CZMQChannel::ReleasePeer",
                    "ObjectId: 0x%llX, Fail, ECode: 0x%X", mServerObjectId, ec);
        }
    }
    else {
        alive = false;
        ec = ZMQ_BAD_REPLY_DATA;
        Logger::E("CZMQChannel::ReleasePeer", "RCZMQUtils::CzmqRecvMsg().");
    }

    if ((rc < -1) || (rc > 0)) {
        // Release message
        zmq_msg_close(&msg);
    }

    return ec;
}

ECode CZMQChannel::ReleaseObject(
    /* [in] */ Long objectId)
{
    /**
     * 1. Find out who asked me for ReleaseObject and establish a socket connection with it.
     * 2. 'mSocket' is used to process requests, not to communicate with client
     */
    void *socket = CZMQUtils::CzmqGetSocket(nullptr, mServerName, ZMQ_REQ);
    if (nullptr == socket) {
        Logger::E("CZMQChannel::ReleaseObject",
                  "CzmqGetSocket error, endpoint: %s", mServerName);
        return E_RUNTIME_EXCEPTION;
    }

    Logger::D("CZMQChannel::ReleaseObject",
              "Try to CzmqSendBuf to endpoint %s", mServerName.string());
    int rc = CZMQUtils::CzmqSendBuf(mServerObjectId,
                                             ZmqFunCode::Object_Release, socket,
                                               (void *)&objectId, sizeof(Long));
    if (rc <= 0) {
        return E_RUNTIME_EXCEPTION;
    }

    ECode ec;
    HANDLE hChannel;
    zmq_msg_t msg;
    rc = CZMQUtils::CzmqRecvMsg(hChannel, ec, socket, msg, 0);
    if (rc > 0) {
        if (FAILED(ec)) {
            Logger::E("CZMQChannel::ReleaseObject",
                      "ObjectId: 0x%llX, Fail, ECode: 0x%X", objectId, ec);
        }
    }
    else {
        ec = ZMQ_BAD_REPLY_DATA;
        Logger::E("CZMQChannel::ReleaseObject", "RCZMQUtils::CzmqRecvMsg().");
    }

    if ((rc < -1) || (rc > 0)) {
        // Release message
        zmq_msg_close(&msg);
    }

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

    /**
     * 1. Find out who asked me for GetComponentMetadata and establish a socket
     * connection with it.
     * 2. 'mSocket' is used to process requests, not to communicate with client
     */
    void *socket = CZMQUtils::CzmqGetSocket(nullptr, mServerName, ZMQ_REQ);
    if (nullptr == socket) {
        Logger::E("CZMQChannel::GetComponentMetadata",
                  "CzmqGetSocket error, endpoint: %s", mServerName);
        return E_RUNTIME_EXCEPTION;
    }

    Logger::D("CZMQChannel::GetComponentMetadata",
              "Try to CzmqSendBuf to endpoint %s", mServerName.string());
    int rc = CZMQUtils::CzmqSendBuf(mServerObjectId,
                  ZmqFunCode::GetComponentMetadata, socket, (void *)data, size);
    if (rc <= 0) {
        return E_RUNTIME_EXCEPTION;
    }

    HANDLE hChannel;
    ECode eventCode;
    zmq_msg_t msg;
    rc = CZMQUtils::CzmqRecvMsg(hChannel, eventCode, socket, msg, 0);
    if (rc > 0) {
        if (FAILED(eventCode)) {
            Logger::E("CZMQChannel::GetComponentMetadata",
                      "Bad eventCode: %d", eventCode);
            ec = E_RUNTIME_EXCEPTION;
        }
        else {
            void* replyData = zmq_msg_data(&msg);
            Integer replySize = zmq_msg_size(&msg);
            metadata = Array<Byte>::Allocate(replySize);
            if (metadata.IsNull()) {
                Logger::E("CZMQChannel::GetComponentMetadata",
                          "Malloc %d size metadata failed.", replySize);
                ec = E_OUT_OF_MEMORY_ERROR;
            }
            else {
                memcpy(metadata.GetPayload(), replyData, replySize);
            }
        }
    }
    else {
        ec = ZMQ_BAD_REPLY_DATA;
        Logger::E("CZMQChannel::GetComponentMetadata",
                  "RCZMQUtils::CzmqRecvMsg().");
    }

    if ((rc < -1) || (rc > 0)) {
        // Release message
        zmq_msg_close(&msg);
    }

    return ec;
}

ECode CZMQChannel::Invoke(
    /* [in] */ IMetaMethod* method,
    /* [in] */ IParcel* argParcel,
    /* [out] */ AutoPtr<IParcel>& resParcel)
{
    ECode ec;

    /**
     * 1. Find out who asked me for Invoke and establish a socket connection with it.
     * 2. 'mSocket' is used to process requests, not to communicate with client
     */
    if (mServerName.IsEmpty()) {
        Logger::E("CZMQChannel::Invoke", "serverName error");
        return E_RUNTIME_EXCEPTION;
    }

    void *socket = CZMQUtils::CzmqGetSocket(nullptr, mServerName.string(), ZMQ_REQ);
    if (nullptr == socket) {
        Logger::E("CZMQChannel::Invoke",
                  "CzmqGetSocket error, channel endpoint: %s",
                  mServerName.string());
        return E_RUNTIME_EXCEPTION;
    }

    Logger::D("CZMQChannel::Invoke",
              "Try to CzmqSendBuf to endpoint %s", mServerName.string());

    HANDLE data;
    Long size;
    argParcel->GetData(data);
    argParcel->GetDataSize(size);
    int rc = CZMQUtils::CzmqSendBuf(mServerObjectId,
                         ZmqFunCode::Method_Invoke, socket, (void *)data, size);
    if (rc <= 0) {
        return E_RUNTIME_EXCEPTION;
    }

    /**
     * It's time to send a request to the server, and if there are redundant
     * computing requirements, I broadcast the request.
     */
    if (0 != mPubSocket) {
        rc = CZMQUtils::CzmqSendBuf(mServerObjectId, ZmqFunCode::Method_Invoke,
                                        (void *)mPubSocket, (void *)data, size);
        if (rc <= 0) {
            return E_RUNTIME_EXCEPTION;
        }
    }

    HANDLE hChannel;
    zmq_msg_t msg;
    rc = CZMQUtils::CzmqRecvMsg(hChannel, ec, socket, msg, 0);

    if (rc > 0) {
        if (SUCCEEDED(ec)) {
#ifdef COMO_FUNCTION_SAFETY_RTOS
            void *buf = CZMQParcel::MemPoolAlloc(sizeof(CZMQParcel));
            if (nullptr == buf) {
                zmq_msg_close(&msg);
                return E_OUT_OF_MEMORY_ERROR;
            }

            CZMQParcel *czmqParcel = new(buf) CZMQParcel();
            resParcel = czmqParcel;
            czmqParcel->SetFunFreeMem(CZMQParcel::MemPoolFree, 0);
#else
            resParcel = new CZMQParcel();
#endif
            // When COMO_FUNCTION_SAFETY_RTOS, this judgment is redundant
            if (nullptr != resParcel) {
                Integer hasOutArgs;
                method->GetOutArgumentsNumber(hasOutArgs);
                if (hasOutArgs != 0) {
                    resParcel->SetData(reinterpret_cast<HANDLE>(zmq_msg_data(&msg)),
                                                                zmq_msg_size(&msg));
                }
            }
            else {
                Logger::E("CZMQChannel::Invoke", "new CZMQParcel failed.");
                ec = E_OUT_OF_MEMORY_ERROR;
            }
        }
        else {
            Logger::D("CZMQChannel::Invoke",
                      "Remote call failed with ec = 0x%X.", ec);
        }
    }

    if ((rc < -1) || (rc > 0)) {
        // Release message
        zmq_msg_close(&msg);
    }

    return ec;
}

ECode CZMQChannel::StartListening(
    /* [in] */ IStub* stub)
{
    if (mPeer == RPCPeer::Stub) {
        std::string endpoint = GetEndpoint();
        AutoPtr<TPZA_Executor::Worker> worker = new TPZA_Executor::Worker(
                                                          this, stub, endpoint);
        if (nullptr == worker) {
            Logger::E("CZMQChannel::StartListening",
                                            "new TPZA_Executor::Worker failed");
            return E_OUT_OF_MEMORY_ERROR;
        }

        Logger::D("CZMQChannel::StartListening",
                                             "endpoint: %s", mEndpoint.c_str());

        /**
         * TPZA_Executor::GetInstance() just takes the variable that has already
         * been initialized, there is no need to determine whether it is nullptr.
         */
        if (TPZA_Executor::GetInstance()->RunTask(worker) < 0) {
            Logger::E("CZMQChannel::StartListening",
                                                "Too much Task in WorkerList");
            return E_OUT_OF_MEMORY_ERROR;
        }
    }

    return NOERROR;
}

ECode CZMQChannel::Match(
    /* [in] */ IInterfacePack* ipack,
    /* [out] */ Boolean& matched)
{
    IZMQInterfacePack* idpack = IZMQInterfacePack::Probe(ipack);
    if (idpack != nullptr) {
        IInterfacePack* pack = (IInterfacePack*)idpack;
        Long serverObjectId;

        // which server, the same machine, ServerAddress is nullptr
        // IZMQInterfacePack::mServerName;
        // which object
        // IZMQInterfacePack::mServerObjectId;

        pack->GetServerObjectId(serverObjectId);
        if (serverObjectId == mServerObjectId) {
            matched = true;
            return NOERROR;
        }
    }
    matched = false;
    return NOERROR;
}

ECode CZMQChannel::MonitorRuntime(
    /* [in] */ const Array<Byte>& request,
    /* [out] */ Array<Byte>& response)
{
    /**
     * 1. Find out who asked me for ReleaseObject and establish a socket connection with it.
     * 2. 'mSocket' is used to process requests, not to communicate with client
     */
    void *socket = CZMQUtils::CzmqGetSocket(nullptr, mServerName, ZMQ_REQ);
    if (nullptr == socket) {
        Logger::E("CZMQChannel::MonitorRuntime",
                  "CzmqGetSocket error, endpoint: %s", mServerName);
        return E_RUNTIME_EXCEPTION;
    }

    HANDLE resData = reinterpret_cast<HANDLE>(request.GetPayload());
    Long resSize = request.GetLength() + 1;

    Logger::D("CZMQChannel::MonitorRuntime",
              "Try to CzmqSendBuf to endpoint %s", mServerName.string());
    int rc = CZMQUtils::CzmqSendBuf(mServerObjectId,
                                             ZmqFunCode::RuntimeMonitor, socket,
                                                      (void *)resData, resSize);
    if (rc <= 0) {
        return E_RUNTIME_EXCEPTION;
    }

    ECode ec;
    HANDLE hChannel;
    zmq_msg_t msg;
    rc = CZMQUtils::CzmqRecvMsg(hChannel, ec, socket, msg, 0);
    if (rc > 0) {
        if (FAILED(ec)) {
            Logger::E("CZMQChannel::MonitorRuntime", "ECode: 0x%X", ec);
        }
        else {
            if (zmq_msg_size(&msg) > 0) {
                response = Array<Byte>::Allocate(zmq_msg_size(&msg));
                response.Copy((Byte*)zmq_msg_data(&msg), zmq_msg_size(&msg));
            }
        }
    }
    else {
        Logger::E("CZMQChannel::MonitorRuntime", "RCZMQUtils::CzmqRecvMsg().");
    }

    if ((rc < -1) || (rc > 0)) {
        // Release message
        zmq_msg_close(&msg);
    }

    return ec;
}

ECode CZMQChannel::SetPubSocket(
    /* [in] */ HANDLE pubSocket)
{
    int option_value;
    size_t option_len = sizeof(option_value);
    int rc = zmq_getsockopt((void*)pubSocket, ZMQ_TYPE, &option_value, &option_len);
    if (0 != rc) {
        Logger::E("CZMQChannel::SetPubSocket",
                                    "zmq_getsockopt error, errno %d %s",
                                    zmq_errno(), zmq_strerror(zmq_errno()));
        return E_FAILED_EXCEPTION;
    }

    if (ZMQ_SUB != option_value) {
        Logger::E("CZMQChannel::SetPubSocket", "pubSocket is no ZMQ_SUB type");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    mPubSocket = pubSocket;
    return NOERROR;
}

} // namespace como
