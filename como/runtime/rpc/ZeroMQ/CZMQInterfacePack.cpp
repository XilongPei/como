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

#include "ComoConfig.h"
#include "CZMQUtils.h"
#include "CZMQChannel.h"
#include "CZMQInterfacePack.h"

namespace como {

// $ ./comouuid --cpp -cityhash como::IZMQInterfacePack
// {0xeb4666d2,0xf89d,0x3564,0x54e9,{0xc7,0xf0,0x08,0x05,0x93,0x79}}
const InterfaceID IID_IZMQInterfacePack =
        {{0xeb4666d2,0xf89d,0x3564,0x54e9,{0xc7,0xf0,0x08,0x05,0x93,0x79}}, &CID_COMORuntime};

COMO_INTERFACE_IMPL_LIGHT_3(CZMQInterfacePack, LightRefBase, IInterfacePack, IZMQInterfacePack, IParcelable);

CZMQInterfacePack::~CZMQInterfacePack()
{
    ReleaseCoclassID(mCid);
    ReleaseInterfaceID(mIid);
}

CMemPool *CZMQInterfacePack::memPool = new CMemPool(nullptr, ComoConfig::POOL_SIZE_InterfacePack,
                                                    sizeof(CZMQInterfacePack));
void *CZMQInterfacePack::MemPoolAlloc(size_t ulSize)
{
    return memPool->Alloc(ulSize, MUST_USE_MEM_POOL);
}

void CZMQInterfacePack::MemPoolFree(Short id, const void *p)
{
    memPool->Free((void *)p);
}

ECode CZMQInterfacePack:: GetCoclassID(
    /* [out] */ CoclassID& cid)
{
    cid = mCid;
    return NOERROR;
}

ECode CZMQInterfacePack::GetInterfaceID(
    /* [out] */ InterfaceID& iid)
{
    iid = mIid;
    return NOERROR;
}

ECode CZMQInterfacePack::IsParcelable(
    /* [out] */ Boolean& parcelable)
{
    parcelable = mIsParcelable;
    return NOERROR;
}

ECode CZMQInterfacePack::SetServerName(
    /* [in] */ const String& serverName)
{
    mServerName = serverName;

    if (nullptr != mChannel) {
        mChannel->SetServerName(serverName);
    }

    return NOERROR;
}

ECode CZMQInterfacePack::GetServerName(
    /* [out] */ String& serverName)
{
    serverName = mServerName;
    return NOERROR;
}

ECode CZMQInterfacePack::SetProxyInfo(
    /* [in] */ Long proxyId)
{
    mProxyId = proxyId;
    return NOERROR;
}

ECode CZMQInterfacePack::GetProxyInfo(
    /* [out] */ Long& proxyId)
{
    proxyId = mProxyId;
    return NOERROR;
}

ECode CZMQInterfacePack::GetHashCode(
    /* [out] */ Long& hash)
{
    hash = reinterpret_cast<HANDLE>(this);
    return NOERROR;
}

ECode CZMQInterfacePack::GetServerObjectId(
    /* [out] */ Long& serverObjectId)
{
    serverObjectId = mServerObjectId;
    return NOERROR;
}

ECode CZMQInterfacePack::SetServerObjectId(
    /* [out] */ Long serverObjectId)
{
    mServerObjectId = serverObjectId;
    return NOERROR;
}

ECode CZMQInterfacePack::ReadFromParcel(
    /* [in] */ IParcel* source)
{
    FAIL_RETURN(source->ReadCoclassID(mCid));
    FAIL_RETURN(source->ReadInterfaceID(mIid));
    FAIL_RETURN(source->ReadBoolean(mIsParcelable));
    FAIL_RETURN(source->ReadLong(mServerObjectId));
    FAIL_RETURN(source->ReadString(mServerName));
    return NOERROR;
}

ECode CZMQInterfacePack::WriteToParcel(
    /* [in] */ IParcel* dest)
{
    FAIL_RETURN(dest->WriteCoclassID(mCid));
    FAIL_RETURN(dest->WriteInterfaceID(mIid));
    FAIL_RETURN(dest->WriteBoolean(mIsParcelable));
    FAIL_RETURN(dest->WriteLong(mServerObjectId));
    FAIL_RETURN(dest->WriteString(mServerName));
    return NOERROR;
}

ECode CZMQInterfacePack::GiveMeAhand(
    /* [in] */ const String& aHand)
{
    //TODO
    // Requests that do not come from the default port need to be forwarded to
    // the default port
#if 0
    std::string name;
    std::string endpoint;
    void *socket;

    Mutex::AutoLock lock(mLock);

    std::map<std::string, ServerNodeInfo*>::iterator iter =
            ComoConfig::ServerNameEndpointMap.find(std::string(aHand.string()));

    if (iter != ComoConfig::ServerNameEndpointMap.end()) {
        name = iter->first;
        endpoint = iter->second->endpoint;
    }
    else {
        Logger::E("CZMQInterfacePack::GiveMeAhand",
                                 "Unregistered ServerName: %s", aHand.string());
        return E_NOT_FOUND_EXCEPTION;
    }

    if (nullptr != iter->second->socket) {
        socket = iter->second->socket;
        iter->second->inChannel++;
    }
    else {
        socket = CZMQUtils::CzmqGetSocket(nullptr, endpoint.c_str(), ZMQ_REP);
        if (nullptr != socket) {
            iter->second->socket = socket;
            iter->second->inChannel++;
        }
    }

    CZMQChannel::From(mChannel)->SetSocket(socket);
#endif
    return NOERROR;
}

void CZMQInterfacePack::SetCoclassID(
    /* [in] */ const CoclassID& cid)
{
    mCid = CloneCoclassID(cid);
}

void CZMQInterfacePack::SetInterfaceID(
    /* [in] */ const InterfaceID& iid)
{
    mIid = CloneInterfaceID(iid);
}

void CZMQInterfacePack::SetParcelable(
    /* [in] */ Boolean parcelable)
{
    mIsParcelable = parcelable;
}

} // namespace como
