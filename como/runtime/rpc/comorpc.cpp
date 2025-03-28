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

#include "comorpc.h"
#include "CProxy.h"
#include "CStub.h"
#include <ThreadPoolChannelInvoke.h>

/*
 * use dbus always (?)
 *
#if defined(__android__)
#include "binder/CBinderChannelFactory.h"
#elif defined(__linux__)
#include "dbus/CDBusChannelFactory.h"
#endif
*/
#include "dbus/CDBusChannelFactory.h"
#include "dbus/ThreadPoolExecutor.h"

#if defined(RPC_OVER_ZeroMQ_SUPPORT)
#include "ZeroMQ/CZMQChannelFactory.h"
#include "ZeroMQ/CZMQUtils.h"
#include "ZeroMQ/ThreadPoolZmqActor.h"
#endif
#include "registry.h"

namespace como {

#if defined(__android__)
/*
 * use dbus always (?)
 *
static AutoPtr<IRPCChannelFactory> sLocalFactory = new CBinderChannelFactory(RPCType::Local);
*/
static AutoPtr<IRPCChannelFactory> sLocalFactory = new CDBusChannelFactory(RPCType::Local);
#elif defined(__linux__)
static AutoPtr<IRPCChannelFactory> sLocalFactory = new CDBusChannelFactory(RPCType::Local);
#endif
#if defined(RPC_OVER_ZeroMQ_SUPPORT)
static AutoPtr<IRPCChannelFactory> sRemoteFactory = new CZMQChannelFactory(RPCType::Remote);
#else
static AutoPtr<IRPCChannelFactory> sRemoteFactory;
#endif

ECode CoCreateParcel(
    /* [in] */ RPCType type,
    /* [out] */ AutoPtr<IParcel>& parcel)
{
    AutoPtr<IRPCChannelFactory> factory =
                    ((type == RPCType::Local) ? sLocalFactory : sRemoteFactory);
    return factory->CreateParcel(parcel);
}

ECode CoCreateInterfacePack(
    /* [in] */ RPCType type,
    /* [out] */ AutoPtr<IInterfacePack>& ipack)
{
    AutoPtr<IRPCChannelFactory> factory =
                    ((type == RPCType::Local) ? sLocalFactory : sRemoteFactory);
    return factory->CreateInterfacePack(ipack);
}

ECode CoCreateProxy(
    /* [in] */ IInterfacePack* ipack,
    /* [in] */ RPCType type,
    /* [in] */ IClassLoader* loader,
    /* [out] */ AutoPtr<IProxy>& proxy)
{
    AutoPtr<IRPCChannelFactory> factory =
                    ((type == RPCType::Local) ? sLocalFactory : sRemoteFactory);
    AutoPtr<IRPCChannel> channel;
    ECode ec = factory->CreateChannel(RPCPeer::Proxy, channel);
    if (FAILED(ec)) {
        proxy = nullptr;
        return ec;
    }
    channel->Apply(ipack);

    String serverName;
    ipack->GetServerName(serverName);
    channel->SetServerName(serverName);

    Long serverObjectId;
    ipack->GetServerObjectId(serverObjectId);
    channel->SetServerObjectId(serverObjectId);

    CoclassID cid;
    ipack->GetCoclassID(cid);
    return CProxy::CreateObject(cid, channel, loader, proxy);
}

ECode CoCreateStub(
    /* [in] */ IInterface* object,
    /* [in] */ RPCType type,
    /* [out] */ AutoPtr<IStub>& stub)
{
    AutoPtr<IRPCChannelFactory> factory =
                    ((type == RPCType::Local) ? sLocalFactory : sRemoteFactory);
    AutoPtr<IRPCChannel> channel;
    ECode ec = factory->CreateChannel(RPCPeer::Stub, channel);
    if (FAILED(ec)) {
        stub = nullptr;
        return ec;
    }
    return CStub::CreateObject(object, channel, stub);
}

ECode CoMarshalInterface(
    /* [in] */ IInterface* object,
    /* [in] */ RPCType type,
    /* [out] */ AutoPtr<IInterfacePack>& ipack)
{
    if (nullptr == object) {
        ipack = nullptr;
        return NOERROR;
    }

    AutoPtr<IRPCChannelFactory> factory =
                    ((type == RPCType::Local) ? sLocalFactory : sRemoteFactory);
    return factory->MarshalInterface(object, ipack);
}

ECode CoUnmarshalInterface(
    /* [in] */ IInterfacePack* data,
    /* [in] */ RPCType type,
    /* [out] */ AutoPtr<IInterface>& object)
{
    if (nullptr == data) {
        object = nullptr;
        return NOERROR;
    }

    AutoPtr<IRPCChannelFactory> factory =
                    ((type == RPCType::Local) ? sLocalFactory : sRemoteFactory);
    return factory->UnmarshalInterface(data, object);
}

ECode CoUnregisterImportObject(
    /* [in] */ RPCType type,
    /* [in] */ Long channel)
{
    return UnregisterImportObjectById(type, channel);
}

ECode CoStopAll()
{
    // rpc/dbus
    (void)ThreadPoolExecutor::threadPool->StopAll();

#ifdef RPC_OVER_ZeroMQ_SUPPORT
    // rpc/ZeroMQ
    TPZA_Executor::threadPool->StopAll();
#endif

#ifdef COMO_FUNCTION_SAFETY
    // ThreadPoolChannelInvoke
    TPCI_Executor::threadPool->StopAll();
#endif

    return NOERROR;
}

} // namespace como
