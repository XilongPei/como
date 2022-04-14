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

#include "component/comoobjapi.h"
#include "rpc/comorpc.h"
#include "rpc/CProxy.h"
#include "rpc/CStub.h"
#include "rpc/registry.h"
#include "rpc/dbus/CDBusChannel.h"
#include "rpc/dbus/CDBusChannelFactory.h"
#include "rpc/dbus/CDBusParcel.h"
#include "rpc/dbus/InterfacePack.h"
#include "util/comosp.h"

namespace como {

COMO_INTERFACE_IMPL_LIGHT_1(CDBusChannelFactory, LightRefBase, IRPCChannelFactory);

CDBusChannelFactory::CDBusChannelFactory(
    /* [in] */ RPCType type)
    : mType(type)
{
    // initial the ThreadPool environment, to avoid the delay of first RPC call
    ThreadPoolExecutor::GetInstance();
}

ECode CDBusChannelFactory::CreateInterfacePack(
    /* [out] */ AutoPtr<IInterfacePack>& ipack)
{
    ipack = new InterfacePack();
    if (nullptr == ipack) {
        return E_OUT_OF_MEMORY_ERROR;
    }
    return NOERROR;
}

ECode CDBusChannelFactory::CreateParcel(
    /* [out] */ AutoPtr<IParcel>& parcel)
{
    parcel = new CDBusParcel();
    if (nullptr == parcel) {
        return E_OUT_OF_MEMORY_ERROR;
    }
    return NOERROR;
}

ECode CDBusChannelFactory::CreateChannel(
    /* [in] */ RPCPeer peer,
    /* [out] */ AutoPtr<IRPCChannel>& channel)
{
    channel = (IRPCChannel*)new CDBusChannel(mType, peer);
    if (nullptr == channel) {
        return E_OUT_OF_MEMORY_ERROR;
    }
    return NOERROR;
}

ECode CDBusChannelFactory::MarshalInterface(
    /* [in] */ IInterface* object,
    /* [out] */ AutoPtr<IInterfacePack>& ipack)
{
    InterfaceID iid;
    object->GetInterfaceID(object, iid);
    InterfacePack* pack = new InterfacePack();
    if (nullptr == pack) {
        return E_OUT_OF_MEMORY_ERROR;
    }

    pack->SetInterfaceID(iid);

    if (IParcelable::Probe(object) != nullptr) {
        if (IObject::Probe(object) == nullptr) {
            Logger::E("CDBusChannelFactory", "The Object is not a como object.");
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
            CDBusChannel* channel = CDBusChannel::GetStubChannel(stub);
            pack->SetDBusName(channel->mName);
            pack->SetCoclassID(((CStub*)stub.Get())->GetTargetCoclassID());
        }
        else {
            IProxy* proxy = IProxy::Probe(object);
            if (proxy != nullptr) {
                CDBusChannel* channel = CDBusChannel::GetProxyChannel(proxy);
                pack->SetDBusName(channel->mName);
                pack->SetCoclassID(((CProxy*)proxy)->GetTargetCoclassID());
            }
            else {
                ec = CoCreateStub(object, mType, stub);
                if (FAILED(ec)) {
                    Logger::E("CDBusChannelFactory", "Marshal interface failed.");
                    ipack = nullptr;
                    return ec;
                }
                CDBusChannel* channel = CDBusChannel::GetStubChannel(stub);
                pack->SetDBusName(channel->mName);
                pack->SetCoclassID(((CStub*)stub.Get())->GetTargetCoclassID());
                RegisterExportObject(mType, IObject::Probe(object), stub);
            }
        }
    }

    ipack = (IInterfacePack*)pack;
    return NOERROR;
}

ECode CDBusChannelFactory::UnmarshalInterface(
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
            Logger::E("CDBusChannelFactory::UnmarshalInterface",
                               "CoCreateObjectInstance failed. ECode: 0x%X", ec);
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
            Logger::E("CDBusChannelFactory::UnmarshalInterface",
                                         "CoCreateProxy failed. ECode: 0x%X", ec);
            object = nullptr;
            return ec;
        }

        ((CProxy*)(IProxy*)proxy)->mIpack = ipack;

        RegisterImportObject(mType, ipack, IObject::Probe(proxy));

        InterfaceID iid;
        ipack->GetInterfaceID(iid);
        object = proxy->Probe(iid);
    }

    return NOERROR;
}

} // namespace como
