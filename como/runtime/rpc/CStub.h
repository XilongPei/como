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

//=========================================================================
// Copyright (C) 2012 The Elastos Open Source Project
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

#ifndef __COMO_CSTUB_H__
#define __COMO_CSTUB_H__

#include "reflection/comoreflapi.h"
#include "type/comoarray.h"
#include "util/comoobj.h"

namespace como {

class CStub;

class InterfaceStub
{
public:
    Integer AddRef(
        /* [in] */ HANDLE id = 0);

    Integer Release(
        /* [in] */ HANDLE id = 0);

    ECode Invoke(
        /* [in] */ IParcel* argParcel,
        /* [out] */ AutoPtr<IParcel>& resParcel);

private:
    ECode UnmarshalArguments(
        /* [in] */ IMetaMethod* method,
        /* [in] */ IParcel* argParcel,
        /* [out] */ AutoPtr<IArgumentList>& argList);

    ECode MarshalResults(
        /* [in] */ IMetaMethod* method,
        /* [in] */ IArgumentList* argList,
        /* [out] */ AutoPtr<IParcel>& resParcel);

private:
    friend class CStub;

    InterfaceID mIid;
    IMetaInterface* mTargetMetadata;
    IInterface* mObject;
    CStub* mOwner;
};

extern const CoclassID CID_CStub;

COCLASS_ID(52068014-e347-453f-87a9-0becfb69d8ed)
class CStub
    : public Object
    , public IStub
{
public:
    COMO_INTERFACE_DECL();

    COMO_OBJECT_DECL();

    void OnLastStrongRef(
        /* [in] */ const void* id) override;

    ECode Match(
        /* [in] */ IInterfacePack* ipack,
        /* [out] */ Boolean& matched) override;

    ECode Invoke(
        /* [in] */ IParcel* argParcel,
        /* [out] */ AutoPtr<IParcel>& resParcel) override;

    AutoPtr<IObject> GetTarget();

    AutoPtr<IRPCChannel> GetChannel();

    CoclassID GetTargetCoclassID();

    static ECode CreateObject(
        /* [in] */ IInterface* object,
        /* [in] */ IRPCChannel* channel,
        /* [out] */ AutoPtr<IStub>& stub);

private:
    friend class InterfaceStub;

    static constexpr Boolean DEBUG = false;

    AutoPtr<IObject> mTarget;
    CoclassID mCid;
    IMetaCoclass* mTargetMetadata;
    Array<InterfaceStub*> mInterfaces;
    AutoPtr<IRPCChannel> mChannel;
};

} // namespace como

#endif // __COMO_CSTUB_H__
