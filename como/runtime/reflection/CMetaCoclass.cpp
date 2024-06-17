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
#include "reflection/CMetaCoclass.h"
#include "reflection/CMetaComponent.h"
#include "reflection/CMetaConstructor.h"
#include "reflection/CMetaConstant.h"
#include "reflection/reflection.h"

namespace como {

COMO_INTERFACE_IMPL_LIGHT_1(CMetaCoclass, LightRefBase, IMetaCoclass);

CMetaCoclass::CMetaCoclass(
    /* [in] */ CMetaComponent *mcObj,
    /* [in] */ MetaComponent *mc,
    /* [in] */ MetaCoclass *mk)
    : mMetadata(mk)
    , mOwner(mcObj)
    , mName(mk->mName)
    , mNamespace(mk->mNamespace)
    , mFuncSafetySetting(mk->mFuncSafetySetting)
    , mStrFramacBlock(mk->mStrFramacBlock)
    , mInterfaces(mk->mInterfaceNumber - 1)
    , mConstants(mk->mConstantNumber)
    , mOpaque(0)
{
    mCid.mUuid = mk->mUuid;
    mCid.mCid = &mcObj->mCid;

//没有索引，就找不到？？？
    MetaInterface *mi = mOwner->mMetadata->mInterfaces[
            mMetadata->mInterfaceIndexes[mMetadata->mInterfaceNumber - 1]];
    mConstructors = Array<IMetaConstructor*>(mi->mMethodNumber);

#ifdef COMO_FUNCTION_SAFETY_RTOS
    /**
     * In the functional safety calculation of RTOS, there shall be no dynamic
     * memory call, and the metadata shall be handled well in advance.
     */
    ECode ec;
    ec = BuildAllConstructors();
    ec |= BuildAllInterfaces();
    ec |= BuildAllMethods();
    ec |= BuildAllConstants();
    if (FAILED(ec)) {
        Logger::E("CMetaCoclass", "BuildAll... failed.");
    }
#endif
}

ECode CMetaCoclass::GetComponent(
    /* [out] */ AutoPtr<IMetaComponent>& comp)
{
    comp = mOwner;
    return NOERROR;
}

ECode CMetaCoclass::GetName(
    /* [out] */ String& name)
{
    name = mName;
    return NOERROR;
}

ECode CMetaCoclass::GetNamespace(
    /* [out] */ String& ns)
{
    ns = mNamespace.Equals(NAMESPACE_GLOBAL) ? "" : mNamespace;
    return NOERROR;
}

ECode CMetaCoclass::GetFuncSafetySetting(
    /* [out] */ String& funcSafetySetting)
{
    funcSafetySetting = mFuncSafetySetting;
    return NOERROR;
}

ECode CMetaCoclass::GetStrFramacBlock(
    /* [out] */ String& strFramacBlock)
{
    strFramacBlock = mStrFramacBlock;
    return NOERROR;
}

ECode CMetaCoclass::GetCoclassID(
    /* [out] */ CoclassID& cid)
{
    cid = mCid;
    return NOERROR;
}

ECode CMetaCoclass::GetClassLoader(
    /* [out] */ AutoPtr<IClassLoader>& loader)
{
    loader = mOwner->mLoader;
    return NOERROR;
}

ECode CMetaCoclass::GetConstructorNumber(
    /* [out] */ Integer& number)
{
    number = mConstructors.GetLength();
    return NOERROR;
}

ECode CMetaCoclass::GetAllConstructors(
    /* [out] */ Array<IMetaConstructor*>& constrs)
{
    if (mConstructors.IsEmpty()) {
        return NOERROR;
    }

    FAIL_RETURN(BuildAllConstructors());

    Integer N = MIN(mConstructors.GetLength(), constrs.GetLength());
    for (Integer i = 0;  i < N;  i++) {
        constrs.Set(i, mConstructors[i]);
    }
    return NOERROR;
}

ECode CMetaCoclass::GetConstructor(
    /* [in] */ const String& signature,
    /* [out] */ AutoPtr<IMetaConstructor>& constr)
{
    constr = nullptr;

    if (mConstructors.IsEmpty()) {
        return NOERROR;
    }

    FAIL_RETURN(BuildAllConstructors());

    if (signature.IsEmpty() && (mConstructors.GetLength() == 1)) {
        constr = mConstructors[0];
        return NOERROR;
    }

    for (Integer i = 0;  i < mConstructors.GetLength();  i++) {
        IMetaConstructor* mc = mConstructors[i];
        String mcSig;
        mc->GetSignature(mcSig);
        if (signature.Equals(mcSig)) {
            constr = mc;
            return NOERROR;
        }
    }

    return NOERROR;
}

ECode CMetaCoclass::GetInterfaceNumber(
    /* [out] */ Integer& number)
{
    number = mInterfaces.GetLength();
    return NOERROR;
}

ECode CMetaCoclass::GetAllInterfaces(
    /* [out] */ Array<IMetaInterface*>& intfs)
{
    if (mInterfaces.IsEmpty()) {
        return NOERROR;
    }

    FAIL_RETURN(BuildAllInterfaces());

    Integer N = MIN(mInterfaces.GetLength(), intfs.GetLength());
    for (Integer i = 0;  i < N;  i++) {
        IMetaInterface* miObj = mInterfaces[i];
        intfs.Set(i, miObj);
    }

    return NOERROR;
}

ECode CMetaCoclass::GetInterface(
    /* [in] */ const String& fullName,
    /* [out] */ AutoPtr<IMetaInterface>& intf)
{
    intf = nullptr;

    if (fullName.IsEmpty() || mInterfaces.IsEmpty()) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    FAIL_RETURN(BuildAllInterfaces());

    for (Integer i = 0;  i < mInterfaces.GetLength();  i++) {
        IMetaInterface* miObj = mInterfaces[i];
        String name, ns;
        miObj->GetName(name);
        miObj->GetNamespace(ns);
        if (fullName.Equals(ns + "::" + name)) {
            intf = miObj;
            return NOERROR;
        }
    }

    return NOERROR;
}

ECode CMetaCoclass::GetInterface(
    /* [in] */ const InterfaceID& iid,
    /* [out] */ AutoPtr<IMetaInterface>& intf)
{
    intf = nullptr;

    FAIL_RETURN(BuildAllInterfaces());

    for (Integer i = 0;  i < mInterfaces.GetLength();  i++) {
        IMetaInterface* miObj = mInterfaces[i];
        InterfaceID iidTmp;
        miObj->GetInterfaceID(iidTmp);
        if (iid == iidTmp) {
            intf = miObj;
            return NOERROR;
        }
    }

    return NOERROR;
}

ECode CMetaCoclass::ContainsInterface(
    /* [in] */ const String& fullName,
    /* [out] */ Boolean& result)
{
    result = false;

    if (fullName.IsEmpty() || mInterfaces.IsEmpty()) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    FAIL_RETURN(BuildAllInterfaces());

    for (Integer i = 0;  i < mInterfaces.GetLength();  i++) {
        IMetaInterface* miObj = mInterfaces[i];
        String name, ns;
        miObj->GetName(name);
        miObj->GetNamespace(ns);
        if (fullName.Equals(ns + "::" + name)) {
            result = true;
            return NOERROR;
        }
    }

    return NOERROR;
}

ECode CMetaCoclass::GetMethodNumber(
    /* [out] */ Integer& number)
{
    if (mMethods.IsEmpty()) {
        Mutex::AutoLock lock(mMethodsLock);
        if (mMethods.IsEmpty()) {
            Integer num = 4;
            for (Integer i = 0;  i < mMetadata->mInterfaceNumber - 1;  i++) {
                MetaInterface *mi = mOwner->mMetadata->mInterfaces[
                                                    mMetadata->mInterfaceIndexes[i]];
                String fullName = String::Format("%s::%s", mi->mNamespace, mi->mName);
                if (fullName.Equals("como::IInterface")) {
                    continue;
                }
#ifdef NOT_REFLECTION_TYPE_EXTERNAL
                if (fullName.Equals("como::IComoFunctionSafetyObject")) {
                    continue;
                }
#endif
                AutoPtr<IMetaInterface> miObj;
                GetInterface(fullName, miObj);
                Integer methodNum;

                if (nullptr == miObj) {
                    return E_RUNTIME_EXCEPTION;
                }

                miObj->GetMethodNumber(methodNum);
                num += methodNum - 4;
            }
            mMethods = Array<IMetaMethod*>(num);
        }
    }

    number = mMethods.GetLength();
    return NOERROR;
}

ECode CMetaCoclass::GetAllMethods(
    /* [out] */ Array<IMetaMethod*>& methods)
{
    ECode ec = BuildAllMethods();
    if (FAILED(ec)) {
        return ec;
    }

    Integer N = MIN(mMethods.GetLength(), methods.GetLength());
    for (Integer i = 0;  i < N;  i++) {
        methods.Set(i, mMethods[i]);
    }

    return NOERROR;
}

ECode CMetaCoclass::GetAllMethodsOverrideInfo(
    /* [out] */ Array<Boolean>& overridesInfo)
{
    ECode ec = BuildAllMethods();
    if (FAILED(ec)) {
        return ec;
    }

    Integer N = MIN(mOverridesInfo.GetLength(), overridesInfo.GetLength());
    for (Integer i = 0;  i < N;  i++) {
        overridesInfo.Set(i, mOverridesInfo[i]);
    }

    return NOERROR;
}

ECode CMetaCoclass::GetMethod(
    /* [in] */ const String& fullName,
    /* [in] */ const String& signature,
    /* [out] */ AutoPtr<IMetaMethod>& method)
{
    method = nullptr;

    if (fullName.IsEmpty() || mInterfaces.IsEmpty()) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    ECode ec = BuildAllMethods();
    if (FAILED(ec)) {
        return ec;
    }

    // fullName style: nameSpace.methodName
    Integer pos = fullName.IndexOf('.');
    Integer i;
    if (pos != -1) {
        IMetaInterface* miObj = nullptr;
        String nameSpace = fullName.Substring(0, pos-1);
        String methodName = fullName.Substring(pos+1);
        for (i = 0;  i < mInterfaces.GetLength();  i++) {
            miObj = mInterfaces[i];
            String name, ns;
            (void)miObj->GetName(name);
            (void)miObj->GetNamespace(ns);
            if (nameSpace.Equals(ns + "::" + name)) {
                break;
            }
        }

        if (i < mInterfaces.GetLength()) {
            return miObj->GetMethod(methodName, signature, method);
        }

        return NOERROR;
    }

    for (i = 0;  i < mMethods.GetLength();  i++) {
        IMetaMethod* mmObj = mMethods[i];
        String mmName, mmSignature;
        (void)mmObj->GetName(mmName);
        if (mmName.Equals(fullName)) {
            if (! mOverridesInfo[i]) {
                if (signature.IsEmpty()) {
                    method = mmObj;
                    return NOERROR;
                }
                (void)mmObj->GetSignature(mmSignature);
                if (mmSignature.Equals(signature)) {
                    method = mmObj;
                    return NOERROR;
                }
                break;
            }

            (void)mmObj->GetSignature(mmSignature);
            if (mmSignature.Equals(signature)) {
                method = mmObj;
                return NOERROR;
            }
        }
    }

    return NOERROR;
}

ECode CMetaCoclass::CreateObject(
    /* [out] */ IInterface** object)
{
    return CreateObject(IID_IInterface, object);
}

ECode CMetaCoclass::CreateObject(
    /* [in] */ const InterfaceID& iid,
    /* [out] */ IInterface** object)
{
    VALIDATE_NOT_NULL(object);
    *object = nullptr;

    AutoPtr<IClassObject> factory;
    ECode ec = mOwner->GetClassObject(mCid, factory);
    if (FAILED(ec)) {
        return ec;
    }

    factory->AttachMetadata(mOwner);
    return factory->CreateObject(iid, object);
}

ECode CMetaCoclass::BuildAllConstructors()
{
    if (nullptr == mConstructors[0]) {
        Mutex::AutoLock lock(mConstructorsLock);
        if (nullptr == mConstructors[0]) {
            MetaInterface* mi = mOwner->mMetadata->mInterfaces[
                 mMetadata->mInterfaceIndexes[mMetadata->mInterfaceNumber - 1]];
            for (Integer i = 0;  i < mi->mMethodNumber;  i++) {
                MetaMethod* mm = mi->mMethods[i];
                AutoPtr<IMetaConstructor> mcObj = new CMetaConstructor(
                                                               this, mi, i, mm);
                if (nullptr == mcObj) {
                    return E_OUT_OF_MEMORY_ERROR;
                }

                mConstructors.Set(i, mcObj);
            }
        }
    }

    return NOERROR;
}

ECode CMetaCoclass::BuildAllInterfaces()
{
    if (nullptr == mInterfaces[0]) {
        Mutex::AutoLock lock(mInterfacesLock);
        if (nullptr == mInterfaces[0]) {
            for (Integer i = 0;  i < mMetadata->mInterfaceNumber - 1;  i++) {
                Integer intfIndex = mMetadata->mInterfaceIndexes[i];
                AutoPtr<IMetaInterface> miObj = mOwner->BuildInterface(intfIndex);
                if (nullptr == miObj) {
                    return E_OUT_OF_MEMORY_ERROR;
                }

                mInterfaces.Set(i, miObj);
            }
        }
    }

    return NOERROR;
}

ECode CMetaCoclass::BuildAllMethods()
{
    Integer number;
    GetMethodNumber(number);

    if (nullptr == mMethods[0]) {
        Mutex::AutoLock lock(mMethodsLock);
        if (nullptr == mMethods[0]) {
            Integer index = 0;
            FAIL_RETURN(BuildInterfaceMethodLocked(mOwner->mIInterface, index));
            for (Integer i = 0;  i < mMetadata->mInterfaceNumber - 1;  i++) {
                AutoPtr<IMetaInterface> miObj = mOwner->BuildInterface(
                                                mMetadata->mInterfaceIndexes[i]);
                if (nullptr == miObj) {
                    return E_OUT_OF_MEMORY_ERROR;
                }

                String name, ns, nsName;
                (void)miObj->GetName(name);
                (void)miObj->GetNamespace(ns);
                nsName = ns + "::" + name;

                static String nsName_interface = String("como::IInterface");

                if (mOwner->ChkExternalInterface(mMetadata->mInterfaceIndexes[i])) {
                    continue;
                }

                if (nsName_interface.Equals(nsName)) {
                    continue;
                }

                FAIL_RETURN(BuildInterfaceMethodLocked(miObj, index));
            }

            /**
             * These 2 values (index, mMethods.GetLength()) may not be equal
             * because of ExternalInterface or como::IInterface, checked by upper
             * 2 continue statements.
             *
             * So in this case the statement is wrong:
             * Integer methodNumber = mMethods.GetLength();
             */
            Integer methodNumber = index;

            mOverridesInfo = Array<Boolean>(methodNumber);

            for (Integer i = 0;  i < methodNumber;  i++) {
                mOverridesInfo[i] = false;
            }

            for (Integer idxMethod = 0;  idxMethod < methodNumber;  idxMethod++) {
                if (mOverridesInfo[idxMethod]) {
                    continue;
                }

                String strMethodName;
                (void)mMethods[idxMethod]->GetName(strMethodName);
                String str_;
                for (Integer i = 0;  i < methodNumber;  i++) {
                    if (i < idxMethod) {
                        if (mOverridesInfo[i]) {
                            (void)mMethods[i]->GetName(str_);
                            if (strMethodName.Equals(str_)) {
                                mOverridesInfo[idxMethod] = true;
                                break;
                            }
                        }
                    }
                    else if (i > idxMethod) {
                        (void)mMethods[i]->GetName(str_);
                        if (strMethodName.Equals(str_)) {
                            mOverridesInfo[idxMethod] = true;
                            mOverridesInfo[i] = true;
                            break;
                        }
                    }
                }
            }
        }
    }

    return NOERROR;
}

ECode CMetaCoclass::BuildInterfaceMethodLocked(
    /* [in] */ IMetaInterface* miObj,
    /* [in, out] */ Integer& index)
{
    Integer N;
    miObj->GetMethodNumber(N);
    for (Integer i = ((miObj == mOwner->mIInterface) ? 0 : 4);  i < N;  i++) {
        AutoPtr<IMetaMethod> mmObj;
        FAIL_RETURN(miObj->GetMethod(i, mmObj));
        if (nullptr == (IMetaMethod *)mmObj) {
            return E_RUNTIME_EXCEPTION;
        }

        mMethods.Set(index, mmObj);
        index++;
    }

    return NOERROR;
}

ECode CMetaCoclass::SetOpaque(
    /* [in] */ HANDLE opaque)
{
    mOpaque = opaque;
    return NOERROR;
}

ECode CMetaCoclass::GetOpaque(
    /* [out] */ HANDLE &opaque)
{
    opaque = mOpaque;
    return NOERROR;
}

ECode CMetaCoclass::GetConstantNumber(
    /* [out] */ Integer& number)
{
    number = mConstants.GetLength();
    return NOERROR;
}

ECode CMetaCoclass::GetAllConstants(
    /* [out] */ Array<IMetaConstant*>& consts)
{
    FAIL_RETURN(BuildAllConstants());

    Integer N = MIN(mConstants.GetLength(), consts.GetLength());
    for (Integer i = 0;  i < N;  i++) {
        consts.Set(i, mConstants[i]);
    }
    return NOERROR;
}

ECode CMetaCoclass::GetConstant(
    /* [in] */ const String& name,
    /* [out] */ AutoPtr<IMetaConstant>& constt)
{
    if (name.IsEmpty() || mConstants.IsEmpty()) {
        constt = nullptr;
        return E_CONST_NOT_FOUND_EXCEPTION;
    }

    FAIL_RETURN(BuildAllConstants());

    for (Integer i = 0;  i < mConstants.GetLength();  i++) {
        String mcName;
        mConstants[i]->GetName(mcName);
        if (mcName.Equals(name)) {
            constt = mConstants[i];
            return NOERROR;
        }
    }
    constt = nullptr;
    return E_CONST_NOT_FOUND_EXCEPTION;
}

ECode CMetaCoclass::GetConstant(
    /* [in] */ Integer index,
    /* [out] */ AutoPtr<IMetaConstant>& constt)
{
    if ((index < 0) || (index >= mConstants.GetLength())) {
        constt = nullptr;
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    FAIL_RETURN(BuildAllConstants());

    constt = mConstants[index];
    return NOERROR;
}

ECode CMetaCoclass::BuildAllConstants()
{
    if (nullptr == mConstants[0]) {
        Mutex::AutoLock lock(mConstantsLock);
        if (nullptr == mConstants[0]) {
            for (Integer i = 0;  i < mMetadata->mConstantNumber;  i++) {
                AutoPtr<CMetaConstant> mcObj = new CMetaConstant(
                                   mOwner->mMetadata, mMetadata->mConstants[i]);
                if (nullptr != mcObj) {
                    mConstants.Set(i, mcObj);
                }
                else {
                    return E_OUT_OF_MEMORY_ERROR;
                }
            }
        }
    }

    return NOERROR;
}

} // namespace como
