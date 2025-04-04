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

#ifndef __COMO_CMETACOCLASS_H__
#define __COMO_CMETACOCLASS_H__

#include "metadata/Component.h"
#include "util/comoref.h"
#include "util/mutex.h"

namespace como {

class CMetaComponent;

class CMetaCoclass
    : public LightRefBase
    , public IMetaCoclass
{
public:
    CMetaCoclass(
        /* [in] */ CMetaComponent *mcObj,
        /* [in] */ MetaComponent *mc,
        /* [in] */ MetaCoclass *mk);

    COMO_INTERFACE_DECL();

    ECode GetComponent(
        /* [out] */ AutoPtr<IMetaComponent>& comp) override;

    ECode GetName(
        /* [out] */ String& name) override;

    ECode GetNamespace(
        /* [out] */ String& ns) override;

    ECode GetFuncSafetySetting(
        /* [out] */ String& funcSafetySetting) override;

    ECode GetStrFramacBlock(
        /* [out] */ String& strFramacBlock) override;

    ECode GetCoclassID(
        /* [out] */ CoclassID& cid) override;

    ECode GetClassLoader(
        /* [out] */ AutoPtr<IClassLoader>& loader) override;

    ECode GetConstructorNumber(
        /* [out] */ Integer& number) override;

    ECode GetAllConstructors(
        /* [out] */ Array<IMetaConstructor*>& constrs) override;

    ECode GetConstructor(
        /* [in] */ const String& paramNumber,
        /* [out] */ AutoPtr<IMetaConstructor>& constr) override;

    ECode GetInterfaceNumber(
        /* [out] */ Integer& number) override;

    ECode GetAllInterfaces(
        /* [out] */ Array<IMetaInterface*>& intfs) override;

    ECode GetInterface(
        /* [in] */ const String& fullName,
        /* [out] */ AutoPtr<IMetaInterface>& intf) override;

    ECode GetInterface(
        /* [in] */ const InterfaceID& iid,
        /* [out] */ AutoPtr<IMetaInterface>& intf) override;

    ECode ContainsInterface(
        /* [in] */ const String& fullName,
        /* [out] */ Boolean& result) override;

    ECode GetMethodNumber(
        /* [out] */ Integer& number) override;

    ECode GetAllMethods(
        /* [out] */ Array<IMetaMethod*>& methods) override;

    ECode GetAllMethodsOverrideInfo(
        /* [out] */ Array<Boolean>& overridesInfo) override;

    ECode GetMethod(
        /* [in] */ const String& fullName,
        /* [in] */ const String& signature,
        /* [out] */ AutoPtr<IMetaMethod>& method) override;

    ECode CreateObject(
        /* [out] */ IInterface** object) override;

    ECode CreateObject(
        /* [in] */ const InterfaceID& iid,
        /* [out] */ IInterface** object) override;

    ECode SetOpaque(
        /* [in] */ HANDLE opaque) override;

    ECode GetOpaque(
        /* [out] */ HANDLE &opaque) override;

    ECode GetConstantNumber(
        /* [out] */ Integer& number) override;

    ECode GetAllConstants(
        /* [out] */ Array<IMetaConstant*>& consts) override;

    ECode GetConstant(
        /* [in] */ const String& name,
        /* [out] */ AutoPtr<IMetaConstant>& constt) override;

    ECode GetConstant(
        /* [in] */ Integer index,
        /* [out] */ AutoPtr<IMetaConstant>& constt) override;

private:
    ECode BuildAllConstructors();

    ECode BuildAllInterfaces();

    ECode BuildAllConstants();

    ECode BuildAllMethods();

    ECode BuildInterfaceMethodLocked(
        /* [in] */ IMetaInterface *miObj,
        /* [in, out] */ Integer& index);

    HANDLE mOpaque;

public:
    MetaCoclass    *mMetadata;
    CMetaComponent *mOwner;
    CoclassID       mCid;
    String          mName;
    String          mNamespace;
    String          mFuncSafetySetting;
    String          mStrFramacBlock;
    Array<IMetaConstructor*> mConstructors;
    Mutex           mConstructorsLock;
    Array<IMetaMethod*> mMethods;
    Array<Boolean>  mOverridesInfo;
    Mutex           mMethodsLock { true };
    Array<IMetaInterface*> mInterfaces;
    Mutex           mInterfacesLock;
    Array<IMetaConstant*> mConstants;
    Mutex           mConstantsLock;
};

} // namespace como

#endif // __COMO_CMETACOCLASS_H__
