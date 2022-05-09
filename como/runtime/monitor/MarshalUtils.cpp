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

#include "util/comolog.h"
#include "reflection/reflHelpers.h"
#include "MarshalUtils.h"

namespace como {

/* TripleHexDump() output:

"array_var":["
  0000  20 54 68 69 73 20 63 6c 61 73 73 20 69 73 20 75   This class is u
  0010  73 65 64 20 74 6f 20 63 72 65 61 74 65 20 61 20  sed to create a
  0020  68 65 78 20 64 75 6d 70 20 6f 66 20 61 6e 79 20  hex dump of any
  0030  63 6f 6e 74 69 67 75 6f 75 73 20 6d 65 6d 6f 72  contiguous memor
  0040  79 20 61 72 65 61 2e 20 54 68 65 20 68 65 78 20  y area. The hex
"]
*/
static String& TripleHexDump(Triple& triple, String& strBuffer)
{
    const int perLine = 16;

    strBuffer.Reserve(triple.mSize * 4);
    strBuffer = "[\"\n";

    int i;
    unsigned char buff[perLine + 1];
    const unsigned char *pc = (const unsigned char *)triple.mData;

    // Process every byte in the data.
    for (i = 0;  i < triple.mSize;  i++) {
        // Multiple of perLine means new or first line (with line offset).
        if ((i % perLine) == 0) {
            // Only print previous-line ASCII buffer for lines beyond first.
            if (i != 0)
                strBuffer += String::Format("  %s\n", buff);

            // Output the offset of current line.
            strBuffer += String::Format("  %04x ", i);
        }

        // Now the hex code for the specific character.
        strBuffer += String::Format(" %02x", pc[i]);

        // And buffer a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e)) // isprint() may be better.
            buff[i % perLine] = '.';
        else
            buff[i % perLine] = pc[i];
        buff[(i % perLine) + 1] = '\0';
    }

    // Pad out last line if not exactly perLine characters.
    while ((i % perLine) != 0) {
        strBuffer += String::Format("   ");
        i++;
    }

    // And print the final ASCII buffer.
    strBuffer += String::Format("  %s\n\"]\n", buff);

    return strBuffer;
}

#define MakeParamString(str)    ("\"" + paramName + "\":" + str)

/**
 * refer to InterfaceProxy::MarshalArguments in CProxy.cpp
 *
 * The output data conforms to JSON specification
 */
ECode MarshalUtils::UnMarshalArguments(
    /* [in] */ IMetaMethod* method,
    /* [in] */ IParcel* argParcel,
    /* [out] */ String& strBuffer)
{
    // `sizeof(Integer)` * 3 corresponds to the three statements in CProxy.cpp
    //
    // inParcel->WriteInteger(RPC_MAGIC_NUMBER);
    // inParcel->WriteInteger(thisObj->mIndex);
    // inParcel->WriteInteger(methodIndex + 4);
    //
    argParcel->SetDataPosition(sizeof(Integer) * 3);

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
                    strBuffer += MakeParamString(TripleHexDump(value, strTemp));
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
 * The output data conforms to JSON specification
 */
ECode MarshalUtils::UnUnmarshalResults(
    /* [in] */ IMetaMethod* method,
    /* [in] */ IParcel* resParcel,
    /* [out] */ String& strBuffer)
{
    resParcel->SetDataPosition(0);

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
                    strBuffer += MakeParamString(TripleHexDump(value, strTemp));
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
                    strBuffer += MakeParamString(TripleHexDump(value, strTemp));
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
