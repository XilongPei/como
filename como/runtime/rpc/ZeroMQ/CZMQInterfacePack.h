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

#ifndef __COMO_INTERFACEPACK_H__
#define __COMO_INTERFACEPACK_H__

#include "comotypes.h"
#include "rpc/registry.h"
#include "util/comoref.h"
#include "util/mutex.h"
#include "util/MemPool.h"

namespace como {

extern const InterfaceID IID_IZMQInterfacePack;

INTERFACE_ID(d26646eb-9df8-6435-e954-c7f008059379)
interface IZMQInterfacePack : public IInterface
{
    using IInterface::Probe;

    inline static IZMQInterfacePack* Probe(
        /* [in] */ IInterface* object)
    {
        if (object == nullptr) return nullptr;
        return (IZMQInterfacePack*)object->Probe(IID_IZMQInterfacePack);
    }
};

class CZMQInterfacePack
    : public LightRefBase
    , public IInterfacePack
    , public IParcelable
{
public:
    ~CZMQInterfacePack();

    COMO_INTERFACE_DECL();

    static void *MemPoolAlloc(size_t ulSize);
    static void MemPoolFree(Short id, const void *p);

    ECode GetCoclassID(
        /* [out] */ CoclassID& cid) override;

    ECode GetInterfaceID(
        /* [out] */ InterfaceID& iid) override;

    ECode IsParcelable(
        /* [out] */ Boolean& parcelable) override;

    ECode SetServerName(
        /* [in] */ const String& serverName) override;

    ECode GetServerName(
        /* [out] */ String& serverName) override;

    ECode SetProxyInfo(
        /* [in] */ Long proxyId) override;

    ECode GetProxyInfo(
        /* [out] */ Long& proxyId) override;

    ECode GetHashCode(
        /* [out] */ Long& hash) override;

    ECode GetServerObjectId(
        /* [out] */ Long& serverObjectId) override;

    ECode SetServerObjectId(
        /* [in] */ Long serverObjectId) override;

    ECode ReadFromParcel(
        /* [in] */ IParcel* source) override;

    ECode WriteToParcel(
        /* [in] */ IParcel* dest) override;

    ECode GiveMeAhand(
        /* [in] */ const String& aHand) override;

    void SetCoclassID(
        /* [in] */ const CoclassID& cid);

    void SetInterfaceID(
        /* [in] */ const InterfaceID& iid);

    void SetParcelable(
        /* [in] */ Boolean parcelable);

    inline static CZMQInterfacePack* From(
        /* [in] */ IInterfacePack* ipack);

    AutoPtr<IRPCChannel> mChannel;
    Mutex mLock;

private:
    static CMemPool *memPool;

    CoclassID mCid;
    InterfaceID mIid;
    Boolean mIsParcelable { false };
    String mServerName;     // the same machine, ServerAddress is nullptr
    Long mServerObjectId;
    Long mProxyId;
};

CZMQInterfacePack* CZMQInterfacePack::From(
    /* [in] */ IInterfacePack* ipack)
{
    return static_cast<CZMQInterfacePack*>(ipack);
}

} // namespace como

#endif // __COMO_INTERFACEPACK_H__
