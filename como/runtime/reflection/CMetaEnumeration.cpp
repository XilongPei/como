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
#include "reflection/CMetaEnumeration.h"
#include "reflection/CMetaEnumerator.h"
#include "reflection/reflection.h"

namespace como {

COMO_INTERFACE_IMPL_LIGHT_1(CMetaEnumeration, LightRefBase, IMetaEnumeration);

CMetaEnumeration::CMetaEnumeration(
    /* [in] */ CMetaComponent *mcObj,
    /* [in] */ MetaComponent *mc,
    /* [in] */ MetaEnumeration *me)
    : mMetadata(me)
    , mOwner(mcObj)
    , mName(me->mName)
    , mNamespace(me->mNamespace)
    , mEnumerators(me->mEnumeratorNumber)
{
    if (me->mProperties & TYPE_EXTERNAL) {
        char** externalPtr = reinterpret_cast<char**>(ALIGN((uintptr_t)me +
                                                      sizeof(MetaEnumeration)));
        mExternalModuleName = String(*externalPtr);
    }
    else {
        mExternalModuleName = nullptr;
    }

    mProperties = me->mProperties;

#ifdef COMO_FUNCTION_SAFETY_RTOS
    /**
     * In the functional safety calculation of RTOS, there shall be no dynamic
     * memory call, and the metadata shall be handled well in advance.
     */
    ECode ec;
    ec = BuildAllEnumerators();
    if (FAILED(ec)) {
        /**
         * Use the variable mEnumerators to identify whether the object was
         * successfully constructed.
         */
        mEnumerators->FreeData();

        Logger::E("CMetaEnumeration", "BuildAll... failed.");
    }
#endif
}

ECode CMetaEnumeration::GetComponent(
    /* [out] */ AutoPtr<IMetaComponent>& comp)
{
    comp = mOwner;
    return NOERROR;
}

ECode CMetaEnumeration::GetName(
    /* [out] */ String& name)
{
    name = mName;
    return NOERROR;
}

ECode CMetaEnumeration::GetNamespace(
    /* [out] */ String& ns)
{
    ns = (mNamespace.Equals(NAMESPACE_GLOBAL) ? "" : mNamespace);
    return NOERROR;
}

ECode CMetaEnumeration::GetEnumeratorNumber(
    /* [out] */ Integer& number)
{
    number = mMetadata->mEnumeratorNumber;
    return NOERROR;
}

ECode CMetaEnumeration::GetAllEnumerators(
    /* [out] */ Array<IMetaEnumerator*>& enumrs)
{
    if (mEnumerators.IsEmpty()) {
        return NOERROR;
    }

    FAIL_RETURN(BuildAllEnumerators());

    for (Integer i = 0;  i < mEnumerators.GetLength();  i++) {
        enumrs.Set(i, mEnumerators[i]);
    }

    return NOERROR;
}

ECode CMetaEnumeration::GetEnumerator(
    /* [in] */ const String& name,
    /* [out] */ AutoPtr<IMetaEnumerator>& enumr)
{
    if (name.IsEmpty() || mEnumerators.IsEmpty()) {
        enumr = nullptr;
        return NOERROR;
    }

    FAIL_RETURN(BuildAllEnumerators());

    for (Integer i = 0;  i < mEnumerators.GetLength();  i++) {
        String enumrName;
        mEnumerators[i]->GetName(enumrName);
        if (enumrName.Equals(name)) {
            enumr = mEnumerators[i];
            return NOERROR;
        }
    }

    enumr = nullptr;
    return NOERROR;
}

ECode CMetaEnumeration::GetExternalModuleName(
    /* [out] */ String& externalModuleName)
{
    externalModuleName = mExternalModuleName;
    return NOERROR;
}

ECode CMetaEnumeration::IsExternal(
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

ECode CMetaEnumeration::BuildAllEnumerators()
{
    if (nullptr == mEnumerators[0]) {
        Mutex::AutoLock lock(mEnumeratorsLock);

        if (nullptr == mEnumerators[0]) {
            for (Integer i = 0;  i < mMetadata->mEnumeratorNumber;  i++) {
                MetaEnumerator *me = mMetadata->mEnumerators[i];
                AutoPtr<IMetaEnumerator> meObj = new CMetaEnumerator(this, me);
                if (nullptr == meObj) {
                    for (Integer j = i - 1;  j >= 0;  j--) {
                        delete mEnumerators[j];
                    }
                    return E_OUT_OF_MEMORY_ERROR;
                }

                mEnumerators.Set(i, meObj);
            }
        }
    }
    return NOERROR;
}

} // namespace como
