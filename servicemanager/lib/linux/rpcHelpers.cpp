//=========================================================================
// Copyright (C) 2022 The C++ Component Model(COMO) Open Source Project
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

#include <comolog.h>
#include <comoapi.h>
#include <comosp.h>
#include "ServiceManager.h"
#include "rpcHelpers.h"

namespace jing {

ECode RpcHelpers::ReleaseService(
    /* [in] */ const String& name,
    /* [in] */ IInterface *intfService)
{
    if (nullptr == intfService) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    IProxy* proxy = IProxy::Probe(intfService);
    if (proxy != nullptr) {
        Boolean alive;
        proxy->ReleaseStub(alive);
        if (! alive) {
            Logger_D("ServiceManager::ReleaseService",
                                           "Corresponding IStub doesn't exist");
        }
    }

    return ServiceManager::GetInstance()->RemoveService(name);
}

ECode RpcHelpers::ReleaseRemoteObject(
    /* [in] */ IInterface *intfService,
    /* [in] */ IInterface *obj)
{
    ECode ec;
    Long hash;

    if ((nullptr == intfService) || (nullptr == obj)) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    IProxy* proxy = IProxy::Probe(intfService);
    if (nullptr != proxy) {
    // It does not implement the COMO class of IParcelable interface.
    // The method execution is on the service server side.
        proxy = IProxy::Probe(obj);
        if (nullptr == proxy) {
            return E_INTERFACE_NOT_FOUND_EXCEPTION;
        }

        AutoPtr<como::IInterfacePack> ipack;
        ec = proxy->GetIpack(ipack);
        if (FAILED(ec)) {
            return ec;
        }

        ipack->GetServerObjectId(hash);
        return proxy->ReleaseObject(hash);
    }

    // It implements the COMO class of IParcelable interface. Its method
    // execution is executed on the client side.
    return IObject::Probe(obj)->Release();
}

ECode RpcHelpers::ReleaseImportObject(
    /* [in] */ IInterface *intfService,
    /* [in] */ IInterface *obj)
{
    ECode ec;
    Long hash;
    Long channelId;

    if ((nullptr == intfService) || (nullptr == obj)) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    IProxy* proxy = IProxy::Probe(intfService);
    if (nullptr != proxy) {
    // It does not implement the COMO class of IParcelable interface.
    // The method execution is on the service server side.

        if (nullptr == obj) {
        // Release(REFCOUNT_RELEASE) all ImportObjects include intfService
            AutoPtr<como::IInterfacePack> ipack;
            ec = proxy->GetIpack(ipack);
            if (FAILED(ec)) {
                return ec;
            }

            ipack->GetProxyInfo(channelId);

            String str = nullptr;
            ec = ipack->GetServerName(str);
            if ((nullptr == str) || str.IsEmpty()) {
                return como::CoUnregisterImportObject(RPCType::Local, channelId);
            }
            else {
                return como::CoUnregisterImportObject(RPCType::Remote, channelId);
            }
        }

        // Release one ImportObject
        proxy = IProxy::Probe(obj);
        if (nullptr == proxy) {
            return E_INTERFACE_NOT_FOUND_EXCEPTION;
        }

        AutoPtr<como::IInterfacePack> ipack;
        ec = proxy->GetIpack(ipack);
        if (FAILED(ec)) {
            return ec;
        }

        ipack->GetProxyInfo(channelId);

        String str = nullptr;
        ec = ipack->GetServerName(str);
        if ((nullptr == str) || str.IsEmpty()) {
            return como::CoUnregisterImportObject(RPCType::Local, channelId);
        }
        else {
            return como::CoUnregisterImportObject(RPCType::Remote, channelId);
        }
    }

    // It implements the COMO class of IParcelable interface. Its method
    // execution is executed on the client side.
    IObject::Probe(obj)->GetHashCode(hash);
    (void)como::CoUnregisterImportObject(RPCType::Local, hash);
    return como::CoUnregisterImportObject(RPCType::Remote, hash);
}

} // namespace jing
