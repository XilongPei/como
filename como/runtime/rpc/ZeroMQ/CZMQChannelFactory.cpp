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
#include "ThreadPoolZmqActor.h"

namespace como {

COMO_INTERFACE_IMPL_LIGHT_1(CZMQChannelFactory, LightRefBase, IRPCChannelFactory);

CZMQChannelFactory::CZMQChannelFactory(
    /* [in] */ RPCType type)
    : mType(type)
{
    // initial the ThreadPool environment, to avoid the delay of first RPC call
    TPZA_Executor::GetInstance();
}

ECode CZMQChannelFactory::CreateInterfacePack(
    /* [out] */ AutoPtr<IInterfacePack>& ipack)
{
    ipack = new CZMQInterfacePack();
    if (nullptr == ipack) {
        return E_OUT_OF_MEMORY_ERROR;
    }
    return NOERROR;
}

ECode CZMQChannelFactory::CreateParcel(
    /* [out] */ AutoPtr<IParcel>& parcel)
{
    parcel = new CZMQParcel();
    if (nullptr == parcel) {
        return E_OUT_OF_MEMORY_ERROR;
    }
    return NOERROR;
}

ECode CZMQChannelFactory::CreateChannel(
    /* [in] */ RPCPeer peer,
    /* [out] */ AutoPtr<IRPCChannel>& channel)
{
    channel = (IRPCChannel*)new CZMQChannel(mType, peer);
    if (nullptr == channel) {
        return E_OUT_OF_MEMORY_ERROR;
    }

    if (CZMQChannel::From(channel)->GetSocket() == nullptr) {
        Logger::E("CZMQChannelFactory::CreateChannel", "new CZMQChannel error");
        return E_RUNTIME_EXCEPTION;
    }

    return NOERROR;
}

ECode CZMQChannelFactory::MarshalInterface(
    /* [in] */ IInterface* object,
    /* [out] */ AutoPtr<IInterfacePack>& ipack)
{
    InterfaceID iid;
    object->GetInterfaceID(object, iid);
    CZMQInterfacePack* pack = new CZMQInterfacePack();
    if (nullptr == pack) {
        return E_OUT_OF_MEMORY_ERROR;
    }

    pack->SetInterfaceID(iid);

    IObject *obj = IObject::Probe(object);
    if (nullptr == obj) {
        Logger::E("CZMQChannelFactory::MarshalInterface",
                                            "The Object is not a COMO object.");
        ipack = nullptr;
        return E_NOT_COMO_OBJECT_EXCEPTION;
    }

    if (IParcelable::Probe(object) != nullptr) {
        CoclassID cid;
        IObject::Probe(object)->GetCoclassID(cid);
        pack->SetCoclassID(cid);
        pack->SetParcelable(true);
    }
    else {
        AutoPtr<IStub> stub;
        ECode ec = FindExportObject(mType, obj, stub);
        if (SUCCEEDED(ec)) {
            CZMQChannel* channel = CZMQChannel::GetStubChannel(stub);
            //pack->SetDBusName(channel->mName);
            pack->SetCoclassID(((CStub*)stub.Get())->GetTargetCoclassID());
        }
        else {
            IProxy* proxy = IProxy::Probe(object);
            if (proxy != nullptr) {
                CZMQChannel* channel = CZMQChannel::GetProxyChannel(proxy);
                //pack->SetDBusName(channel->mName);
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

                // TODO
                pack->mChannel = (IRPCChannel*)channel;

                pack->SetCoclassID(((CStub*)stub.Get())->GetTargetCoclassID());

                Long hash;
                obj->GetHashCode(hash);
                pack->SetServerObjectId(hash);

                ec = RegisterExportObject(mType, obj, stub);
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
        CoclassID cid;
        ipack->GetCoclassID(cid);
        InterfaceID iid;
        ipack->GetInterfaceID(iid);
        ECode ec = CoCreateObjectInstance(cid, iid, nullptr, &object);
        if (FAILED(ec)) {
            Logger::E("CZMQChannelFactory::UnmarshalInterface",
                                  "Create the object in ReadInterface failed.");
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
            Logger::E("CZMQChannelFactory::UnmarshalInterface",
                            "Unmarshal the interface in ReadInterface failed.");
            object = nullptr;
            return ec;
        }

        ec = RegisterImportObject(mType, ipack, IObject::Probe(proxy));
        if (FAILED(ec)) {
            Logger::E("CZMQChannelFactory::UnmarshalInterface",
                                "RegisterImportObject failed. ECode: 0x%X", ec);
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
