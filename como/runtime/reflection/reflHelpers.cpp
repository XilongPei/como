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

ECode reflHelpers::constantGetLong(AutoPtr<IMetaConstant> constt, String constName, Long& lvalue)
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

ECode reflHelpers::intfGetConstantLong(AutoPtr<IMetaInterface> intf, String constName, Long& lvalue)
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

} // namespace como
