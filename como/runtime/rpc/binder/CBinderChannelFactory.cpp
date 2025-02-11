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

#include <new>
#include "component/comoobjapi.h"
#include "rpc/comorpc.h"
#include "rpc/CProxy.h"
#include "rpc/CStub.h"
#include "rpc/binder/CBinderChannel.h"
#include "rpc/binder/CBinderChannelFactory.h"
#include "rpc/binder/CBinderParcel.h"
#include "rpc/binder/InterfacePack.h"

namespace como {

COMO_INTERFACE_IMPL_LIGHT_1(CBinderChannelFactory, LightRefBase, IRPCChannelFactory);

CBinderChannelFactory::CBinderChannelFactory(
    /* [in] */ RPCType type)
    : mType(type)
{}

ECode CBinderChannelFactory::CreateInterfacePack(
    /* [out] */ AutoPtr<IInterfacePack>& ipack)
{
#ifdef COMO_FUNCTION_SAFETY_RTOS
    void *buf = InterfacePack::MemPoolAlloc(sizeof(InterfacePack));
    if (nullptr == buf) {
        return E_OUT_OF_MEMORY_ERROR;
    }

    SetFunFreeMem(InterfacePack::MemPoolFree, 0);
    ipack = new(buf) InterfacePack();
#else
    ipack = new InterfacePack();
    if (nullptr == ipack) {
        return E_OUT_OF_MEMORY_ERROR;
    }
#endif

    return NOERROR;
}

ECode CBinderChannelFactory::CreateParcel(
    /* [out] */ AutoPtr<IParcel>& parcel)
{
#ifdef COMO_FUNCTION_SAFETY_RTOS
    void *buf = CBinderParcel::MemPoolAlloc(sizeof(CBinderParcel));
    if (nullptr == buf) {
        return E_OUT_OF_MEMORY_ERROR;
    }

    void *buf = CBinderParcel::MemPoolAlloc(sizeof(CBinderParcel));
    if (nullptr == buf) {
        return E_OUT_OF_MEMORY_ERROR;
    }

    CBinderParcel *cbinderParcel = new(buf) CBinderParcel();
    parcel = cbinderParcel;
    cbinderParcel->SetFunFreeMem(CBinderParcel::MemPoolFree, 0);
#else
    parcel = new CBinderParcel();
    if (nullptr == parcel) {
        return E_OUT_OF_MEMORY_ERROR;
    }
#endif

    return NOERROR;
}

ECode CBinderChannelFactory::CreateChannel(
    /* [in] */ RPCPeer peer,
    /* [out] */ AutoPtr<IRPCChannel>& channel)
{
#ifdef COMO_FUNCTION_SAFETY_RTOS
    void *buf = CBinderParcel::MemPoolAlloc(sizeof(CBinderParcel));
    if (nullptr == buf) {
        return E_OUT_OF_MEMORY_ERROR;
    }

    SetFunFreeMem(CDBusParcel::MemPoolFree, 0);
    channel = new(buf) CBinderChannel(mType, peer);
#else
    channel = (IRPCChannel*)new CBinderChannel(mType, peer);
    if (nullptr == channel) {
        return E_OUT_OF_MEMORY_ERROR;
    }
#endif

    return NOERROR;
}

ECode CBinderChannelFactory::MarshalInterface(
    /* [in] */ IInterface* object,
    /* [out] */ AutoPtr<IInterfacePack>& ipack)
{
    InterfaceID iid;
    object->GetInterfaceID(object, iid);

#ifdef COMO_FUNCTION_SAFETY_RTOS
    void *buf = InterfacePack::MemPoolAlloc(sizeof(InterfacePack));
    if (nullptr == buf) {
        return E_OUT_OF_MEMORY_ERROR;
    }

    SetFunFreeMem(InterfacePack::MemPoolFree, 0);
    InterfacePack* pack = new(buf) InterfacePack();
#else
    InterfacePack* pack = new InterfacePack();
    if (nullptr == pack) {
        return E_OUT_OF_MEMORY_ERROR;
    }
#endif

    pack->SetInterfaceID(iid);

    if (IParcelable::Probe(object) != nullptr) {
        if (IObject::Probe(object) == nullptr) {
            Logger::E("CBinderChannelFactory", "The Object is not a como object.");
            ipack = nullptr;
            return E_NOT_COMO_OBJECT_EXCEPTION;
        }
        CoclassID cid;
        IObject::Probe(object)->GetCoclassID(cid);
        pack->SetCoclassID(cid);
        pack->SetParcelable(true);
    }
    else {
        AutoPtr<IStub> stub;
        ECode ec = FindExportObject(mType, IObject::Probe(object), stub);
        if (SUCCEEDED(ec)) {
            CBinderChannel* channel = CBinderChannel::GetStubChannel(stub);
            pack->SetAndroidBinder(channel->mBinder);
            pack->SetCoclassID(((CStub*)stub.Get())->GetTargetCoclassID());
        }
        else {
            IProxy* proxy = IProxy::Probe(object);
            if (proxy != nullptr) {
                CBinderChannel* channel = CBinderChannel::GetProxyChannel(proxy);
                pack->SetAndroidBinder(channel->mBinder);
                pack->SetCoclassID(((CProxy*)proxy)->GetTargetCoclassID());

                //@ `ServerObjectId`
                // Who reference the object {/* [in] */ IInterface* object} through
                // {/* [out] */ AutoPtr<IInterfacePack>& ipack}, it can get the
                // ID (ServerObjectId) of object
                obj->GetHashCode(hash);
                pack->SetServerObjectId(hash);
            }
            else {
                ec = CoCreateStub(object, mType, stub);
                if (FAILED(ec)) {
                    Logger::E("CBinderChannelFactory", "Marshal interface failed.");
                    ipack = nullptr;
                    return ec;
                }
                CBinderChannel* channel = CBinderChannel::GetStubChannel(stub);
                pack->SetAndroidBinder(channel->mBinder);
                pack->SetCoclassID(((CStub*)stub.Get())->GetTargetCoclassID());
                RegisterExportObject(mType, IObject::Probe(object), stub, 0);
            }
        }
    }

    ipack = (IInterfacePack*)pack;
    return NOERROR;
}

ECode CBinderChannelFactory::UnmarshalInterface(
    /* [in] */ IInterfacePack* ipack,
    /* [out] */ AutoPtr<IInterface>& object)
{
    Boolean parcelable;
    ipack->IsParcelable(parcelable);
    if (parcelable) {
        CoclassID cid;
        ipack->GetCoclassID(cid);
        InterfaceID iid;
        ipack->GetInterfaceID(iid);
        ECode ec = CoCreateObjectInstance(cid, iid, nullptr, &object);
        if (FAILED(ec)) {
            Logger::E("CBinderChannelFactory", "Create the object in ReadInterface failed.");
            return ec;
        }
    }
    else {
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
            Logger::E("CBinderChannelFactory",
                                       "CoCreateProxy failed. ECode: 0x%X", ec);
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

        InterfaceID iid;
        ipack->GetInterfaceID(iid);
        object = proxy->Probe(iid);
    }

    return NOERROR;
}

} // namespace como
