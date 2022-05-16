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

#include "reflection/CArgumentList.h"
#include "reflection/CMetaComponent.h"
#include "reflection/CMetaInterface.h"
#include "reflection/CMetaMethod.h"
#include "reflection/CMetaParameter.h"
#include "reflection/CMetaType.h"

namespace como {

EXTERN_C ECode invoke(
    /* [in] */ HANDLE func,
    /* [in] */ Byte* params,
    /* [in] */ Integer paramNum,
    /* [in] */ Integer stackParamNum,
    /* [in] */ struct ParameterInfo* paramInfos);

COMO_INTERFACE_IMPL_LIGHT_1(CMetaMethod, LightRefBase, IMetaMethod);

CMetaMethod::CMetaMethod()
    : mMetadata(nullptr)
    , mOwner(nullptr)
    , mIndex(0)
    , mHasOutArguments(0)
    , mOpaque(0)
    , mMethodAddr(0)
    , mHotCode(0)
{}

CMetaMethod::CMetaMethod(
    /* [in] */ MetaComponent* mc,
    /* [in] */ CMetaInterface* miObj,
    /* [in] */ Integer index,
    /* [in] */ MetaMethod* mm)
    : mMetadata(mm)
    , mOwner(miObj)
    , mIndex(index)
    , mName(mm->mName)
    , mSignature(mm->mSignature)
    , mParameters(mMetadata->mParameterNumber)
    , mHasOutArguments(0)
    , mOpaque(0)
    , mMethodAddr(0)
    , mHotCode(0)
{
    // there is no metadata in this component for external method
    if (miObj->mMetadata->mProperties & TYPE_EXTERNAL)
        mReturnType = nullptr;
    else {
        mReturnType = new CMetaType(mc, mc->mTypes[mm->mReturnTypeIndex]);
        if ((nullptr != mReturnType) &&
                             (nullptr == CMetaType::From(mReturnType)->mName)) {
            delete mReturnType;
            mReturnType = nullptr;
        }
    }
}

ECode CMetaMethod::GetInterface(
    /* [out] */ AutoPtr<IMetaInterface>& intf)
{
    intf = mOwner;
    return NOERROR;
}

ECode CMetaMethod::GetName(
    /* [out] */ String& name)
{
    name = mName;
    return NOERROR;
}

ECode CMetaMethod::GetSignature(
    /* [out] */ String& signature)
{
    signature = mSignature;
    return NOERROR;
}

ECode CMetaMethod::GetParameterNumber(
    /* [out] */ Integer& number)
{
    number = mParameters.GetLength();
    return NOERROR;
}

ECode CMetaMethod::GetAllParameters(
    /* [out] */ Array<IMetaParameter*>& params)
{
    FAIL_RETURN(BuildAllParameters());

    Integer N = MIN(mParameters.GetLength(), params.GetLength());
    for (Integer i = 0; i < N; i++) {
        params.Set(i, mParameters[i]);
    }
    return NOERROR;
}

ECode CMetaMethod::GetParameter(
    /* [in] */ Integer index,
    /* [out] */ AutoPtr<IMetaParameter>& param)
{
    if (index < 0 || index >= mMetadata->mParameterNumber) {
        param = nullptr;
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    FAIL_RETURN(BuildAllParameters());

    param = mParameters[index];
    return NOERROR;
}

ECode CMetaMethod::GetParameter(
    /* [in] */ const String& name,
    /* [out] */ AutoPtr<IMetaParameter>& param)
{
    if (name.IsEmpty() || mParameters.IsEmpty()) {
        param = nullptr;
        return NOERROR;
    }

    FAIL_RETURN(BuildAllParameters());

    for (Integer i = 0; i < mParameters.GetLength(); i++) {
        IMetaParameter* mpObj = mParameters[i];
        String mpName;
        mpObj->GetName(mpName);
        if (name.Equals(mpName)) {
            param = mpObj;
            return NOERROR;
        }
    }
    param = nullptr;
    return NOERROR;
}

ECode CMetaMethod::GetOutArgumentsNumber(
    /* [out] */ Integer& outArgs)
{
    FAIL_RETURN(BuildAllParameters());

    outArgs = mHasOutArguments;
    return NOERROR;
}

ECode CMetaMethod::CreateArgumentList(
    /* [out] */ AutoPtr<IArgumentList>& argList)
{
    FAIL_RETURN(BuildAllParameters());

    // `SMALL_PARAM_BUFFER`
    void* addr;
    addr = calloc(sizeof(CArgumentList) + sizeof(Long) * mHasOutArguments, 1);
    if (nullptr == addr)
        return E_OUT_OF_MEMORY_ERROR;

    CArgumentList *cArgList = new(addr) CArgumentList(mParameters, mHasOutArguments);
    if (nullptr == cArgList->GetParameterBuffer()) {
        free(addr);
        return E_OUT_OF_MEMORY_ERROR;
    }
    argList = (IArgumentList*)cArgList;

    return NOERROR;
}

ECode CMetaMethod::InvokeImpl(
    /* [in] */ IInterface* thisObject,
    /* [in] */ IArgumentList* argList,
    /* [in] */ CpuInvokeDsa invokeImpl)
{
    struct VTable
    {
        HANDLE mMethods[0];
    };

    struct VObject
    {
        VTable* mVtab;
    };

    CArgumentList* args = (CArgumentList*)argList;

    Byte* params = args->GetParameterBuffer();
    Integer paramNum = args->GetParameterNumber();
    ParameterInfo* paramInfos = args->GetParameterInfos();
    Integer intParamNum = 1, fpParamNum = 0;
    for (Integer i = 0; i < paramNum; i++) {
        if (paramInfos[i].mNumberType == NUMBER_TYPE_INTEGER) {
            intParamNum++;
        }
        else {
            fpParamNum++;
        }
    }
#if defined(__aarch64__)

#if defined(ARM_FP_SUPPORT)
    Integer stackParamNum = (intParamNum > 8 ? intParamNum - 8 : 0) +
                            (fpParamNum > 8 ? fpParamNum - 8 : 0);
#else
    Integer stackParamNum = (intParamNum > 8 ? intParamNum - 8 : 0) + fpParamNum;
#endif

#elif defined(__arm__)

#if defined(ARM_FP_SUPPORT)
    Integer stackParamNum = (intParamNum > 8 ? intParamNum - 8 : 0) +
                            (fpParamNum > 8 ? fpParamNum - 8 : 0);
#else
    Integer stackParamNum = (intParamNum > 8 ? intParamNum - 8 : 0) + fpParamNum;
#endif

#elif defined(__x86_64__)
    Integer stackParamNum = (intParamNum > 6 ? intParamNum - 6 : 0) +
                            (fpParamNum > 8 ? fpParamNum - 8 : 0);
#elif defined(__i386__)
    Integer stackParamNum = (intParamNum > 6 ? intParamNum - 6 : 0) +
                            (fpParamNum > 8 ? fpParamNum - 8 : 0);
#elif defined(__riscv)
    #if (__riscv_xlen == 64)
        Integer stackParamNum = (intParamNum > 8 ? intParamNum - 8 : 0) +
                                (fpParamNum > 8 ? fpParamNum - 8 : 0);
    #endif
#else
    #error Unknown Architecture
#endif
    if (mMethodAddr == 0) {
        VObject* vobj = reinterpret_cast<VObject*>(thisObject->Probe(mOwner->mIid));
        mVobj = reinterpret_cast<HANDLE>(vobj);
        reinterpret_cast<HANDLE*>(params)[0] = mVobj;
        mMethodAddr = vobj->mVtab->mMethods[mIndex];
    } else {
        reinterpret_cast<HANDLE*>(params)[0] = mVobj;
    }

    if (invokeImpl == nullptr)
        return invoke(mMethodAddr, params, paramNum + 1, stackParamNum, paramInfos);

    return invokeImpl(mMethodAddr, params, paramNum + 1, stackParamNum, paramInfos);
}

ECode CMetaMethod::Invoke(
    /* [in] */ IInterface* thisObject,
    /* [in] */ IArgumentList* argList)
{
#ifdef ENABLE_RUNTIME_LOGGER
    String methodName;
    GetName(methodName);
    Logger::V("CMetaMethod::Invoke", "%s", (const char*)methodName);
#endif

    return InvokeImpl(thisObject, argList, nullptr);
}

/*
 * call method from DSA(Domain Specific Architecture)
 */
ECode CMetaMethod::InvokeDsa(
    /* [in] */ IInterface* thisObject,
    /* [in] */ Integer idxDsa,
    /* [in] */ IArgumentList* argList)
{
#ifdef ENABLE_RUNTIME_LOGGER
    String methodName;
    GetName(methodName);
    Logger::V("CMetaMethod::InvokeDsa", "%s", (const char*)methodName);
#endif

    return InvokeImpl(thisObject, argList, ComoConfig::cpuInvokeDsa[idxDsa]);
}

/*
 * check object, then call a method
 */
ECode CMetaMethod::InvokeVm(
    /* [in] */ IInterface* thisObject,
    /* [in] */ IArgumentList* argList)
{
#ifdef ENABLE_RUNTIME_LOGGER
    String methodName;
    GetName(methodName);
    Logger::V("CMetaMethod::InvokeVm", "%s", (const char*)methodName);
#endif

    // check object
    ECode ec = NOERROR;
    if (FAILED(ec)) {
        return ec;
    }

    return InvokeImpl(thisObject, argList, nullptr);
}

ECode CMetaMethod::SetOpaque(
    /* [in] */ HANDLE opaque)
{
    mOpaque = opaque;
    return NOERROR;
}

ECode CMetaMethod::GetOpaque(
    /* [out] */ HANDLE &opaque)
{
    opaque = mOpaque;
    return NOERROR;
}

ECode CMetaMethod::BuildAllParameters()
{
    if (mParameters[0] == nullptr) {
        Mutex::AutoLock lock(mParametersLock);
        if (mParameters[0] == nullptr) {
            for (Integer i = 0; i < mMetadata->mParameterNumber; i++) {
                AutoPtr<CMetaParameter> mpObj = new CMetaParameter(
                                                mOwner->mOwner->mMetadata, this,
                                                  mMetadata->mParameters[i], i);
                if ((nullptr == mpObj) || (nullptr == mpObj->mType))
                    return E_OUT_OF_MEMORY_ERROR;

                mParameters.Set(i, mpObj);
                if ((mpObj->mIOAttr == IOAttribute::OUT) ||
                                      (mpObj->mIOAttr == IOAttribute::IN_OUT)) {
                    mHasOutArguments++;
                }
            }
        }
    }
    return NOERROR;
}

} // namespace como
