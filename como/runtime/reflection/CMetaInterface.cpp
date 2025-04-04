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

#include "reflection/CMetaComponent.h"
#include "reflection/CMetaConstant.h"
#include "reflection/CMetaInterface.h"
#include "reflection/CMetaMethod.h"
#include "reflection/reflection.h"

namespace como {

COMO_INTERFACE_IMPL_LIGHT_1(CMetaInterface, LightRefBase, IMetaInterface);

CMetaInterface::CMetaInterface(
    /* [in] */ CMetaComponent *mcObj,
    /* [in] */ MetaComponent *mc,
    /* [in] */ MetaInterface *mi)
    : mMetadata(mi)
    , mOwner(mcObj)
    , mName(mi->mName)
    , mNamespace(mi->mNamespace)
    , mStrFramacBlock(mi->mStrFramacBlock)
    , mConstants(mi->mConstantNumber)
{
    mProperties = mi->mProperties;

    mIid.mUuid = mi->mUuid;
    mIid.mCid = &mcObj->mCid;
    BuildBaseInterface();
    mMethods = Array<IMetaMethod*>(CalculateMethodNumber());

    if (mi->mProperties & TYPE_EXTERNAL) {
        char** externalPtr = reinterpret_cast<char**>(ALIGN((uintptr_t)mi +
                                                        sizeof(MetaInterface)));
        mExternalModuleName = String(*externalPtr);
    }
    else {
        mExternalModuleName = nullptr;
    }

#ifdef COMO_FUNCTION_SAFETY_RTOS
    /**
     * In the functional safety calculation of RTOS, there shall be no dynamic
     * memory call, and the metadata shall be handled well in advance.
     */
    ECode ec;
    ec = BuildAllConstants();
    ec |= BuildAllMethods();
    if (FAILED(ec)) {

        /**
         * Use the variable mMethods to identify whether the object was
         * successfully constructed.
         */
        mMethods->FreeData();

        Logger::E("CMetaInterface", "BuildAll... failed.");
    }
#endif
}

ECode CMetaInterface::GetComponent(
    /* [out] */ AutoPtr<IMetaComponent>& metaComp)
{
    metaComp = mOwner;
    return NOERROR;
}

ECode CMetaInterface::GetName(
    /* [out] */ String& name)
{
    name = mName;
    return NOERROR;
}

ECode CMetaInterface::GetNamespace(
    /* [out] */ String& ns)
{
    ns = (mNamespace.Equals(NAMESPACE_GLOBAL) ? "" : mNamespace);
    return NOERROR;
}

ECode CMetaInterface::GetStrFramacBlock(
    /* [out] */ String& strFramacBlock)
{
    strFramacBlock = mStrFramacBlock;
    return NOERROR;
}

ECode CMetaInterface::GetInterfaceID(
    /* [out] */ InterfaceID& iid)
{
    iid = mIid;
    return NOERROR;
}

ECode CMetaInterface::GetBaseInterface(
    /* [out] */ AutoPtr<IMetaInterface>& baseIntf)
{
    baseIntf = mBaseInterface;
    return NOERROR;
}

ECode CMetaInterface::GetConstantNumber(
    /* [out] */ Integer& number)
{
    number = mConstants.GetLength();
    return NOERROR;
}

ECode CMetaInterface::GetAllConstants(
    /* [out] */ Array<IMetaConstant*>& consts)
{
    FAIL_RETURN(BuildAllConstants());

    Integer N = MIN(mConstants.GetLength(), consts.GetLength());
    for (Integer i = 0;  i < N;  i++) {
        consts.Set(i, mConstants[i]);
    }
    return NOERROR;
}

ECode CMetaInterface::GetConstant(
    /* [in] */ const String& name,
    /* [out] */ AutoPtr<IMetaConstant>& constt)
{
    constt = nullptr;

    if (name.IsEmpty() || mConstants.IsEmpty()) {
        return E_CONST_NOT_FOUND_EXCEPTION;
    }

    FAIL_RETURN(BuildAllConstants());

    for (Integer i = 0;  i < mConstants.GetLength();  i++) {
        String mcName;
        // It's just operation about reference count, No error checking required
        (void)mConstants[i]->GetName(mcName);
        if (mcName.Equals(name)) {
            constt = mConstants[i];
            return NOERROR;
        }
    }
    constt = nullptr;
    return E_CONST_NOT_FOUND_EXCEPTION;
}

ECode CMetaInterface::GetConstant(
    /* [in] */ Integer index,
    /* [out] */ AutoPtr<IMetaConstant>& constt)
{
    constt = nullptr;

    if ((index < 0) || (index >= mConstants.GetLength())) {
        constt = nullptr;
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    FAIL_RETURN(BuildAllConstants());

    constt = mConstants[index];
    return NOERROR;
}

ECode CMetaInterface::GetMethodNumber(
    /* [out] */ Integer& number)
{
    number = mMethods.GetLength();
    return NOERROR;
}

ECode CMetaInterface::GetAllMethods(
    /* [out] */ Array<IMetaMethod*>& methods)
{
    FAIL_RETURN(BuildAllMethods());

    Integer N = MIN(mMethods.GetLength(), methods.GetLength());
    for (Integer i = 0;  i < N;  i++) {
        methods.Set(i, mMethods[i]);
    }
    return NOERROR;
}

ECode CMetaInterface::GetDeclaredMethodNumber(
    /* [out] */ Integer& number)
{
    number = mMetadata->mMethodNumber;
    return NOERROR;
}

ECode CMetaInterface::GetDeclaredMethods(
    /* [out] */ Array<IMetaMethod*>& methods)
{
    FAIL_RETURN(BuildAllMethods());

    Integer offset = mMethods.GetLength() - mMetadata->mMethodNumber;
    Integer N = MIN(mMetadata->mMethodNumber, methods.GetLength());
    for (Integer i = 0;  i < N;  i++) {
        methods.Set(i, mMethods[i + offset]);
    }
    return NOERROR;
}

ECode CMetaInterface::GetMethod(
    /* [in] */ const String& name,
    /* [in] */ const String& signature,
    /* [out] */ AutoPtr<IMetaMethod>& method)
{
    method = nullptr;

    if (name.IsEmpty() || mMethods.IsEmpty()) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    FAIL_RETURN(BuildAllMethods());

    if (! signature.IsEmpty()) {
        for (Integer i = 0;  i < mMethods.GetLength();  i++) {
            IMetaMethod *mmObj = mMethods[i];
            String mmName, mmSignature;
            (void)mmObj->GetName(mmName);
            (void)mmObj->GetSignature(mmSignature);
            if (mmName.Equals(name) && mmSignature.Equals(signature)) {
                method = mmObj;
                return NOERROR;
            }
        }
    }
    else {
        for (Integer i = 0;  i < mMethods.GetLength();  i++) {
            IMetaMethod *mmObj = mMethods[i];
            String mmName;
            (void)mmObj->GetName(mmName);
            if (mmName.Equals(name)) {
                if (nullptr != method)
                    return E_INVALID_SIGNATURE_EXCEPTION;;

                method = mmObj;
            }
        }
    }

    return NOERROR;
}

ECode CMetaInterface::GetMethod(
    /* [in] */ Integer index,
    /* [out] */ AutoPtr<IMetaMethod>& method)
{
    method = nullptr;

    if ((index < 0) || (index >= mMethods.GetLength())) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    FAIL_RETURN(BuildAllMethods());

    method = mMethods[index];
    return NOERROR;
}

ECode CMetaInterface::IsExternal(
    /* [out] */ Char& properties)
{
    // char32_t == Char;
    /**
     * isExternal = (mProperties & (unsigned char)TYPE_EXTERNAL);
     */
    properties = 0;
    properties |= mProperties;
    return NOERROR;
}

Integer CMetaInterface::CalculateMethodNumber()
{
    MetaInterface *mi = mMetadata;
    Integer number = 0;
    while (mi != nullptr) {
        number += mi->mMethodNumber;
        if (mi->mBaseInterfaceIndex != -1) {
            mi = mOwner->mMetadata->mInterfaces[mi->mBaseInterfaceIndex];
        }
        else {
            mi = nullptr;
        }
    }
    return number;
}

ECode CMetaInterface::BuildBaseInterface()
{
    if (mMetadata->mBaseInterfaceIndex != -1) {
        AutoPtr<IMetaInterface> base =
                        mOwner->BuildInterface(mMetadata->mBaseInterfaceIndex);
        if (nullptr == base) {
            return E_OUT_OF_MEMORY_ERROR;
        }

        mBaseInterface = static_cast<CMetaInterface*>(base.Get());
    }
    return NOERROR;
}

ECode CMetaInterface::BuildAllConstants()
{
    if (nullptr == mConstants[0]) {
        Mutex::AutoLock lock(mConstantsLock);

        if (nullptr == mConstants[0]) {
            for (Integer i = 0;  i < mMetadata->mConstantNumber;  i++) {
                AutoPtr<CMetaConstant> mcObj = new CMetaConstant(
                                   mOwner->mMetadata, mMetadata->mConstants[i]);
                if ((nullptr == mcObj) || (nullptr == mcObj->mValue)) {
                    return E_OUT_OF_MEMORY_ERROR;
                }

                mConstants.Set(i, mcObj);
            }
        }
    }
    return NOERROR;
}

ECode CMetaInterface::BuildAllMethods()
{
    if (nullptr == mMethods[0]) {
        Mutex::AutoLock lock(mMethodsLock);

        if (nullptr == mMethods[0]) {
            if (BuildInterfaceMethod(mMetadata) < 0) {
                return E_OUT_OF_MEMORY_ERROR;
            }
        }
    }
    return NOERROR;
}

Integer CMetaInterface::BuildInterfaceMethod(
    /* [in] */ MetaInterface *mi)
{
    Integer startIndex = 0;
    if (mi->mBaseInterfaceIndex != -1) {
        startIndex = BuildInterfaceMethod(mOwner->mMetadata->mInterfaces[
                                                      mi->mBaseInterfaceIndex]);
        if (startIndex < 0) {
            return startIndex;
        }
    }

    for (Integer i = 0;  i < mi->mMethodNumber;  i++) {
        AutoPtr<CMetaMethod> mmObj = new CMetaMethod(mOwner->mMetadata,
                                         this, startIndex + i, mi->mMethods[i]);
        // Methods in all interfaces return values of type ECode,
        // so mmObj->mReturnType shouldn't be nullptr here.
        if ((nullptr == mmObj) || (nullptr == mmObj->mReturnType)) {
            for (Integer j = i - 1;  j >= 0;  j--) {
                delete mMethods[j];
            }
            return -1;
        }

        mMethods.Set(startIndex + i, mmObj);
    }
    return startIndex + mi->mMethodNumber;
}

ECode CMetaInterface::GetExternalModuleName(
    /* [out] */ String& externalModuleName)
{
    externalModuleName = mExternalModuleName;
    return NOERROR;
}

} // namespace como
