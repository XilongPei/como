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

#include "reflHelpers.h"

namespace como {

ECode reflHelpers::constantGetLong(AutoPtr<IMetaConstant> constt,
                                                 String constName, Long& lvalue)
{
    ECode ec = NOERROR;
    AutoPtr<IMetaValue> value;
    constt->GetValue(value);

    AutoPtr<IMetaType> type;
    constt->GetType(type);
    TypeKind kind;
    type->GetTypeKind(kind);

    switch (kind) {
        case TypeKind::Boolean: {
            Boolean ivalue;
            value->GetBooleanValue(ivalue);
            lvalue = ivalue;
            break;
        }
        case TypeKind::Char: {
            Char ivalue;
            value->GetCharValue(ivalue);
            lvalue = ivalue;
            break;
        }
        case TypeKind::Byte: {
            Byte ivalue;
            value->GetByteValue(ivalue);
            lvalue = ivalue;
            break;
        }
        case TypeKind::Short: {
            Short ivalue;
            value->GetShortValue(ivalue);
            lvalue = ivalue;
            break;
        }
        case TypeKind::Integer: {
            Integer ivalue;
            value->GetIntegerValue(ivalue);
            lvalue = ivalue;
            break;
        }
        case TypeKind::Long:
            value->GetLongValue(lvalue);
            break;
        default:
            ec = E_TYPE_MISMATCH_EXCEPTION;     // not Long compatible datatype
    }

    return ec;
}

ECode reflHelpers::intfGetConstantLong(AutoPtr<IMetaInterface> intf,
                                                 String constName, Long& lvalue)
{
    if (nullptr == intf)
        return E_ILLEGAL_ARGUMENT_EXCEPTION;

    ECode ec;
    AutoPtr<IMetaConstant> constt;
    ec = intf->GetConstant(constName, constt);
    if (FAILED(ec) || (nullptr == constt))
        return ec;

    return constantGetLong(constt, constName, lvalue);
}

ECode reflHelpers::coclassGetConstantLong(AutoPtr<IMetaCoclass> mc,
                                                 String constName, Long& lvalue)
{
    if (nullptr == mc)
        return E_ILLEGAL_ARGUMENT_EXCEPTION;

    ECode ec;
    AutoPtr<IMetaConstant> constt;
    ec = mc->GetConstant(constName, constt);
    if (FAILED(ec) || (nullptr == constt))
        return ec;

    return constantGetLong(constt, constName, lvalue);
}

ECode reflHelpers::mmGetMethodInfo(AutoPtr<IMetaMethod> mm, String& strBuffer)
{
    String name;
    String signature;

    mm->GetName(name);
    mm->GetSignature(signature);
    strBuffer = name + "(" + signature + ")";

    return NOERROR;
}

ECode reflHelpers::intfGetObjectInfo(AutoPtr<IInterface> intf, String& strBuffer)
{
    AutoPtr<IMetaCoclass> klass;
    IObject::Probe(intf)->GetCoclass(klass);
    if (nullptr == klass) {
        strBuffer = "";
    }
    else {
        String name, ns;
        klass->GetName(name);
        klass->GetNamespace(ns);
        strBuffer = ns + "::" + name;
    }

    return NOERROR;
}

ECode reflHelpers::intfGetObjectInfo(AutoPtr<IInterface> intf,
                              const char *functionName, String& strClassInfo, 
                              String& strInterfaceInfo, String& methodSignature)
{
    ECode ec;

    AutoPtr<IMetaCoclass> klass;
    IObject::Probe(intf)->GetCoclass(klass);
    if (nullptr == klass) {
        strClassInfo = "";
        return NOERROR;
    }
    else {
        String name, ns;
        klass->GetName(name);
        klass->GetNamespace(ns);
        strClassInfo = ns + "::" + name;
    }

    InterfaceID iid;
    AutoPtr<IMetaInterface> imintf;
    intf->GetInterfaceID(intf, iid);
    klass->GetInterface(iid, imintf);
    if (nullptr == imintf) {
        strInterfaceInfo = "";
    }
    else {
        String name, ns;
        imintf->GetName(name);
        imintf->GetNamespace(ns);
        strInterfaceInfo = ns + "::" + name;
    }

    AutoPtr<IMetaMethod> method;
    klass->GetMethod(functionName, nullptr/*function signature*/, method);
    if (nullptr != method) {
        method->GetSignature(methodSignature);
    }

    return NOERROR;
}

} // namespace como
