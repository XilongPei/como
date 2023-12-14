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

#include "metadata/MetadataSerializer.h"
#include "reflection/CMetaComponent.h"
#include "reflection/CMetaCoclass.h"
#include "reflection/CMetaConstant.h"
#include "reflection/CMetaEnumeration.h"
#include "reflection/CMetaInterface.h"
#include "reflection/CMetaMethod.h"
#include "reflection/CMetaParameter.h"
#include "reflection/CMetaType.h"
#include <cstdlib>

namespace como {

COMO_INTERFACE_IMPL_LIGHT_1(CMetaComponent, LightRefBase, IMetaComponent);

CMetaComponent::CMetaComponent(
    /* [in] */ IClassLoader *loader,
    /* [in] */ ComoComponent *component,
    /* [in] */ MetaComponent *metadata)
    : mLoader(loader)
    , mComponent(component)
    , mMetadata(metadata)
    , mName(metadata->mName)
    , mUri(metadata->mUri)
    , mConstants(mMetadata->mConstantNumber)
    , mConstantNameMap(mMetadata->mConstantNumber)
    , mConstantsAlreadyBuilt(false)
    , mCoclasses(mMetadata->mCoclassNumber)
    , mCoclassNameMap(mMetadata->mCoclassNumber)
    , mCoclassIdMap(mMetadata->mCoclassNumber)
    , mCoclassesAlreadyBuilt(false)
    , mEnumerations(mMetadata->mEnumerationNumber -
                                          mMetadata->mExternalEnumerationNumber)
    , mEnumerationNameMap(mMetadata->mEnumerationNumber -
                                          mMetadata->mExternalEnumerationNumber)
    , mEnumerationsAlreadyBuilt(false)
    , mInterfaces(mMetadata->mInterfaceNumber -
                                            mMetadata->mExternalInterfaceNumber)
    , mInterfaceNameMap(mMetadata->mInterfaceNumber -
                                            mMetadata->mExternalInterfaceNumber)
    , mInterfaceIdMap(mMetadata->mInterfaceNumber -
                                            mMetadata->mExternalInterfaceNumber)
    , mInterfacesAlreadyBuilt(false)
    , mStrFramacBlock(mMetadata->mStrFramacBlock)
    , mOpaque(0)
{
    mCid.mUuid = metadata->mUuid;
    mCid.mUri = mUri.string();

    ECode ec;
    ec = LoadAllClassObjectGetters();
    if (FAILED(ec)) {
        mIInterface = nullptr;
    }

    ec = BuildIInterface();
    if (FAILED(ec)) {
        mIInterface = nullptr;
    }

#ifdef COMO_FUNCTION_SAFETY_RTOS
    /**
     * In the functional safety calculation of RTOS, there shall be no dynamic
     * memory call, and the metadata shall be handled well in advance.
     */
    if (FAILED(Preload())) {
        Logger::E("CMetaComponent", "BuildAll... failed.");
    }
#endif
}

CMetaComponent::~CMetaComponent()
{
    ReleaseResources();
}

ECode CMetaComponent::GetName(
    /* [ou] */ String& name)
{
    name = mName;
    return NOERROR;
}

ECode CMetaComponent::GetComponentID(
    /* [out] */ ComponentID& cid)
{
    cid = mCid;
    return NOERROR;
}

ECode CMetaComponent::GetConstantNumber(
    /* [out] */ Integer& number)
{
    number = mConstants.GetLength();
    return NOERROR;
}

ECode CMetaComponent::GetAllConstants(
    /* [out] */ Array<IMetaConstant*>& consts)
{
    if (mConstants.IsEmpty()) {
        return NOERROR;
    }

    FAIL_RETURN(BuildAllConstants());

    Integer N = MIN(mConstants.GetLength(), consts.GetLength());
    for (Integer i = 0;  i < N;  i++) {
        consts.Set(i, mConstants[i]);
    }

    return NOERROR;
}

ECode CMetaComponent::GetConstant(
    /* [in] */ const String& name,
    /* [out] */ AutoPtr<IMetaConstant>& constt)
{
    constt = nullptr;

    if (name.IsEmpty() || mConstants.IsEmpty()) {
        return NOERROR;
    }

    FAIL_RETURN(BuildAllConstants());

    constt = mConstantNameMap.Get(name);
    return NOERROR;
}

ECode CMetaComponent::GetCoclassNumber(
    /* [out] */ Integer& number)
{
    number = mCoclasses.GetLength();
    return NOERROR;
}

ECode CMetaComponent::GetAllCoclasses(
    /* [out] */ Array<IMetaCoclass*>& klasses)
{
    if (mCoclasses.IsEmpty()) {
        return NOERROR;
    }

    FAIL_RETURN(BuildAllCoclasses());

    Integer N = MIN(mCoclasses.GetLength(), klasses.GetLength());
    for (Integer i = 0;  i < N;  i++) {
        klasses.Set(i, mCoclasses[i]);
    }

    return NOERROR;
}

ECode CMetaComponent::GetCoclass(
    /* [in] */ const String& fullName,
    /* [out] */ AutoPtr<IMetaCoclass>& klass)
{
    klass = nullptr;

    if (fullName.IsEmpty() || mCoclasses.IsEmpty()) {
        return NOERROR;
    }

    FAIL_RETURN(BuildAllCoclasses());

    klass = mCoclassNameMap.Get(fullName);
    return NOERROR;
}

ECode CMetaComponent::GetCoclass(
    /* [in] */ const CoclassID& cid,
    /* [out] */ AutoPtr<IMetaCoclass>& klass)
{
    klass = nullptr;

    FAIL_RETURN(BuildAllCoclasses());

    klass = mCoclassIdMap.Get(cid.mUuid);
    return NOERROR;
}

ECode CMetaComponent::GetEnumerationNumber(
    /* [out] */ Integer& number)
{
    number = mEnumerations.GetLength();
    return NOERROR;
}

ECode CMetaComponent::GetAllEnumerations(
    /* [out] */ Array<IMetaEnumeration*>& enumns)
{
    if (mEnumerations.IsEmpty()) {
        return NOERROR;
    }

    FAIL_RETURN(BuildAllEnumerations());

    Integer N = MIN(mEnumerations.GetLength(), enumns.GetLength());
    for (Integer i = 0;  i < N;  i++) {
        enumns.Set(i, mEnumerations[i]);
    }

    return NOERROR;
}

ECode CMetaComponent::GetEnumeration(
    /* [in] */ const String& fullName,
    /* [out] */ AutoPtr<IMetaEnumeration>& enumn)
{
    enumn = nullptr;

    if (fullName.IsEmpty() || mEnumerations.IsEmpty()) {
        return NOERROR;
    }

    FAIL_RETURN(BuildAllEnumerations());

    enumn = mEnumerationNameMap.Get(fullName);
    return NOERROR;
}

ECode CMetaComponent::GetInterfaceNumber(
    /* [out] */ Integer& number)
{
    number = mInterfaces.GetLength();
    return NOERROR;
}

ECode CMetaComponent::GetAllInterfaces(
    /* [out] */ Array<IMetaInterface*>& intfs)
{
    if (mInterfaces.IsEmpty()) {
        return NOERROR;
    }

    FAIL_RETURN(BuildAllInterfaces());

    Integer N = MIN(mInterfaces.GetLength(), intfs.GetLength());
    for (Integer i = 0;  i < N;  i++) {
        intfs.Set(i, mInterfaces[i]);
    }

    return NOERROR;
}

ECode CMetaComponent::GetInterface(
    /* [in] */ const String& fullName,
    /* [out] */ AutoPtr<IMetaInterface>& metaIntf)
{
    metaIntf = nullptr;

    if (fullName.IsEmpty() || mInterfaces.IsEmpty()) {
        return NOERROR;
    }

    FAIL_RETURN(BuildAllInterfaces());

    metaIntf = mInterfaceNameMap.Get(fullName);
    return NOERROR;
}

ECode CMetaComponent::GetInterface(
    /* [in] */ const InterfaceID& iid,
    /* [out] */ AutoPtr<IMetaInterface>& metaIntf)
{
    metaIntf = nullptr;

    FAIL_RETURN(BuildAllInterfaces());
    metaIntf = mInterfaceIdMap.Get(iid.mUuid);
    return NOERROR;
}

ECode CMetaComponent::GetSerializedMetadata(
    /* [out, callee] */ Array<Byte>& metadata)
{
    MetaComponent* mmc = reinterpret_cast<MetaComponent*>(
                                        mComponent->mMetadataWrapper->mMetadata);
    metadata = Array<Byte>::Allocate(mmc->mSize);
    if (metadata.IsNull()) {
        Logger::E("CMetaComponent", "Malloc %lu size metadata failed.", mmc->mSize);
        return E_OUT_OF_MEMORY_ERROR;
    }
    memcpy(metadata.GetPayload(), mmc, mmc->mSize);
    return NOERROR;
}

ECode CMetaComponent::IsOnlyMetadata(
    /* [out] */ Boolean& onlyMetadata)
{
    onlyMetadata = mComponent->mSoHandle == nullptr;
    return NOERROR;
}

ECode CMetaComponent::LoadComponent(
    /* [in] */ IMetaComponent *comp)
{
    Boolean onlyMetadata;
    IsOnlyMetadata(onlyMetadata);
    if (onlyMetadata) {

        /**
         * Are there any problems with the program that used the metadata
         * (stored in mComponent) before?
         * No, because this is only metadata. The reflection program is the
         * interpretation of these data. Reflectors are not coupled with data.
         */
        if (mComponent != nullptr) {
            free(mComponent);
        }

        mComponent = (ComoComponent*)malloc(sizeof(ComoComponent));
        if (mComponent == nullptr) {
            Logger::E("CMetaComponent", "Malloc ComoComponent failed.");
            return E_OUT_OF_MEMORY_ERROR;
        }

        ComoComponent* component = CMetaComponent::From(comp)->mComponent;

        mComponent->mSoHandle = component->mSoHandle;
        mComponent->mSoGetClassObject = component->mSoGetClassObject;
        mComponent->mSoGetAllClassObjects = component->mSoGetAllClassObjects;
        mComponent->mSoCanUnload = component->mSoCanUnload;
        mComponent->mMetadataWrapper = component->mMetadataWrapper;

        FAIL_RETURN(LoadAllClassObjectGetters());
    }

    return NOERROR;
}

ECode CMetaComponent::Preload()
{
    FAIL_RETURN(BuildAllConstants());
    FAIL_RETURN(BuildAllEnumerations());
    FAIL_RETURN(BuildAllInterfaces());
    FAIL_RETURN(BuildAllCoclasses());
    return NOERROR;
}

ECode CMetaComponent::CanUnload(
    /* [out] */ Boolean& unload)
{
    unload = mComponent->mSoCanUnload != nullptr
                                        ? mComponent->mSoCanUnload()
                                        : false;
    return NOERROR;
}

ECode CMetaComponent::Unload()
{
    ECode ec = mLoader->UnloadComponent(mCid);
    if (SUCCEEDED(ec)) {
        ReleaseResources();
    }
    return ec;
}

ECode CMetaComponent::GetClassObject(
    /* [in] */ const CoclassID& cid,
    /* [out] */ AutoPtr<IClassObject>& object)
{
    ClassObjectGetter* getter = mClassObjects.Get(cid.mUuid);
    if (getter != nullptr) {
        return getter->mGetter(object);
    }
    if (mComponent->mSoGetClassObject != nullptr) {
        return mComponent->mSoGetClassObject(cid, object);
    }
    else {
        object = nullptr;
        return NOERROR;
    }
}

ECode CMetaComponent::BuildAllConstants()
{
    if (mMetadata->mConstantNumber == 0) {
        return NOERROR;
    }

    if (mConstantsAlreadyBuilt) {
        return NOERROR;
    }

    Mutex::AutoLock lock(mConstantsLock);

    if (mConstantsAlreadyBuilt) {
        return NOERROR;
    }

    for (Integer i = 0;  i < mMetadata->mConstantNumber;  i++) {
        MetaConstant *mc = mMetadata->mConstants[i];
        String fullName = String::Format("%s::%s", mc->mNamespace, mc->mName);
        AutoPtr<CMetaConstant> mcObj = new CMetaConstant(mMetadata, mc);
        if ((nullptr != mcObj) && (nullptr != mcObj->mValue)) {
            mConstants.Set(i, mcObj);
            mConstantNameMap.Put(fullName, mcObj);
        }
        else {
            return E_OUT_OF_MEMORY_ERROR;
        }
    }
    mConstantsAlreadyBuilt = true;
    return NOERROR;
}

ECode CMetaComponent::BuildAllCoclasses()
{
    if (mMetadata->mCoclassNumber == 0) {
        return NOERROR;
    }

    if (mCoclassesAlreadyBuilt) {
        return NOERROR;
    }

    Mutex::AutoLock lock(mCoclassesLock);

    if (mCoclassesAlreadyBuilt) {
        return NOERROR;
    }

    for (Integer i = 0;  i < mMetadata->mCoclassNumber;  i++) {
        MetaCoclass *mc = mMetadata->mCoclasses[i];
        String fullName = String::Format("%s::%s", mc->mNamespace, mc->mName);
        AutoPtr<CMetaCoclass> mcObj = new CMetaCoclass(this, mMetadata, mc);
        if ((nullptr != mcObj) && (! fullName.IsEmpty())) {
            mCoclasses.Set(i, mcObj);
            if (0 != mCoclassNameMap.Put(fullName, mcObj))
                return E_OUT_OF_MEMORY_ERROR;

            if (0 != mCoclassIdMap.Put(mcObj->mCid.mUuid, mcObj))
                return E_OUT_OF_MEMORY_ERROR;
        }
        else {
            // roll back this transaction?
            return E_OUT_OF_MEMORY_ERROR;
        }
    }
    mCoclassesAlreadyBuilt = true;
    return NOERROR;
}

ECode CMetaComponent::BuildAllEnumerations()
{
    if (mMetadata->mEnumerationNumber == 0) {
        return NOERROR;
    }

    if (mEnumerationsAlreadyBuilt) {
        return NOERROR;
    }

    Mutex::AutoLock lock(mEnumerationsLock);

    if (mEnumerationsAlreadyBuilt) {
        return NOERROR;
    }

    Integer index = 0;
    for (Integer i = 0;  i < mMetadata->mEnumerationNumber;  i++) {
        MetaEnumeration* me = mMetadata->mEnumerations[i];
        if (me->mProperties & TYPE_EXTERNAL) {
            continue;
        }
        String fullName = String::Format("%s::%s", me->mNamespace, me->mName);
        AutoPtr<CMetaEnumeration> meObj = new CMetaEnumeration(this, mMetadata, me);
        if ((nullptr != meObj) && (! fullName.IsEmpty())) {
            mEnumerations.Set(index, meObj);
            mEnumerationNameMap.Put(fullName, meObj);
            index++;
        }
        else {
            // roll back this transaction?
            return E_OUT_OF_MEMORY_ERROR;
        }
    }
    mEnumerationsAlreadyBuilt = true;
    return NOERROR;
}

ECode CMetaComponent::BuildAllInterfaces()
{
    if (mMetadata->mInterfaceNumber == 0) {
        return NOERROR;
    }

    if (mInterfacesAlreadyBuilt) {
        return NOERROR;
    }

    Mutex::AutoLock lock(mInterfacesLock);

    if (mInterfacesAlreadyBuilt) {
        return NOERROR;
    }

    Integer index = 0;
    for (Integer i = 0;  i < mMetadata->mInterfaceNumber;  i++) {
        MetaInterface *mi = mMetadata->mInterfaces[i];
        if (mi->mProperties & TYPE_EXTERNAL) {
            continue;
        }

        String fullName = String::Format("%s::%s", mi->mNamespace, mi->mName);
        if (! mInterfaceNameMap.ContainsKey(fullName)) {
            AutoPtr<CMetaInterface> miObj = new CMetaInterface(this, mMetadata, mi);
            if ((nullptr != miObj) && (! fullName.IsEmpty())) {
                mInterfaces.Set(index, miObj);
                mInterfaceNameMap.Put(fullName, miObj);
                mInterfaceIdMap.Put(miObj->mIid.mUuid, miObj);
                index++;
            }
            else {
                // roll back this transaction?
                return E_OUT_OF_MEMORY_ERROR;
            }
        }
    }
    mInterfacesAlreadyBuilt = true;
    return NOERROR;
}

AutoPtr<IMetaInterface> CMetaComponent::BuildInterface(
    /* [in] */ Integer index)
{
    if ((index < 0) || (index >= mMetadata->mInterfaceNumber)) {
        return nullptr;
    }

    MetaInterface *mi = mMetadata->mInterfaces[index];
    String fullName = String::Format("%s::%s", mi->mNamespace, mi->mName);
    if (fullName.IsEmpty()) {
        return nullptr;
    }

    if (! mInterfaceNameMap.ContainsKey(fullName)) {
        Mutex::AutoLock lock(mInterfacesLock);
        if (! mInterfaceNameMap.ContainsKey(fullName)) {
            AutoPtr<CMetaInterface> miObj = new CMetaInterface(this, mMetadata, mi);
            if (nullptr == miObj) {
                return nullptr;
            }

            Integer realIndex = index;
            for (Integer i = 0;  i <= index;  i++) {
                if (mMetadata->mInterfaces[i]->mProperties & TYPE_EXTERNAL) {
                    realIndex--;
                }
            }
            
            if (! (mi->mProperties & TYPE_EXTERNAL)) {
                mInterfaces.Set(realIndex, miObj);
            }

            mInterfaceNameMap.Put(fullName, miObj);
            mInterfaceIdMap.Put(miObj->mIid.mUuid, miObj);
            return miObj;
        }
    }

    return mInterfaceNameMap.Get(fullName);
}

ECode CMetaComponent::LoadAllClassObjectGetters()
{
    if (mComponent->mSoGetAllClassObjects != nullptr) {
        Integer N;
        ClassObjectGetter *getters = mComponent->mSoGetAllClassObjects(N);
        if (nullptr == getters) {
            return E_OUT_OF_MEMORY_ERROR;
        }

        for (Integer i = 0;  i < N;  i++) {
            const CoclassID& cid = getters[i].mCid;
            mClassObjects.Put(cid.mUuid, &getters[i]);
        }
    }

    return NOERROR;
}

/**
 * This function does not fully free memory if error E_OUT_OF_MEMORY_ERROR occurs.
 */
ECode CMetaComponent::BuildIInterface()
{
    CMetaInterface *miObj = new CMetaInterface();
    if (nullptr == miObj) {
        return E_OUT_OF_MEMORY_ERROR;
    }
    miObj->mOwner = this;
    miObj->mIid = IID_IInterface;
    miObj->mName = "IInterface";
    miObj->mNamespace = "como::";
    miObj->mMethods = Array<IMetaMethod*>(4);

    /**
     * method 0:
     * Integer AddRef([in] HANDLE id)
     **/
    CMetaMethod* mmObj = new CMetaMethod();
    if (nullptr == mmObj) {
        return E_OUT_OF_MEMORY_ERROR;
    }
    mmObj->mOwner = miObj;
    mmObj->mIndex = 0;
    mmObj->mName = "AddRef";
    mmObj->mSignature = "(H)I";
    mmObj->mParameters = Array<IMetaParameter*>(1);
    CMetaParameter* mpObj = new CMetaParameter();
    if (nullptr == mpObj) {
        return E_OUT_OF_MEMORY_ERROR;
    }
    mpObj->mOwner = mmObj;
    mpObj->mName = "id";
    mpObj->mIndex = 0;
    mpObj->mIOAttr = IOAttribute::IN;
    CMetaType* mtObj = new CMetaType();
    if (nullptr == mtObj) {
        return E_OUT_OF_MEMORY_ERROR;
    }
    mtObj->mKind = TypeKind::HANDLE;
    mtObj->mName = "HANDLE";
    mpObj->mType = mtObj;
    mmObj->mParameters.Set(0, mpObj);
    mtObj = new CMetaType();
    if (nullptr == mtObj) {
        return E_OUT_OF_MEMORY_ERROR;
    }
    mtObj->mKind = TypeKind::Integer;
    mtObj->mName = "Integer";
    mmObj->mReturnType = mtObj;
    miObj->mMethods.Set(0, mmObj);

    /**
     * method 1:
     * Integer Release([in] HANDLE id)
     **/
    mmObj = new CMetaMethod();
    if (nullptr == mmObj) {
        return E_OUT_OF_MEMORY_ERROR;
    }
    mmObj->mOwner = miObj;
    mmObj->mIndex = 1;
    mmObj->mName = "Release";
    mmObj->mSignature = "(H)I";
    mmObj->mParameters = Array<IMetaParameter*>(1);
    mpObj = new CMetaParameter();
    if (nullptr == mpObj) {
        return E_OUT_OF_MEMORY_ERROR;
    }
    mpObj->mOwner = mmObj;
    mpObj->mName = "id";
    mpObj->mIndex = 0;
    mpObj->mIOAttr = IOAttribute::IN;
    mtObj = new CMetaType();
    if (nullptr == mtObj) {
        return E_OUT_OF_MEMORY_ERROR;
    }
    mtObj->mKind = TypeKind::HANDLE;
    mtObj->mName = "HANDLE";
    mpObj->mType = mtObj;
    mmObj->mParameters.Set(0, mpObj);
    mtObj = new CMetaType();
    if (nullptr == mtObj) {
        return E_OUT_OF_MEMORY_ERROR;
    }
    mtObj->mKind = TypeKind::Integer;
    mtObj->mName = "Integer";
    mmObj->mReturnType = mtObj;
    miObj->mMethods.Set(1, mmObj);

    /**
     * method 2:
     * IInterface* ClassName::Probe([in] const InterfaceID& iid)
     **/
    mmObj = new CMetaMethod();
    if (nullptr == mmObj) {
        return E_OUT_OF_MEMORY_ERROR;
    }
    mmObj->mOwner = miObj;
    mmObj->mIndex = 2;
    mmObj->mName = "Probe";
    mmObj->mSignature = "(U)Lcomo/IInterface*";
    mmObj->mParameters = Array<IMetaParameter*>(1);
    mpObj = new CMetaParameter();
    if (nullptr == mpObj) {
        return E_OUT_OF_MEMORY_ERROR;
    }
    mpObj->mOwner = mmObj;
    mpObj->mName = "iid";
    mpObj->mIndex = 0;
    mpObj->mIOAttr = IOAttribute::IN;
    mtObj = new CMetaType();
    if (nullptr == mtObj) {
        return E_OUT_OF_MEMORY_ERROR;
    }
    mtObj->mKind = TypeKind::InterfaceID;
    mtObj->mName = "InterfaceID";
    mpObj->mType = mtObj;
    mmObj->mParameters.Set(0, mpObj);
    mtObj = new CMetaType();
    if (nullptr == mtObj) {
        return E_OUT_OF_MEMORY_ERROR;
    }
    mtObj->mKind = TypeKind::Interface;
    mtObj->mName = "IInterface";
    mtObj->mMode = TypeModification::POINTER;
    mmObj->mReturnType = mtObj;
    miObj->mMethods.Set(2, mmObj);

    /**
     * method 3:
     * ECode ClassName::GetInterfaceID([in]  IInterface* object,
     *                                 [out] InterfaceID& iid)
     */
    mmObj = new CMetaMethod();
    if (nullptr == mmObj) {
        return E_OUT_OF_MEMORY_ERROR;
    }
    mmObj->mOwner = miObj;
    mmObj->mIndex = 3;
    mmObj->mName = "GetInterfaceID";
    mmObj->mSignature = "(Lcomo/IInterface*U&)E";
    mmObj->mParameters = Array<IMetaParameter*>(2);
    mpObj = new CMetaParameter();
    if (nullptr == mpObj) {
        return E_OUT_OF_MEMORY_ERROR;
    }
    mpObj->mOwner = mmObj;
    mpObj->mName = "object";
    mpObj->mIndex = 0;
    mpObj->mIOAttr = IOAttribute::IN;
    mtObj = new CMetaType();
    if (nullptr == mtObj) {
        return E_OUT_OF_MEMORY_ERROR;
    }
    mtObj->mKind = TypeKind::Interface;
    mtObj->mName = "IInterface";
    mtObj->mMode = TypeModification::POINTER;
    mpObj->mType = mtObj;
    mmObj->mParameters.Set(0, mpObj);
    mpObj = new CMetaParameter();
    if (nullptr == mpObj) {
        return E_OUT_OF_MEMORY_ERROR;
    }
    mpObj->mOwner = mmObj;
    mpObj->mName = "iid";
    mpObj->mIndex = 1;
    mpObj->mIOAttr = IOAttribute::OUT;
    mtObj = new CMetaType();
    if (nullptr == mtObj) {
        return E_OUT_OF_MEMORY_ERROR;
    }
    mtObj->mKind = TypeKind::InterfaceID;
    mtObj->mName = "InterfaceID";
    mtObj->mMode = TypeModification::REFERENCE;
    mpObj->mType = mtObj;
    mmObj->mParameters.Set(1, mpObj);
    mtObj = new CMetaType();
    if (nullptr == mtObj) {
        return E_OUT_OF_MEMORY_ERROR;
    }
    mtObj->mKind = TypeKind::ECode;
    mtObj->mName = "ECode";
    mmObj->mReturnType = mtObj;
    miObj->mMethods.Set(3, mmObj);

    mIInterface = miObj;

    return NOERROR;
}

void CMetaComponent::ReleaseResources()
{
    mLoader = nullptr;
    if (mComponent != nullptr) {
        free(mComponent);
        mComponent = nullptr;
    }
    if (mMetadata != nullptr) {
        free(mMetadata);
        mMetadata = nullptr;
    }
    mCid.mUuid = UUID_ZERO;
    mCid.mUri = nullptr;
    mName = nullptr;
    mUri = nullptr;
    mConstants.Clear();
    mConstantNameMap.Clear();
    mConstantsAlreadyBuilt = false;
    mCoclasses.Clear();
    mCoclassNameMap.Clear();
    mCoclassIdMap.Clear();
    mCoclassesAlreadyBuilt = false;
    mEnumerations.Clear();
    mEnumerationNameMap.Clear();
    mEnumerationsAlreadyBuilt = false;
    mInterfaces.Clear();
    mInterfaceNameMap.Clear();
    mInterfaceIdMap.Clear();
    mInterfacesAlreadyBuilt = false;
}

ECode CMetaComponent::GetStrFramacBlock(
    /* [out] */ String& strFramacBlock)
{
    strFramacBlock = mStrFramacBlock;
    return NOERROR;
}

ECode CMetaComponent::SetOpaque(
    /* [in] */ HANDLE opaque)
{
    mOpaque = opaque;
    return NOERROR;
}

ECode CMetaComponent::GetOpaque(
    /* [out] */ HANDLE &opaque)
{
    opaque = mOpaque;
    return NOERROR;
}

Boolean CMetaComponent::ChkExternalInterface(
    /* [in] */ Integer index)
{
    if (index < 0 || index >= mMetadata->mInterfaceNumber) {
        return true;
    }

    MetaInterface *mi = mMetadata->mInterfaces[index];
    return mi->mProperties & TYPE_EXTERNAL;
}

} // namespace como
