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

#ifndef __COMO_CMETAMETHOD_H__
#define __COMO_CMETAMETHOD_H__

#include "comotypes.h"
#include "metadata/Component.h"
#include "util/comoref.h"
#include "util/mutex.h"
#include "ComoConfig.h"

namespace como {

class CMetaInterface;

class CMetaMethod
    : public LightRefBase
    , public IMetaMethod
{
public:
    CMetaMethod();

    CMetaMethod(
        /* [in] */ MetaComponent *mc,
        /* [in] */ CMetaInterface *miObj,
        /* [in] */ Integer index,
        /* [in] */ MetaMethod *mm);

    COMO_INTERFACE_DECL();

    ECode GetInterface(
        /* [out] */ AutoPtr<IMetaInterface>& intf) override;

    ECode GetName(
        /* [out] */ String& name) override;

    ECode GetSignature(
        /* [out] */ String& signature) override;

    ECode GetStrFramacBlock(
        /* [out] */ String& strFramacBlock) override;

    ECode GetParameterNumber(
        /* [out] */ Integer& number) override;

    ECode GetAllParameters(
        /* [out] */ Array<IMetaParameter*>& params) override;

    ECode GetParameter(
        /* [in] */ Integer index,
        /* [out] */ AutoPtr<IMetaParameter>& param) override;

    ECode GetParameter(
        /* [in] */ const String& name,
        /* [out] */ AutoPtr<IMetaParameter>& param) override;

    ECode GetOutArgumentsNumber(
        /* [out] */ Integer& outArgs) override;

    ECode CreateArgumentList(
        /* [out] */ AutoPtr<IArgumentList>& argList) override;

    ECode Invoke(
        /* [in] */ IInterface* thisObject,
        /* [in] */ IArgumentList* argList) override;

    ECode InvokeDsa(
        /* [in] */ IInterface* thisObject,
        /* [in] */ Integer idxDsa,
        /* [in] */ IArgumentList* argList) override;

    ECode InvokeVm(
        /* [in] */ IInterface* thisObject,
        /* [in] */ IArgumentList* argList) override;

    ECode SetOpaque(
        /* [in] */ HANDLE opaque) override;

    ECode GetOpaque(
        /* [out] */ HANDLE &opaque) override;

    inline static CMetaMethod *From(IMetaMethod *method);

    Integer mHotCode;

private:
    ECode BuildAllParameters();

    ECode InvokeImpl(
        /* [in] */ IInterface* thisObject,
        /* [in] */ IArgumentList* argList,
        /* [in] */ CpuInvokeDsa invokeImpl);

    HANDLE mOpaque;

public:
    MetaMethod     *mMetadata;
    CMetaInterface *mOwner;
    Integer         mIndex;
    String          mName;
    String          mSignature;
    String          mStrFramacBlock;
    Array<IMetaParameter*> mParameters;
    Mutex           mParametersLock;
    Integer         mHasOutArguments;
    AutoPtr<IMetaType> mReturnType;

    HANDLE          mMethodAddr;
    HANDLE          mVobj;

    AutoPtr<IArgumentList>  mArgListMemory;
};

CMetaMethod *CMetaMethod::From(IMetaMethod* method)
{
    return static_cast<CMetaMethod*>(method);
}

} // namespace como

#endif // __COMO_CMETAMETHOD_H__
