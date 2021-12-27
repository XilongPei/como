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

#ifndef __COMO_CDBUSCHANNEL_H__
#define __COMO_CDBUSCHANNEL_H__

#include "CProxy.h"
#include "CStub.h"
#include "ThreadPoolExecutor.h"
#include "util/comoobj.h"
#include "util/mutex.h"
#include <dbus/dbus.h>

namespace como {

extern const CoclassID CID_CDBusChannel;

COCLASS_ID(8efc6167-e82e-4c7d-89aa-668f397b23cc)
class CZMQChannel
    : public Object
    , public IRPCChannel
{
private:
    class ServiceRunnable
        : public ThreadPoolExecutor::Runnable
    {
    public:
        ServiceRunnable(
            /* [in] */ CZMQChannel* owner,
            /* [in] */ IStub* target);

        ECode Run();

    private:
        static DBusHandlerResult HandleMessage(
            /* [in] */ DBusConnection* conn,
            /* [in] */ DBusMessage* msg,
            /* [in] */ void* arg);

    private:
        CZMQChannel* mOwner;
        IStub* mTarget;
        Boolean mRequestToQuit;
    };

public:
    CZMQChannel(
        /* [in] */ RPCType type,
        /* [in] */ RPCPeer peer);

    COMO_INTERFACE_DECL();

    COMO_OBJECT_DECL();

    ECode Apply(
        /* [in] */ IInterfacePack* ipack) override;

    ECode GetRPCType(
        /* [out] */ RPCType& type) override;

    ECode GetServerName(
        /* [out] */ String& serverName) override;

    ECode IsPeerAlive(
        /* [out] */ Boolean& alive) override;

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

    static CZMQChannel* GetProxyChannel(
        /* [in] */ IProxy* proxy);

    static CZMQChannel* GetStubChannel(
        /* [in] */ IStub* stub);

private:
    friend class CDBusChannelFactory;

    static constexpr Boolean DEBUG = false;
    static constexpr const char* STUB_OBJECT_PATH = "/como/rpc/CStub";
    static constexpr const char* STUB_INTERFACE_PATH = "como.rpc.IStub";

    RPCType mType;
    RPCPeer mPeer;
    String mName;
    String mServerName;
    void  *mSocket;
    Boolean mStarted;
    Mutex mLock;
    Condition mCond;
};

inline CZMQChannel* CZMQChannel::GetProxyChannel(
    /* [in] */ IProxy* proxy)
{
    return (CZMQChannel*)((CProxy*)proxy)->GetChannel().Get();
}

inline CZMQChannel* CZMQChannel::GetStubChannel(
    /* [in] */ IStub* stub)
{
    return (CZMQChannel*)((CStub*)stub)->GetChannel().Get();
}

} // namespace como

#endif // __COMO_CDBUSCHANNEL_H__
