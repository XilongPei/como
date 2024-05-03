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

#ifndef __COMO_CZMQCHANNEL_H__
#define __COMO_CZMQCHANNEL_H__

#include <string>
#include "CProxy.h"
#include "CStub.h"
//#include "ThreadPoolZmqActor.h"
#include "util/comoobj.h"
#include "util/MemPool.h"

namespace como {

extern const CoclassID CID_CZMQChannel;

COCLASS_ID(4006ca46-c33d-a51f-3ac6-2b62ec497c2f)
class CZMQChannel
    : public Object
    , public IRPCChannel
{
public:
    CZMQChannel(
        /* [in] */ RPCType type,
        /* [in] */ RPCPeer peer);

    COMO_INTERFACE_DECL();

    COMO_OBJECT_DECL();

    static void *MemPoolAlloc(size_t ulSize);
    static void MemPoolFree(Short id, const void *p);

    ECode Apply(
        /* [in] */ IInterfacePack* ipack) override;

    ECode GetRPCType(
        /* [out] */ RPCType& type) override;

    ECode SetServerName(
        /* [in] */ const String& serverName) override;

    ECode GetServerName(
        /* [out] */ String& serverName) override;

    ECode GetServerObjectId(
        /* [out] */ Long& serverObjectId) override;

    ECode SetServerObjectId(
        /* [in] */ Long serverObjectId) override;

    ECode IsPeerAlive(
        /* [in, out] */ Long& lvalue,
        /* [out] */ Boolean& alive) override;

    ECode ReleasePeer(
        /* [out] */ Boolean& alive) override;

    ECode ReleaseObject(
        /* [in] */ Long objectId) override;

    ECode LinkToDeath(
        /* [in] */ IProxy* proxy,
        /* [in] */ IDeathRecipient* recipient,
        /* [in] */ HANDLE cookie = 0,
        /* [in] */ Integer flags = 0) override;

    ECode UnlinkToDeath(
        /* [in] */ IProxy* proxy,
        /* [in] */ IDeathRecipient* recipient,
        /* [in] */ HANDLE cookie = 0,
        /* [in] */ Integer flags = 0,
        /* [out] */ AutoPtr<IDeathRecipient>* outRecipient = nullptr) override;

    ECode GetComponentMetadata(
        /* [in] */ const CoclassID& cid,
        /* [out, callee] */ Array<Byte>& metadata) override;

    ECode Invoke(
        /* [in] */ IMetaMethod* method,
        /* [in] */ IParcel* argParcel,
        /* [out] */ AutoPtr<IParcel>& resParcel) override;

    ECode StartListening(
        /* [in] */ IStub* stub) override;

    ECode Match(
        /* [in] */ IInterfacePack* ipack,
        /* [out] */ Boolean& matched) override;

    ECode MonitorRuntime(
        /* [in] */ const Array<Byte>& request,
        /* [out] */ Array<Byte>& response) override;

    ECode SetPubSocket(
        /* [in] */ HANDLE pubSocket) override;

    static CZMQChannel* GetProxyChannel(
        /* [in] */ IProxy* proxy);

    static CZMQChannel* GetStubChannel(
        /* [in] */ IStub* stub);

    inline static CZMQChannel* From(
        /* [in] */ IRPCChannel* iRpcChannel);

    inline std::string& GetEndpoint(void);

private:
    friend class CZMQChannelFactory;

    static CMemPool *memPool;

    RPCType     mType;
    RPCPeer     mPeer;
    String      mName;
    String      mServerName;
    Long        mServerObjectId;
    std::string mEndpoint;
    Boolean     mStarted;
    HANDLE      mPubSocket;
};

inline CZMQChannel* CZMQChannel::GetProxyChannel(
    /* [in] */ IProxy* proxy)
{
    // AutoPtr.Get(): get (IRPCChannel*) from AutoPtr<IRPCChannel> return by GetChannel()
    return (CZMQChannel*)((CProxy*)proxy)->GetChannel().Get();
}

inline CZMQChannel* CZMQChannel::GetStubChannel(
    /* [in] */ IStub* stub)
{
    return (CZMQChannel*)((CStub*)stub)->GetChannel().Get();
}

inline std::string& CZMQChannel::GetEndpoint(void)
{
    return mEndpoint;
}

inline CZMQChannel* CZMQChannel::From(
    /* [in] */ IRPCChannel* iRpcChannel)
{
    return (CZMQChannel*)iRpcChannel;
}

} // namespace como

#endif // __COMO_CZMQCHANNEL_H__
