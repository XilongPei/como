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
    return NOERROR;
}

ECode CZMQInterfacePack::GetServerName(
    /* [out] */ String& serverName)
{
    serverName = mServerName;
    return NOERROR;
}

ECode CZMQInterfacePack::GetHashCode(
    /* [out] */ Long& hash)
{
    hash = mServerName.GetHashCode();
    return NOERROR;
}

ECode CZMQInterfacePack::GetServerObjectId(
    /* [out] */ Long& serverObjectId)
{
    serverObjectId = mServerObjectId;
    return NOERROR;
}

ECode CZMQInterfacePack::ReadFromParcel(
    /* [in] */ IParcel* source)
{
    source->ReadCoclassID(mCid);
    source->ReadInterfaceID(mIid);
    source->ReadBoolean(mIsParcelable);
    source->ReadLong(mServerObjectId);
    source->ReadString(mServerName);
    return NOERROR;
}

ECode CZMQInterfacePack::WriteToParcel(
    /* [in] */ IParcel* dest)
{
    dest->WriteCoclassID(mCid);
    dest->WriteInterfaceID(mIid);
    dest->WriteBoolean(mIsParcelable);
    dest->WriteLong(mServerObjectId);
    dest->WriteString(mServerName);
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
