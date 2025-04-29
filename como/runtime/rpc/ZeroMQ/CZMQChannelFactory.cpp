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

#include <new>
#include "component/comoobjapi.h"
#include "rpc/comorpc.h"
#include "rpc/CProxy.h"
#include "rpc/CStub.h"
#include "rpc/registry.h"
#include "rpc/ZeroMQ/CZMQChannel.h"
#include "rpc/ZeroMQ/CZMQChannelFactory.h"
#include "rpc/ZeroMQ/CZMQParcel.h"
#include "rpc/ZeroMQ/CZMQInterfacePack.h"
#include "util/comosp.h"
#include "ComoerrorHelper.h"
#include "ThreadPoolZmqActor.h"

namespace como {

COMO_INTERFACE_IMPL_LIGHT_1(CZMQChannelFactory, LightRefBase, IRPCChannelFactory);

CZMQChannelFactory::CZMQChannelFactory(
    /* [in] */ RPCType type)
    : mType(type)
{
    // initial the ThreadPool environment, to avoid the delay of first RPC call
    TPZA_Executor *instance = TPZA_Executor::GetInstance();
    if (nullptr == instance) {
        Logger::E("CZMQChannelFactory", "TPZA_Executor::GetInstance() failed.");
    }
}

ECode CZMQChannelFactory::CreateInterfacePack(
    /* [out] */ AutoPtr<IInterfacePack>& ipack)
{
#ifdef COMO_FUNCTION_SAFETY_RTOS
    void *buf = CZMQInterfacePack::MemPoolAlloc(sizeof(CZMQInterfacePack));
    if (nullptr == buf) {
        return E_OUT_OF_MEMORY_ERROR;
    }

    SetFunFreeMem(CZMQInterfacePack::MemPoolFree, 0);
    ipack = new(buf) CZMQInterfacePack();
#else
    ipack = new CZMQInterfacePack();
    if (nullptr == ipack) {
        return E_OUT_OF_MEMORY_ERROR;
    }
#endif

    return NOERROR;
}

ECode CZMQChannelFactory::CreateParcel(
    /* [out] */ AutoPtr<IParcel>& parcel)
{
#ifdef COMO_FUNCTION_SAFETY_RTOS
    void *buf = CZMQParcel::MemPoolAlloc(sizeof(CZMQParcel));
    if (nullptr == buf) {
        return E_OUT_OF_MEMORY_ERROR;
    }

    CZMQParcel *czmqParcel = new(buf) CZMQParcel();
    parcel = czmqParcel;
    czmqParcel->SetFunFreeMem(CZMQParcel::MemPoolFree, 0);
#else
    parcel = new CZMQParcel();
    if (nullptr == parcel) {
        return E_OUT_OF_MEMORY_ERROR;
    }
#endif

    return NOERROR;
}

ECode CZMQChannelFactory::CreateChannel(
    /* [in] */ RPCPeer peer,
    /* [out] */ AutoPtr<IRPCChannel>& channel)
{
#ifdef COMO_FUNCTION_SAFETY_RTOS
    void *buf = CZMQChannel::MemPoolAlloc(sizeof(CZMQChannel));
    if (nullptr == buf) {
        return E_OUT_OF_MEMORY_ERROR;
    }

    SetFunFreeMem(CZMQChannel::MemPoolFree, 0);
    channel = new(buf) CZMQChannel(mType, peer);
#else
    channel = new CZMQChannel(mType, peer);
    if (nullptr == channel) {
        return E_OUT_OF_MEMORY_ERROR;
    }
#endif

    return NOERROR;
}

ECode CZMQChannelFactory::MarshalInterface(
    /* [in] */ IInterface* object,
    /* [out] */ AutoPtr<IInterfacePack>& ipack)
{
    Long hash;
    InterfaceID iid;

    object->GetInterfaceID(object, iid);

#ifdef COMO_FUNCTION_SAFETY_RTOS
    void *buf = CZMQInterfacePack::MemPoolAlloc(sizeof(CZMQInterfacePack));
    if (nullptr == buf) {
        return E_OUT_OF_MEMORY_ERROR;
    }

    SetFunFreeMem(CZMQInterfacePack::MemPoolFree, 0);
    CZMQInterfacePack* pack = new(buf) CZMQInterfacePack();
#else
    CZMQInterfacePack* pack = new CZMQInterfacePack();
    if (nullptr == pack) {
        return E_OUT_OF_MEMORY_ERROR;
    }
#endif

    pack->SetInterfaceID(iid);

    IObject *obj = IObject::Probe(object);
    if (nullptr == obj) {
        Logger::E("CZMQChannelFactory::MarshalInterface",
                                            "The Object is not a COMO object.");
        ipack = nullptr;
        return E_NOT_COMO_OBJECT_EXCEPTION;
    }

    if (IParcelable::Probe(object) != nullptr) {
    // It implements the IParcelable interface. Its method is executed on the
    // client side. The fields that identify the service object needn't to be set.
        CoclassID cid;
        IObject::Probe(object)->GetCoclassID(cid);
        pack->SetCoclassID(cid);
        pack->SetParcelable(true);
    }
    else {
    // It does not implement the IParcelable interface. Its method is executed
    // on the on the server side.
        AutoPtr<IStub> stub;
        ECode ec = FindExportObject(mType, obj, stub);
        if (SUCCEEDED(ec)) {
            CZMQChannel* channel = CZMQChannel::GetStubChannel(stub);

            //pack->SetDBusName(channel->mName);
            //@ `ServerObjectId`
            // Who reference the object {/* [in] */ IInterface* object} through
            // {/* [out] */ AutoPtr<IInterfacePack>& ipack}, it can get the
            // ID (ServerObjectId) of object
            obj->GetHashCode(hash);
            pack->SetServerObjectId(hash);

            pack->SetCoclassID(((CStub*)stub.Get())->GetTargetCoclassID());
        }
        else {
            IProxy* proxy = IProxy::Probe(object);
            if (proxy != nullptr) {
                CZMQChannel* channel = CZMQChannel::GetProxyChannel(proxy);

                //pack->SetDBusName(channel->mName);
                // refer to `ServerObjectId`
                obj->GetHashCode(hash);
                pack->SetServerObjectId(hash);

                pack->SetCoclassID(((CProxy*)proxy)->GetTargetCoclassID());
            }
            else {
                ec = CoCreateStub(object, mType, stub);
                if (FAILED(ec)) {
                    Logger::E("CZMQChannelFactory::MarshalInterface",
                                                   "Marshal interface failed.");
                    ipack = nullptr;
                    return ec;
                }

                CZMQChannel* channel = CZMQChannel::GetStubChannel(stub);
                pack->mChannel = (IRPCChannel*)channel;
                pack->SetCoclassID(((CStub*)stub.Get())->GetTargetCoclassID());

                // refer to `ServerObjectId`
                obj->GetHashCode(hash);
                pack->SetServerObjectId(hash);

                ec = RegisterExportObject(mType, obj, stub, hash);
                if (FAILED(ec)) {
                    Logger::E("CZMQChannelFactory::MarshalInterface",
                                                "RegisterExportObject failed.");
                    ipack = nullptr;
                    return ec;
                }
            }
        }
    }

    ipack = (IInterfacePack*)pack;
    return NOERROR;
}

ECode CZMQChannelFactory::UnmarshalInterface(
    /* [in] */ IInterfacePack* ipack,
    /* [out] */ AutoPtr<IInterface>& object)
{
    Boolean parcelable;
    ipack->IsParcelable(parcelable);
    if (parcelable) {
    // It implements the IParcelable interface. Its method execution is executed
    // on the client side.
        CoclassID cid;
        ipack->GetCoclassID(cid);
        InterfaceID iid;
        ipack->GetInterfaceID(iid);
        ECode ec = CoCreateObjectInstance(cid, iid, nullptr, &object);
        if (FAILED(ec)) {
            Logger::E("CZMQChannelFactory::UnmarshalInterface",
                              "CoCreateObjectInstance failed, ECode: 0x%X", ec);
            return ec;
        }
    }
    else {
    // It does not implement the IParcelable interface. The method execution is
    // on the server side.
        AutoPtr<IObject> iobject;
        ECode ec = FindImportObject(mType, ipack, iobject);
        if (SUCCEEDED(ec)) {
            InterfaceID iid;
            ipack->GetInterfaceID(iid);
            object = iobject->Probe(iid);
            return NOERROR;
        }

        AutoPtr<IStub> stub;
        ec = FindExportObject(mType, ipack, stub);
        if (SUCCEEDED(ec)) {
            CStub* stubObj = (CStub*)stub.Get();
            InterfaceID iid;
            ipack->GetInterfaceID(iid);
            object = stubObj->GetTarget()->Probe(iid);
            return NOERROR;
        }

        AutoPtr<IProxy> proxy;
        ec = CoCreateProxy(ipack, mType, nullptr, proxy);
        if (FAILED(ec)) {
            Logger::E("CZMQChannelFactory::UnmarshalInterface",
                                       "CoCreateProxy failed, ECode: 0x%X", ec);
            object = nullptr;
            return ec;
        }

        ((CProxy*)(IProxy*)proxy)->mIpack = ipack;

        IObject *obj = IObject::Probe(proxy);
        Long channelId;
        ipack->GetProxyInfo(channelId);
        if (0 == channelId) {
        // Service itself, there is no ProxyInfo at this time
            obj->GetHashCode(channelId);
        }

        ec = RegisterImportObject(mType, ipack, obj, channelId);
        if (FAILED(ec)) {
            Logger::E("CZMQChannelFactory::UnmarshalInterface",
                                "RegisterImportObject failed, ECode: 0x%X", ec,
                                ComoerrorHelper::GetEcErrorInfo(ec));
            object = nullptr;
            return ec;
        }

        InterfaceID iid;
        ipack->GetInterfaceID(iid);
        object = proxy->Probe(iid);
    }

    return NOERROR;
}

} // namespace como
