//=========================================================================
// Copyright (C) 2022 The C++ Component Model(COMO) Open Source Project
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

#include <stdlib.h>
#include "util/comolog.h"
#include "reflection/reflHelpers.h"
#include "MarshalUtils.h"

namespace como {

static int TypeKindSizeOf(TypeKind typeKind)
{
    switch (typeKind) {
        case TypeKind::Char:
            return sizeof(Char);
        case TypeKind::Byte:
            return sizeof(Byte);
        case TypeKind::Short:
            return sizeof(Short);
        case TypeKind::Integer:
            return sizeof(Integer);
        case TypeKind::Long:
            return sizeof(Long);
        case TypeKind::Float:
            return sizeof(Float);
        case TypeKind::Double:
            return sizeof(Double);
        case TypeKind::Boolean:
            return sizeof(Boolean);
        case TypeKind::HANDLE:
            return sizeof(HANDLE);
        case TypeKind::ECode:
            return sizeof(ECode);
        case TypeKind::Enum:
            return sizeof(Integer);
        /**
         * clang, warning: 10 enumeration values not handled in switch: 'Unknown',
         * 'String', 'CoclassID'... [-Wswitch]
         */
        default:
            ;
    }
    return 0;
}

/* TripleHexDump() output:

"array_var":["
  20 54 68 69 73 20 63 6c 61 73 73 20 69 73 20 75
  73 65 64 20 74 6f 20 63 72 65 61 74 65 20 61 20
  68 65 78 20 64 75 6d 70 20 6f 66 20 61 6e 79 20
"]
*/
static String& TripleHexDump(Triple& triple, String& strBuffer, int elemSize)
{
    const int perLine = abs(16 / elemSize);

    strBuffer.Reserve(triple.mSize * 4);
    strBuffer = "[\"\n";

    const unsigned char *pc = (const unsigned char *)triple.mData;
    for (int i = 0;  i < triple.mSize;) {
        if ((i % perLine) == 0) {
            if (i != 0) {
                strBuffer += String("\n");
            }
        }

        switch (elemSize) {
            case 2:
                strBuffer += String::Format(" %04X", *(Short*)&pc[i]);
                i += sizeof(Short);
                break;
            case 4:
                strBuffer += String::Format(" %08lX", *(Integer*)&pc[i]);
                i += sizeof(Integer);
                break;
            case 8:
                strBuffer += String::Format(" %016llX", *(Long*)&pc[i]);
                i += sizeof(Long);
                break;
            case -4:
                strBuffer += String::Format(" %10.2f", *(Float*)&pc[i]);
                i += sizeof(Float);
                break;
            case -8:
                strBuffer += String::Format(" %16.2lf", *(Double*)&pc[i]);
                i += sizeof(Double);
                break;
            default:
                strBuffer += String::Format(" %02X", pc[i]);
                i += sizeof(Byte);
        }
    }

    strBuffer += String("\n\"]\n");

    return strBuffer;
}

#define MakeParamString(str)    ("\"" + paramName + "\":" + str)

/**
 * 1. refer to InterfaceProxy::MarshalArguments in CProxy.cpp
 * 2. After UnMarshalArguments(), the argParcel->GetDataPosition is NOT 0
 * 3. The output data conforms to JSON specification
 */
ECode MarshalUtils::UnMarshalArguments(
    /* [in] */ IMetaMethod* method,
    /* [in] */ IParcel* argParcel,
    /* [out] */ String& strBuffer)
{
    // `8 * 3` corresponds to the four statements in CProxy.cpp
    // The data in parcel is 8-byte aligned
    //
    // inParcel->WriteInteger(RPC_MAGIC_NUMBER);
    // inParcel->WriteInteger(thisObj->mIndex);
    // inParcel->WriteInteger(methodIndex + 4);
    // inParcel->WriteLong(Mac::GetUuid64(uuid64));
    //
    argParcel->SetDataPosition(8 * 3);

    String methodName;
    method->GetName(methodName);
    strBuffer += ("{\"" + methodName + "\":{");

    Integer N;
    method->GetParameterNumber(N);
    for (Integer i = 0;  i < N;  i++) {
        AutoPtr<IMetaParameter> param;
        FAIL_RETURN(method->GetParameter(i, param));

        String paramName;
        AutoPtr<IMetaType> type;
        TypeKind kind;
        IOAttribute ioAttr;

        param->GetName(paramName);
        param->GetType(type);
        param->GetIOAttribute(ioAttr);
        type->GetTypeKind(kind);

        if ((ioAttr == IOAttribute::IN) || (ioAttr == IOAttribute::IN_OUT))  {
            switch (kind) {
                case TypeKind::Char: {
                    Char value;
                    argParcel->ReadChar(value);
                    if ('"' != value)
                        strBuffer += MakeParamString(String::Format("\"%c\"", value));
                    else
                        strBuffer += MakeParamString("\"\\\"\"");
                    break;
                }
                case TypeKind::Byte: {
                    Byte value;
                    argParcel->ReadByte(value);
                    if ('"' != value)
                        strBuffer += MakeParamString(String::Format("\"%c\"", value));
                    else
                        strBuffer += MakeParamString("\"\\\"\"");
                    break;
                }
                case TypeKind::Short: {
                    Short value;
                    argParcel->ReadShort(value);
                    strBuffer += MakeParamString(String::Format("%d", value));
                    break;
                }
                case TypeKind::Integer: {
                    Integer value;
                    argParcel->ReadInteger(value);
                    strBuffer += MakeParamString(String::Format("%d", value));
                    break;
                }
                case TypeKind::Long: {
                    Long value;
                    argParcel->ReadLong(value);
                    strBuffer += MakeParamString(String::Format("%lld", value));
                    break;
                }
                case TypeKind::Float: {
                    Float value;
                    argParcel->ReadFloat(value);
                    strBuffer += MakeParamString(String::Format("%f", value));
                    break;
                }
                case TypeKind::Double: {
                    Double value;
                    argParcel->ReadDouble(value);
                    strBuffer += MakeParamString(String::Format("%f", value));
                    break;
                }
                case TypeKind::Boolean: {
                    Boolean value;
                    argParcel->ReadBoolean(value);
                    if (value)
                        strBuffer += MakeParamString("true");
                    else
                        strBuffer += MakeParamString("false");
                    break;
                }
                case TypeKind::String: {
                    String value;
                    argParcel->ReadString(value);
                    strBuffer += MakeParamString(String("\"") +
                                    value.Replace("\"", "\\\"") + String("\""));
                    break;
                }
                case TypeKind::ECode: {
                    ECode value;
                    argParcel->ReadECode(value);
                    strBuffer += MakeParamString(String::Format("0x%X", value));
                    break;
                }
                case TypeKind::Enum: {
                    Integer value;
                    argParcel->ReadEnumeration(value);
                    strBuffer += MakeParamString(String::Format("%d", value));
                    break;
                }
                case TypeKind::Array: {
                    AutoPtr<IMetaType> arrType, elemType;
                    elemType = type;
                    TypeKind elemKind = kind;
                    while (elemKind == TypeKind::Array) {
                        arrType = elemType;
                        arrType->GetElementType(elemType);
                        elemType->GetTypeKind(elemKind);
                    }

                    Triple value;
                    argParcel->ReadArray(&value);
                    String strTemp;
                    strBuffer += MakeParamString(TripleHexDump(value, strTemp,
                                                     TypeKindSizeOf(elemKind)));
                    break;
                }
                case TypeKind::Interface: {
                    AutoPtr<IInterface> value;
                    argParcel->ReadInterface(value);
                    break;
                }
                case TypeKind::CoclassID:
                case TypeKind::ComponentID:
                case TypeKind::InterfaceID:
                case TypeKind::HANDLE:
                case TypeKind::Triple:
                default:
                    strBuffer += MakeParamString("null");
            } // switch
        }
        else {
            strBuffer += MakeParamString("null");
        }

        if (i < (N - 1))
            strBuffer += ",";
    } //for

    strBuffer += "}}";

    return NOERROR;
}

/**
 * 1. After UnUnmarshalResults(), the resParcel->GetDataPosition is NOT 0
 * 2. The output data conforms to JSON specification
 */
ECode MarshalUtils::UnUnmarshalResults(
    /* [in] */ IMetaMethod* method,
    /* [in] */ IParcel* resParcel,
    /* [out] */ String& strBuffer)
{
    // `8 * 2` corresponds to the two statements in CStub.cpp
    // The data in parcel is 8-byte aligned
    //
    // resParcel->WriteInteger(RPC_MAGIC_NUMBER);
    // resParcel->WriteLong(uuid64);
    //
    resParcel->SetDataPosition(8 * 2);

    String methodName;
    method->GetName(methodName);
    strBuffer += ("{\"" + methodName + "\":{");

    Integer N;
    method->GetParameterNumber(N);
    for (Integer i = 0;  i < N;  i++) {
        AutoPtr<IMetaParameter> param;
        FAIL_RETURN(method->GetParameter(i, param));

        String paramName;
        AutoPtr<IMetaType> type;
        TypeKind kind;
        IOAttribute ioAttr;

        param->GetName(paramName);
        param->GetType(type);
        param->GetIOAttribute(ioAttr);
        type->GetTypeKind(kind);

        if (ioAttr == IOAttribute::OUT || ioAttr == IOAttribute::IN_OUT) {
            switch (kind) {
                case TypeKind::Char: {
                    Char value;
                    resParcel->ReadChar(value);
                    if ('"' != value)
                        strBuffer += MakeParamString(String::Format("\"%c\"", value));
                    else
                        strBuffer += MakeParamString("\"\\\"\"");
                    break;
                }
                case TypeKind::Byte: {
                    Byte value;
                    resParcel->ReadByte(value);
                    if ('"' != value)
                        strBuffer += MakeParamString(String::Format("\"%c\"", value));
                    else
                        strBuffer += MakeParamString("\"\\\"\"");
                    break;
                }
                case TypeKind::Short: {
                    Short value;
                    resParcel->ReadShort(value);
                    strBuffer += MakeParamString(String::Format("%d", value));
                    break;
                }
                case TypeKind::Integer: {
                    Integer value;
                    resParcel->ReadInteger(value);
                    strBuffer += MakeParamString(String::Format("%d", value));
                    break;
                }
                case TypeKind::Long: {
                    Long value;
                    resParcel->ReadLong(value);
                    strBuffer += MakeParamString(String::Format("%lld", value));
                    break;
                }
                case TypeKind::Float: {
                    Float value;
                    resParcel->ReadFloat(value);
                    strBuffer += MakeParamString(String::Format("%f", value));
                    break;
                }
                case TypeKind::Double: {
                    Double value;
                    resParcel->ReadDouble(value);
                    strBuffer += MakeParamString(String::Format("%f", value));
                    break;
                }
                case TypeKind::Boolean: {
                    Boolean value;
                    resParcel->ReadBoolean(value);
                    if (value)
                        strBuffer += MakeParamString("true");
                    else
                        strBuffer += MakeParamString("false");
                    break;
                }
                case TypeKind::String: {
                    String value;
                    resParcel->ReadString(value);
                    strBuffer += MakeParamString(String("\"") +
                                     value.Replace("\"", "\\\"") + String("\""));
                    break;
                }
                case TypeKind::ECode: {
                    ECode value;
                    resParcel->ReadECode(value);
                    strBuffer += MakeParamString(String::Format("0x%X", value));
                    break;
                }
                case TypeKind::Enum: {
                    Integer value;
                    resParcel->ReadEnumeration(value);
                    strBuffer += MakeParamString(String::Format("%d", value));
                    break;
                }
                case TypeKind::Array: {
                    AutoPtr<IMetaType> arrType, elemType;
                    elemType = type;
                    TypeKind elemKind = kind;
                    while (elemKind == TypeKind::Array) {
                        arrType = elemType;
                        arrType->GetElementType(elemType);
                        elemType->GetTypeKind(elemKind);
                    }

                    Triple value;
                    resParcel->ReadArray(&value);
                    String strTemp;
                    strBuffer += MakeParamString(TripleHexDump(value, strTemp,
                                                     TypeKindSizeOf(elemKind)));
                    break;
                }
                case TypeKind::Interface: {
                    AutoPtr<IInterface> value;
                    resParcel->ReadInterface(value);
                    break;
                }
                case TypeKind::CoclassID:
                case TypeKind::ComponentID:
                case TypeKind::InterfaceID:
                case TypeKind::HANDLE:
                case TypeKind::Triple:
                default:
                    strBuffer += MakeParamString("nullptr");
            } // switch
        }
        else if (ioAttr == IOAttribute::OUT_CALLEE) {
            switch (kind) {
                case TypeKind::Array: {
                    AutoPtr<IMetaType> arrType, elemType;
                    elemType = type;
                    TypeKind elemKind = kind;
                    while (elemKind == TypeKind::Array) {
                        arrType = elemType;
                        arrType->GetElementType(elemType);
                        elemType->GetTypeKind(elemKind);
                    }

                    Triple value;
                    resParcel->ReadArray(&value);
                    String strTemp;
                    strBuffer += MakeParamString(TripleHexDump(value, strTemp,
                                                     TypeKindSizeOf(elemKind)));
                    break;
                }
                case TypeKind::Char:
                case TypeKind::Byte:
                case TypeKind::Short:
                case TypeKind::Integer:
                case TypeKind::Long:
                case TypeKind::Float:
                case TypeKind::Double:
                case TypeKind::Boolean:
                case TypeKind::String:
                case TypeKind::ECode:
                case TypeKind::Enum:
                case TypeKind::Interface:
                case TypeKind::CoclassID:
                case TypeKind::ComponentID:
                case TypeKind::InterfaceID:
                case TypeKind::Triple:
                default:
                    strBuffer += MakeParamString("null");
            } // switch
        }
        else {
            strBuffer += MakeParamString("null");
        }

        if (i < (N - 1))
            strBuffer += ",";
    } // for

    strBuffer += "}}";

    return NOERROR;
}

} // namespace como
