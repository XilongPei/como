//=========================================================================
// Copyright (C) 2018 The C++ Component Model(CCM) Open Source Project
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

#include "MetaResolver.h"
#include "../ast/Attribute.h"
#include "../ast/Enumeration.h"
#include "../ast/Interface.h"
#include "../ast/Method.h"
#include "../ast/Parameter.h"
#include "../ast/PointerType.h"
#include "../util/String.h"
#include "../util/Uuid.h"
#include "../../runtime/metadata/Component.h"
#include "../../runtime/type/ccmtypekind.h"

using ccdl::ast::Attribute;
using ccdl::ast::Enumeration;
using ccdl::ast::Interface;
using ccdl::ast::Method;
using ccdl::ast::Parameter;
using ccdl::ast::PointerType;

using ccm::CcmTypeKind;
using ccm::metadata::MetaEnumeration;
using ccm::metadata::MetaEnumerator;

namespace ccdl {
namespace metadata {

MetaResolver::MetaResolver(
    /* [in] */ Pool* pool,
    /* [in] */ void* metadata)
    : mPool(pool)
    , mMetaComponent(reinterpret_cast<MetaComponent*>(metadata))
    , mResolvingType(nullptr)
{}

void MetaResolver::InitializeModule()
{
    MetaComponent* mc = mMetaComponent;
    for (int i = 0; i < mc->mNamespaceNumber; i++) {
        MetaNamespace* mn = mc->mNamespaces[i];
        ResolveNamespace(mn);
    }
}

Type* MetaResolver::Resolve(
    /* [in] */ const String& fullName)
{
    mResolvingTypename = fullName;
    int index = fullName.LastIndexOf("::");
    String ns = index == -1 ? String("") : fullName.Substring(0, index + 1);
    String typeName = index == -1 ? fullName : fullName.Substring(index + 2);
    MetaComponent* mc = mMetaComponent;
    for (int i = 0; i < mc->mNamespaceNumber; i++) {
        MetaNamespace* mn = mc->mNamespaces[i];
        if (ns.Equals(mn->mName)) {
            return ResolveType(mn, typeName);
        }
    }
    return nullptr;
}

void MetaResolver::ResolveNamespace(
    /* [in] */ MetaNamespace* mn)
{
    Namespace* ns = mPool->ParseNamespace(String(mn->mName));
    ns->SetResolved(false);
}

Type* MetaResolver::ResolveType(
    /* [in] */ MetaNamespace* mn,
    /* [in] */ const String& typeName)
{
    for (int i = 0; i < mn->mEnumerationNumber; i++) {
        MetaEnumeration* me = mMetaComponent->mEnumerations[
                mn->mEnumerationIndexes[i]];
        if (typeName.Equals(me->mName)) {
            return BuildEnumeration(me);
        }
    }

    for (int i = 0; i < mn->mInterfaceNumber; i++) {
        MetaInterface* mi = mMetaComponent->mInterfaces[
                mn->mInterfaceIndexes[i]];
        if (typeName.Equals(mi->mName)) {
            return BuildInterface(mi);
        }
    }

    return nullptr;
}

Type* MetaResolver::BuildEnumeration(
    /* [in] */ MetaEnumeration* me)
{
    Namespace* ns = BuildNamespace(String(me->mNamespace));

    Enumeration* enumn = new Enumeration();
    enumn->SetName(String(me->mName));
    enumn->SetNamespace(ns);
    enumn->SetExternal(me->mExternal);

    for (int i = 0; i < me->mEnumeratorNumber; i++) {
        MetaEnumerator* enumr = me->mEnumerators[i];
        enumn->AddEnumerator(String(enumr->mName), enumr->mValue);
    }

    return (Type*)enumn;
}

Type* MetaResolver::BuildInterface(
    /* [in] */ MetaInterface* mi)
{
    Namespace* ns = BuildNamespace(String(mi->mNamespace));

    Interface* interface = new Interface();
    interface->SetName(String(mi->mName));
    interface->SetNamespace(ns);
    interface->SetExternal(mi->mExternal);
    if (mi->mBaseInterfaceIndex != -1) {
        MetaInterface* baseMi = mMetaComponent->mInterfaces[mi->mBaseInterfaceIndex];
        String baseIntfFullName = String::Format("%s%s", baseMi->mNamespace, baseMi->mName);
        Interface* baseIntf = mPool->FindInterface(baseIntfFullName);
        if (baseIntf == nullptr) {
            Type* type = Resolve(baseIntfFullName);
            if (type->IsInterfaceType()) {
                baseIntf = (Interface*)type;
            }
        }
        interface->SetBaseInterface(baseIntf);
    }
    mResolvingType = (Type*)interface;

    Attribute attr;
    attr.mUuid = Uuid(mi->mUuid).Dump();
    interface->SetAttribute(attr);
    mPool->AddInterface(interface);

    for (int i = 0; i < mi->mMethodNumber; i++) {
        Method* method = BuildMethod(mi->mMethods[i]);
        interface->AddMethod(method);
    }
    return (Type*)interface;
}

Namespace* MetaResolver::BuildNamespace(
    /* [in] */ const String& ns)
{
    if (ns.IsNullOrEmpty()) {
        return mPool->FindNamespace(String("__global__"));
    }

    return mPool->ParseNamespace(ns);
}

Method* MetaResolver::BuildMethod(
    /* [in] */ MetaMethod* mm)
{
    Method* method = new Method();
    method->SetName(String(mm->mName));
    Type* type = BuildType(mMetaComponent->mTypes[mm->mReturnTypeIndex]);
    method->SetReturnType(type);

    for (int i = 0; i < mm->mParameterNumber; i++) {
        Parameter* param = BuildParameter(mm->mParameters[i]);
        method->AddParameter(param);
    }
    return method;
}

Parameter* MetaResolver::BuildParameter(
    /* [in] */ MetaParameter* mp)
{
    Parameter* param = new Parameter();
    param->SetName(String(mp->mName));
    Type* type = BuildType(mMetaComponent->mTypes[mp->mTypeIndex]);
    param->SetType(type);
    param->SetAttribute(mp->mAttribute);
    return param;
}

Type* MetaResolver::BuildType(
    /* [in] */ MetaType* mt)
{
    String typeStr;

    switch (mt->mKind) {
        case CcmTypeKind::Char:
            typeStr = "ccm::Char";
            break;
        case CcmTypeKind::Byte:
            typeStr = "ccm::Byte";
            break;
        case CcmTypeKind::Short:
            typeStr = "ccm::Short";
            break;
        case CcmTypeKind::Integer:
            typeStr = "ccm::Integer";
            break;
        case CcmTypeKind::Long:
            typeStr = "ccm::Long";
            break;
        case CcmTypeKind::Float:
            typeStr = "ccm::Float";
            break;
        case CcmTypeKind::Double:
            typeStr = "ccm::Double";
            break;
        case CcmTypeKind::Boolean:
            typeStr = "ccm::Boolean";
            break;
        case CcmTypeKind::String:
            typeStr = "ccm::String";
            break;
        case CcmTypeKind::CoclassID:
            typeStr = "ccm::CoclassID";
            break;
        case CcmTypeKind::ComponentID:
            typeStr = "ccm::ComponentID";
            break;
        case CcmTypeKind::InterfaceID:
            typeStr = "ccm::InterfaceID";
            break;
        case CcmTypeKind::HANDLE:
            typeStr = "ccm::HANDLE";
            break;
        case CcmTypeKind::ECode:
            typeStr = "ccm::ECode";
            break;
        case CcmTypeKind::Enum: {
            MetaEnumeration* me = mMetaComponent->mEnumerations[mt->mIndex];
            typeStr = String::Format("%s%s", me->mNamespace, me->mName);
            break;
        }
        case CcmTypeKind::Array: {
            Type* elemType = BuildType(
                    mMetaComponent->mTypes[mt->mNestedTypeIndex]);
            typeStr = String::Format("Array<%s>", elemType->ToString().string());
            break;
        }
        case CcmTypeKind::Interface: {
            MetaInterface* mi = mMetaComponent->mInterfaces[mt->mIndex];
            typeStr = String::Format("%s%s", mi->mNamespace, mi->mName);
            break;
        }
        case CcmTypeKind::Triple:
            typeStr = "ccm::Triple";
            break;
    }

    Type* type = mResolvingTypename.Equals(typeStr) ? mResolvingType : mPool->FindType(typeStr);

    if (mt->mPointerNumber != 0) {
        Type* baseType = type;

        for (int i = 0; i < mt->mPointerNumber; ++i) {
            typeStr += "*";
        }

        type = mPool->FindType(typeStr);
        if (type == nullptr) {
            PointerType* ptrType = new PointerType();
            ptrType->SetBaseType(baseType);
            ptrType->SetPointerNumber(mt->mPointerNumber);
            mPool->AddTemporaryType(ptrType);
            type = ptrType;
        }
    }

    return type;
}

}
}
