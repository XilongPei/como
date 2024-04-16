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

//=========================================================================
// Copyright (C) 2012 The Elastos Open Source Project
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

#include "comorpc.h"
#include "CStub.h"
#include "registry.h"
#include "reflection/reflHelpers.h"
#include "reflection/CMetaMethod.h"
#include "reflection/CArgumentList.h"
#include "RuntimeMonitor.h"

namespace como {

Integer InterfaceStub::AddRef(
    /* [in] */ HANDLE id)
{
    return 1;
}

Integer InterfaceStub::Release(
    /* [in] */ HANDLE id)
{
    return 1;
}

ECode InterfaceStub::UnmarshalArguments(
    /* [in] */ IMetaMethod* method,
    /* [in] */ IParcel* argParcel,
    /* [out] */ AutoPtr<IArgumentList>& argList)
{
    ECode ec = method->CreateArgumentList(argList);
    if (FAILED(ec))
        return ec;

    Integer iOutParam = 0;
    CArgumentList* cArglist = CArgumentList::From(argList);
    cArglist->mHotCode = CMetaMethod::From(method)->mHotCode;

    CMetaMethod* cMethod = CMetaMethod::From(method);
    Integer N = cMethod->mParameters.GetLength();
    if (N > 0) {
        // Help me to call BuildAllParameters once.
        Integer outArgs;
        method->GetOutArgumentsNumber(outArgs);
    }
    else
        return NOERROR;

    for (Integer i = 0;  i < N;  i++) {
        IMetaParameter* param = cMethod->mParameters[i];

        AutoPtr<IMetaType> type;
        TypeKind kind;
        IOAttribute ioAttr;

        // just an assignment, needn't to check the return value
        param->GetIOAttribute(ioAttr);
        param->GetType(type);
        type->GetTypeKind(kind);

        if (ioAttr == IOAttribute::IN) {
            switch (kind) {
                case TypeKind::Char: {
                    Char value;
                    argParcel->ReadChar(value);
                    argList->SetInputArgumentOfChar(i, value);
                    break;
                }
                case TypeKind::Byte: {
                    Byte value;
                    argParcel->ReadByte(value);
                    argList->SetInputArgumentOfByte(i, value);
                    break;
                }
                case TypeKind::Short: {
                    Short value;
                    argParcel->ReadShort(value);
                    argList->SetInputArgumentOfShort(i, value);
                    break;
                }
                case TypeKind::Integer: {
                    Integer value;
                    argParcel->ReadInteger(value);
                    argList->SetInputArgumentOfInteger(i, value);
                    break;
                }
                case TypeKind::Long: {
                    Long value;
                    argParcel->ReadLong(value);
                    argList->SetInputArgumentOfLong(i, value);
                    break;
                }
                case TypeKind::Float: {
                    Float value;
                    argParcel->ReadFloat(value);
                    argList->SetInputArgumentOfFloat(i, value);
                    break;
                }
                case TypeKind::Double: {
                    Double value;
                    argParcel->ReadDouble(value);
                    argList->SetInputArgumentOfDouble(i, value);
                    break;
                }
                case TypeKind::Boolean: {
                    Boolean value;
                    argParcel->ReadBoolean(value);
                    argList->SetInputArgumentOfBoolean(i, value);
                    break;
                }
                case TypeKind::String: {
                    String* value = new String();
                    if (nullptr == value) {
                        Logger::E("InterfaceStub::UnmarshalArguments", "new String() error");
                        return E_OUT_OF_MEMORY_ERROR;
                    }

                    argParcel->ReadString(*value);
                    argList->SetInputArgumentOfString(i, *value);
                    break;
                }
                case TypeKind::ECode: {
                    ECode value;
                    argParcel->ReadECode(value);
                    argList->SetInputArgumentOfECode(i, value);
                    break;
                }
                case TypeKind::Enum: {
                    Integer value;
                    argParcel->ReadEnumeration(value);
                    argList->SetInputArgumentOfEnumeration(i, value);
                    break;
                }
                case TypeKind::Array: {
                    Triple* t = new Triple();
                    if (nullptr == t) {
                        Logger::E("InterfaceStub::UnmarshalArguments", "new Triple() error");
                        return E_OUT_OF_MEMORY_ERROR;
                    }

                    argParcel->ReadArray(t);
                    argList->SetInputArgumentOfArray(i, *t);
                    break;
                }
                case TypeKind::Interface: {
                    AutoPtr<IInterface> value;
                    argParcel->ReadInterface(value);
                    argList->SetInputArgumentOfInterface(i, value);

                    //@ `REFCOUNT`
                    // MarshalResults/UnmarshalArguments organization is equivalent to an
                    // intermediary. The intermediary holds the reference count of the
                    // object. Because the parameter and result conversion of COMO is not
                    // managed by the C++ scope, the intermediary needs to manage itself.
                    // The correctness of the reference data can be guaranteed only when
                    // the control is handed over to the caller or callee

                    // UnmarshalArguments, IN, parameter
                    REFCOUNT_ADD(value);
                    break;
                }
                case TypeKind::CoclassID:
                case TypeKind::ComponentID:
                case TypeKind::InterfaceID:
                case TypeKind::HANDLE:
                case TypeKind::Triple:
                default:
                    Logger::E("CStub", "Invalid [in] type(%d), param index: %d.\n", kind, i);
                    return E_ILLEGAL_ARGUMENT_EXCEPTION;
            }
        }
        else if (ioAttr == IOAttribute::IN_OUT || ioAttr == IOAttribute::OUT) {
            switch (kind) {
                case TypeKind::Char: {
                    //@ `SMALL_PARAM_BUFFER`
                    //
                    // The [out] parameter whose size is less than Long is
                    // directly stored in GetOutParamBuffer(),
                    // CArgumentList::mOutParameterBuffer
                    //Char* value = new Char;
                    Char* value = (Char*)cArglist->GetOutParamBuffer(iOutParam++);
                    if (nullptr == value) {
                        Logger::E("InterfaceStub::UnmarshalArguments", "new Char() error");
                        return E_OUT_OF_MEMORY_ERROR;
                    }

                    if (ioAttr == IOAttribute::IN_OUT) {
                        argParcel->ReadChar(*value);
                    }
                    argList->SetOutputArgumentOfChar(i, reinterpret_cast<HANDLE>(value));
                    break;
                }
                case TypeKind::Byte: {
                    // `SMALL_PARAM_BUFFER`
                    //Byte* value = new Byte;
                    Byte* value = (Byte*)cArglist->GetOutParamBuffer(iOutParam++);
                    if (nullptr == value) {
                        Logger::E("InterfaceStub::UnmarshalArguments", "new Byte() error");
                        return E_OUT_OF_MEMORY_ERROR;
                    }

                    if (ioAttr == IOAttribute::IN_OUT) {
                        argParcel->ReadByte(*value);
                    }
                    argList->SetOutputArgumentOfByte(i, reinterpret_cast<HANDLE>(value));
                    break;
                }
                case TypeKind::Short: {
                    // `SMALL_PARAM_BUFFER`
                    //Short* value = new Short;
                    Short* value = (Short*)cArglist->GetOutParamBuffer(iOutParam++);
                    if (nullptr == value) {
                        Logger::E("InterfaceStub::UnmarshalArguments", "new Short() error");
                        return E_OUT_OF_MEMORY_ERROR;
                    }

                    if (ioAttr == IOAttribute::IN_OUT) {
                        argParcel->ReadShort(*value);
                    }
                    argList->SetOutputArgumentOfShort(i, reinterpret_cast<HANDLE>(value));
                    break;
                }
                case TypeKind::Integer: {
                    // `SMALL_PARAM_BUFFER`
                    //Integer* value = new Integer;
                    Integer* value = (Integer*)cArglist->GetOutParamBuffer(iOutParam++);
                    if (nullptr == value) {
                        Logger::E("InterfaceStub::UnmarshalArguments", "new Integer() error");
                        return E_OUT_OF_MEMORY_ERROR;
                    }

                    if (ioAttr == IOAttribute::IN_OUT) {
                        argParcel->ReadInteger(*value);
                    }
                    argList->SetOutputArgumentOfInteger(i, reinterpret_cast<HANDLE>(value));
                    break;
                }
                case TypeKind::Long: {
                    // `SMALL_PARAM_BUFFER`
                    //Long* value = new Long;
                    Long* value = cArglist->GetOutParamBuffer(iOutParam++);
                    if (nullptr == value) {
                        Logger::E("InterfaceStub::UnmarshalArguments", "new Long() error");
                        return E_OUT_OF_MEMORY_ERROR;
                    }

                    if (ioAttr == IOAttribute::IN_OUT) {
                        argParcel->ReadLong(*value);
                    }
                    argList->SetOutputArgumentOfLong(i, reinterpret_cast<HANDLE>(value));
                    break;
                }
                case TypeKind::Float: {
                    // `SMALL_PARAM_BUFFER`
                    //Float* value = new Float;
                    Float* value = (Float*)cArglist->GetOutParamBuffer(iOutParam++);
                    if (nullptr == value) {
                        Logger::E("InterfaceStub::UnmarshalArguments", "new Float() error");
                        return E_OUT_OF_MEMORY_ERROR;
                    }

                    if (ioAttr == IOAttribute::IN_OUT) {
                        argParcel->ReadFloat(*value);
                    }
                    argList->SetOutputArgumentOfFloat(i, reinterpret_cast<HANDLE>(value));
                    break;
                }
                case TypeKind::Double: {
                    // `SMALL_PARAM_BUFFER`
                    //Double* value = new Double;
                    Double* value = (Double*)cArglist->GetOutParamBuffer(iOutParam++);
                    if (nullptr == value) {
                        Logger::E("InterfaceStub::UnmarshalArguments", "new Double() error");
                        return E_OUT_OF_MEMORY_ERROR;
                    }

                    if (ioAttr == IOAttribute::IN_OUT) {
                        argParcel->ReadDouble(*value);
                    }
                    argList->SetOutputArgumentOfDouble(i, reinterpret_cast<HANDLE>(value));
                    break;
                }
                case TypeKind::Boolean: {
                    // `SMALL_PARAM_BUFFER`
                    //Boolean* value = new Boolean;
                    Boolean* value = (Boolean*)cArglist->GetOutParamBuffer(iOutParam++);
                    if (nullptr == value) {
                        Logger::E("InterfaceStub::UnmarshalArguments", "new Boolean() error");
                        return E_OUT_OF_MEMORY_ERROR;
                    }

                    if (ioAttr == IOAttribute::IN_OUT) {
                        argParcel->ReadBoolean(*value);
                    }
                    argList->SetOutputArgumentOfBoolean(i, reinterpret_cast<HANDLE>(value));
                    break;
                }
                case TypeKind::String: {
                    String* value = new String();
                    if (nullptr == value) {
                        Logger::E("InterfaceStub::UnmarshalArguments", "new String() error");
                        return E_OUT_OF_MEMORY_ERROR;
                    }

                    if (ioAttr == IOAttribute::IN_OUT) {
                        argParcel->ReadString(*value);
                    }
                    argList->SetOutputArgumentOfString(i, reinterpret_cast<HANDLE>(value));
                    break;
                }
                case TypeKind::ECode: {
                    // `SMALL_PARAM_BUFFER`
                    //ECode* value = new ECode;
                    ECode* value = (ECode*)cArglist->GetOutParamBuffer(iOutParam++);
                    if (nullptr == value) {
                        Logger::E("InterfaceStub::UnmarshalArguments", "new ECode() error");
                        return E_OUT_OF_MEMORY_ERROR;
                    }

                    if (ioAttr == IOAttribute::IN_OUT) {
                        argParcel->ReadECode(*value);
                    }
                    argList->SetOutputArgumentOfECode(i, reinterpret_cast<HANDLE>(value));
                    break;
                }
                case TypeKind::Enum: {
                    // `SMALL_PARAM_BUFFER`
                    //Integer* value = new Integer;
                    Integer* value = (Integer*)cArglist->GetOutParamBuffer(iOutParam++);
                    if (nullptr == value) {
                        Logger::E("InterfaceStub::UnmarshalArguments", "new Integer() error");
                        return E_OUT_OF_MEMORY_ERROR;
                    }

                    if (ioAttr == IOAttribute::IN_OUT) {
                        argParcel->ReadEnumeration(*value);
                    }
                    argList->SetOutputArgumentOfEnumeration(i, reinterpret_cast<HANDLE>(value));
                    break;
                }
                case TypeKind::Array: {
                    Triple* t = new Triple();
                    if (nullptr == t) {
                        Logger::E("InterfaceStub::UnmarshalArguments", "new Triple() error");
                        return E_OUT_OF_MEMORY_ERROR;
                    }

                    if (ioAttr == IOAttribute::IN_OUT) {
                        argParcel->ReadArray(t);
                    }
                    argList->SetOutputArgumentOfArray(i, reinterpret_cast<HANDLE>(t));
                    break;
                }
                case TypeKind::Interface: {
                    IInterface** intf = new IInterface*;
                    if (nullptr == intf) {
                        Logger::E("InterfaceStub::UnmarshalArguments", "new IInterface* error");
                        return E_OUT_OF_MEMORY_ERROR;
                    }

                    //@ `REFCOUNT`
                    // UnmarshalArguments, OUT/IN_OUT, parameter
                    if (ioAttr == IOAttribute::IN_OUT) {
                        argParcel->ReadInterface(*reinterpret_cast<AutoPtr<IInterface>*>(intf));
                    }
                    argList->SetOutputArgumentOfInterface(i, reinterpret_cast<HANDLE>(intf));
                    break;
                }
                case TypeKind::CoclassID:
                case TypeKind::ComponentID:
                case TypeKind::InterfaceID:
                case TypeKind::HANDLE:
                case TypeKind::Triple:
                default:
                    Logger::E("CStub", "Invalid [in, out] or [out] type(%d), param index: %d.\n", kind, i);
                    return E_ILLEGAL_ARGUMENT_EXCEPTION;
            }
        }
        else if (ioAttr == IOAttribute::OUT_CALLEE) {
            switch (kind) {
                case TypeKind::Array: {
                    Triple* t = new Triple();
                    if (nullptr == t) {
                        Logger::E("InterfaceStub::UnmarshalArguments", "new Triple() error");
                        return E_OUT_OF_MEMORY_ERROR;
                    }

                    argParcel->ReadArray(t);
                    argList->SetOutputArgumentOfArray(i, reinterpret_cast<HANDLE>(t));
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
                    Logger::E("CStub", "Invalid [out, callee] type(%d), param index: %d.\n", kind, i);
                    return E_ILLEGAL_ARGUMENT_EXCEPTION;
            }
        }
    }

    return NOERROR;
}

ECode InterfaceStub::MarshalResults(
    /* [in] */ IMetaMethod* method,
    /* [in] */ IArgumentList* argList,
    /* [out] */ AutoPtr<IParcel>& resParcel)
{
    HANDLE addr;

    CMetaMethod* cMethod = CMetaMethod::From(method);
    Integer N = cMethod->mParameters.GetLength();
    if (N > 0) {
        // Help me to call BuildAllParameters once.
        Integer outArgs;
        method->GetOutArgumentsNumber(outArgs);
    }
    else
        return NOERROR;

    for (Integer i = 0;  i < N;  i++) {
        IMetaParameter* param = cMethod->mParameters[i];

        AutoPtr<IMetaType> type;
        TypeKind kind;
        IOAttribute ioAttr;

        // just an assignment, needn't to check the return value
        param->GetIOAttribute(ioAttr);
        param->GetType(type);
        type->GetTypeKind(kind);

        if (ioAttr == IOAttribute::IN) {
            switch (kind) {
                case TypeKind::Char:
                case TypeKind::Byte:
                case TypeKind::Short:
                case TypeKind::Integer:
                case TypeKind::Long:
                case TypeKind::Float:
                case TypeKind::Double:
                case TypeKind::Boolean:
                case TypeKind::ECode:
                case TypeKind::Enum:
                    break;

                case TypeKind::String: {
                    argList->GetArgumentAddress(i, addr);
                    String* value = reinterpret_cast<String*>(addr);
                    delete value;
                    break;
                }
                case TypeKind::Array: {
                    argList->GetArgumentAddress(i, addr);
                    Triple* t = reinterpret_cast<Triple*>(addr);
                    t->FreeData();
                    delete t;
                    break;
                }
                case TypeKind::Interface: {
                    argList->GetArgumentAddress(i, addr);
                    IInterface* intf = reinterpret_cast<IInterface*>(addr);

                    //@ `REFCOUNT`
                    // MarshalResults, IN, parameter
                    REFCOUNT_RELEASE(intf);
                    break;
                }
                case TypeKind::CoclassID:
                case TypeKind::ComponentID:
                case TypeKind::InterfaceID:
                case TypeKind::HANDLE:
                case TypeKind::Triple:
                default:
                    Logger::E("InterfaceStub::MarshalResults",
                          "Invalid [in] type(%d), param index: %d.\n", kind, i);
                    return E_ILLEGAL_ARGUMENT_EXCEPTION;
            }
        }
        else if (ioAttr == IOAttribute::OUT || ioAttr == IOAttribute::IN_OUT) {
            switch (kind) {
                case TypeKind::Char: {
                    argList->GetArgumentAddress(i, addr);
                    resParcel->WriteChar(*(reinterpret_cast<Char*>(addr)));
                    break;
                }
                case TypeKind::Byte: {
                    argList->GetArgumentAddress(i, addr);
                    resParcel->WriteByte(*(reinterpret_cast<Byte*>(addr)));
                    break;
                }
                case TypeKind::Short: {
                    argList->GetArgumentAddress(i, addr);
                    resParcel->WriteShort(*(reinterpret_cast<Short*>(addr)));
                    break;
                }
                case TypeKind::Integer: {
                    argList->GetArgumentAddress(i, addr);
                    resParcel->WriteInteger(*(reinterpret_cast<Integer*>(addr)));
                    break;
                }
                case TypeKind::Long: {
                    argList->GetArgumentAddress(i, addr);
                    resParcel->WriteLong(*(reinterpret_cast<Long*>(addr)));
                    break;
                }
                case TypeKind::Float: {
                    argList->GetArgumentAddress(i, addr);
                    resParcel->WriteFloat(*(reinterpret_cast<Float*>(addr)));
                    break;
                }
                case TypeKind::Double: {
                    argList->GetArgumentAddress(i, addr);
                    resParcel->WriteDouble(*(reinterpret_cast<Double*>(addr)));
                    break;
                }
                case TypeKind::Boolean: {
                    argList->GetArgumentAddress(i, addr);
                    resParcel->WriteBoolean(*(reinterpret_cast<Boolean*>(addr)));
                    break;
                }
                case TypeKind::String: {
                    argList->GetArgumentAddress(i, addr);
                    String* value = reinterpret_cast<String*>(addr);
                    resParcel->WriteString(*value);
                    delete value;
                    break;
                }
                case TypeKind::ECode: {
                    argList->GetArgumentAddress(i, addr);
                    resParcel->WriteECode(*(reinterpret_cast<ECode*>(addr)));
                    break;
                }
                case TypeKind::Enum: {
                    argList->GetArgumentAddress(i, addr);
                    resParcel->WriteEnumeration(*(reinterpret_cast<Integer*>(addr)));
                    break;
                }
                case TypeKind::Array: {
                    argList->GetArgumentAddress(i, addr);
                    Triple* t = reinterpret_cast<Triple*>(addr);
                    resParcel->WriteArray(*t);
                    delete t;
                    break;
                }
                case TypeKind::Interface: {
                    argList->GetArgumentAddress(i, addr);
                    IInterface** intf = reinterpret_cast<IInterface**>(addr);
                    resParcel->WriteInterface(*intf);

                    //@ `REFCOUNT`
                    // MarshalResults, OUT/IN_OUT, parameter
                    REFCOUNT_ADD(*intf);
                    delete intf;
                    break;
                }
                case TypeKind::CoclassID:
                case TypeKind::ComponentID:
                case TypeKind::InterfaceID:
                case TypeKind::HANDLE:
                case TypeKind::Triple:
                default:
                    Logger::E("InterfaceStub::MarshalResults",
                              "Invalid [in, out] or [out] type(%d), param index: %d.\n",
                              kind, i);
                    return E_ILLEGAL_ARGUMENT_EXCEPTION;
            }
        }
        else if (ioAttr == IOAttribute::OUT_CALLEE) {
            switch (kind) {
                case TypeKind::Array: {
                    argList->GetArgumentAddress(i, addr);
                    Triple* t = reinterpret_cast<Triple*>(addr);
                    resParcel->WriteArray(*t);
                    delete t;
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
                    Logger::E("InterfaceStub::MarshalResults",
                              "Invalid [out, callee] type(%d), param index: %d.\n",
                              kind, i);
                    return E_ILLEGAL_ARGUMENT_EXCEPTION;
            }
        }
    }

    return NOERROR;
}

ECode InterfaceStub::Invoke(
    /* [in] */ IParcel* argParcel,
    /* [out] */ AutoPtr<IParcel>& resParcel)
{
    Integer methodIndex, methodNumber;
    argParcel->ReadInteger(methodIndex);
    mTargetMetadata->GetMethodNumber(methodNumber);
    if ((methodIndex < 0) || (methodIndex >= methodNumber)) {
        Logger::E("CStub", "MethodIndex %d is invalid.", methodIndex);
        return E_RUNTIME_EXCEPTION;
    }

    AutoPtr<IMetaMethod> mm;
    ECode ec = mTargetMetadata->GetMethod(methodIndex, mm);
    if (FAILED(ec)) {
        Logger::E("CStub", "Invoke GetMethod failed with ec is 0x%X.", ec);
        return ec;
    }

    Long uuid64;
    argParcel->ReadLong(uuid64);

    if (Logger::GetLevel() <= Logger::DEBUG) {
        String strBuffer1;
        String strBuffer2;
        reflHelpers::mmGetMethodInfo(mm, strBuffer1);
        reflHelpers::intfGetObjectInfo(mObject, strBuffer2);
        Logger::D("CStub", "Object of %s Invoke Method: %s",
                                      strBuffer2.string(), strBuffer1.string());
    }

    AutoPtr<IArgumentList> argList;
    ec = UnmarshalArguments(mm, argParcel, argList);
    if (FAILED(ec)) {
        Logger::E("CStub", "Invoke UnmarshalArguments failed with ec is 0x%X.", ec);
        return ec;
    }

    CMetaMethod::From(mm)->mHotCode = 1;

    // RuntimeMonitor
    if (mOwner->mMonitorInvokeMethod) {
        Long serverObjectId;
        mOwner->mChannel->GetServerObjectId(serverObjectId);
        ec = RuntimeMonitor::WriteRtmInvokeMethod(uuid64, serverObjectId,
                              mOwner->mCid, mIid,
                              RTM_ParamTransDirection::CALL_METHOD,
                              methodIndex, argParcel,
                              RTM_WhichQueue::InvokeMethodServerQueue);
    }

    ECode ret = mm->Invoke(mObject, argList);
    if (SUCCEEDED(ret)) {
        resParcel->WriteInteger(RPC_MAGIC_NUMBER);
        resParcel->WriteLong(uuid64);

        ec = MarshalResults(mm, argList, resParcel);
        if (FAILED(ec)) {
            Logger::E("CStub", "MarshalResults failed with ec is 0x%X.", ec);
            argList = nullptr;
            resParcel = nullptr;
            return ec;
        }

        // RuntimeMonitor
        if (mOwner->mMonitorInvokeMethod) {
            Long serverObjectId;
            mOwner->mChannel->GetServerObjectId(serverObjectId);

            ec = RuntimeMonitor::WriteRtmInvokeMethod(uuid64, serverObjectId,
                                  mOwner->mCid, mIid,
                                  RTM_ParamTransDirection::RETURN_FROM_METHOD,
                                  methodIndex, resParcel,
                                  RTM_WhichQueue::InvokeMethodServerQueue);
        }
    }
    else {
        argList = nullptr;
        resParcel = nullptr;
    }

    return ret;
}

//----------------------------------------------------------------------

const CoclassID CID_CStub =
        {{0x52068014,0xe347,0x453f,0x87a9,{0x0b,0xec,0xfb,0x69,0xd8,0xed}}, &CID_COMORuntime};

COMO_OBJECT_IMPL(CStub);

Integer CStub::AddRef(
    /* [in] */ HANDLE id)
{
    return Object::AddRef(id);
}

Integer CStub::Release(
    /* [in] */ HANDLE id)
{
    return Object::Release(id);
}

IInterface* CStub::Probe(
    /* [in] */ const InterfaceID& iid)
{
    if (iid == IID_IInterface) {
        return (IInterface*)(IStub*)this;
    }
    else if (iid == IID_IStub) {
        return (IStub*)this;
    }
    return Object::Probe(iid);
}

ECode CStub::GetInterfaceID(
    /* [in] */ IInterface* object,
    /* [out] */ InterfaceID& iid)
{
    if (object == (IInterface*)(IStub*)this) {
        iid = IID_IStub;
        return NOERROR;
    }
    return Object::GetInterfaceID(object, iid);
}

void CStub::OnLastStrongRef(
    /* [in] */ const void* id)
{
    Object::OnLastStrongRef(id);
}

ECode CStub::Match(
    /* [in] */ IInterfacePack* ipack,
    /* [out] */ Boolean& matched)
{
    return mChannel->Match(ipack, matched);
}

ECode CStub::Invoke(
    /* [in] */ IParcel* argParcel,
    /* [out] */ AutoPtr<IParcel>& resParcel)
{
    Integer magic;
    argParcel->ReadInteger(magic);
    if (magic != RPC_MAGIC_NUMBER) {
        Logger::E("CStub", "Magic number 0x%X is invalid.", magic);
        return E_RUNTIME_EXCEPTION;
    }

    Integer interfaceIndex;
    argParcel->ReadInteger(interfaceIndex);
    if (interfaceIndex < 0 || interfaceIndex >= mInterfaces.GetLength()) {
        Logger::E("CStub", "InterfaceIndex %d is invalid.", interfaceIndex);
        return E_RUNTIME_EXCEPTION;
    }

    return mInterfaces[interfaceIndex]->Invoke(argParcel, resParcel);
}

AutoPtr<IObject> CStub::GetTarget()
{
    return mTarget;
}

AutoPtr<IRPCChannel> CStub::GetChannel()
{
    return mChannel;
}

CoclassID CStub::GetTargetCoclassID()
{
    return mCid;
}

ECode CStub::CreateObject(
    /* [in] */ IInterface* object,
    /* [in] */ IRPCChannel* channel,
    /* [out] */ AutoPtr<IStub>& stub)
{
    stub = nullptr;

    IObject* obj = IObject::Probe(object);
    if (obj == nullptr) {
        Logger::E("CStub", "Object does not have \"IObject\" interface.");
        return E_INTERFACE_NOT_FOUND_EXCEPTION;
    }

    // refer to `ServerObjectId`
    Long hash;
    obj->GetHashCode(hash);
    channel->SetServerObjectId(hash);

    AutoPtr<IMetaCoclass> mc;
    obj->GetCoclass(mc);
    if (mc == nullptr) {
        Logger::E("CStub", "Fail to get object's Coclass.");
        return E_NOT_FOUND_EXCEPTION;
    }

    CoclassID cid;
    mc->GetCoclassID(cid);
    Logger::D("CStub", "Object's CoclassID is %s", DumpUUID(cid.mUuid).string());

    AutoPtr<CStub> stubObj = new CStub();
    if (nullptr == stubObj) {
        return E_OUT_OF_MEMORY_ERROR;
    }

    stubObj->mTarget = obj;
    stubObj->mCid = cid;
    stubObj->mTargetMetadata = mc;
    stubObj->mChannel = channel;

    Long lvalue;
    ECode ec = reflHelpers::coclassGetConstantLong(mc, String("monitor"), lvalue);
    if (FAILED(ec)) {
        // If it is an error such as the definition cannot be found, it is normal
        if (E_TYPE_MISMATCH_EXCEPTION == ec) {
            Logger::E("CStub", "Wrong monitor datatype");
        }
        stubObj->mMonitorInvokeMethod = false;
    }
    else {
        stubObj->mMonitorInvokeMethod = true;
    }

    Integer interfaceNumber;
    mc->GetInterfaceNumber(interfaceNumber);
    Array<IMetaInterface*> interfaces(interfaceNumber);
    if (interfaces.IsNull()) {
        Logger::E("CStub", "GetAllInterfaces failed, Array is NULL");
        return E_OUT_OF_MEMORY_ERROR;
    }

    ec = mc->GetAllInterfaces(interfaces);
    if (FAILED(ec)) {
        Logger::E("CStub", "GetAllInterfaces failed with ec is 0x%X", ec);
        return ec;
    }

    stubObj->mInterfaces = Array<InterfaceStub*>(interfaceNumber);
    if (stubObj->mInterfaces.IsNull()) {
        Logger::E("CStub", "Array<InterfaceStub*> is NULL");
        return E_OUT_OF_MEMORY_ERROR;
    }

    for (Integer i = 0;  i < interfaceNumber;  i++) {
        AutoPtr<InterfaceStub> istub = new InterfaceStub();
        if (nullptr == istub) {
            // rollback this transaction
            for (Integer j = 0;  j < i;  j++)
                delete stubObj->mInterfaces[j];
            delete stubObj;

            return E_OUT_OF_MEMORY_ERROR;
        }

        istub->mOwner = stubObj;
        istub->mTargetMetadata = interfaces[i];
        istub->mTargetMetadata->GetInterfaceID(istub->mIid);
        istub->mObject = object->Probe(istub->mIid);
        if (nullptr == istub->mObject) {
            String name, ns;
            interfaces[i]->GetNamespace(ns);
            interfaces[i]->GetName(name);
            Logger::E("CStub", "Object does not have \"%s::%s\" interface.",
                                                    ns.string(), name.string());
            // rollback this transaction
            for (Integer j = 0;  j < i;  j++) {
                delete stubObj->mInterfaces[j];
            }
            delete stubObj;

            return E_INTERFACE_NOT_FOUND_EXCEPTION;
        }
        stubObj->mInterfaces[i] = istub;

        {
            String name, ns, uuidStr;
            interfaces[i]->GetNamespace(ns);
            interfaces[i]->GetName(name);
            uuidStr = DumpUUID(istub->mIid.mUuid);
            Logger::D("CStub", "Object has \"%s::%s\" [%s] interface.",
                                    ns.string(), name.string(), uuidStr.string());
        }
    }

    ec = channel->StartListening(stubObj);
    if (FAILED(ec)) {
        Logger::E("CStub", "Channel start listening failed with ec is 0x%X", ec);
        return ec;
    }

    stub = stubObj;
    return NOERROR;
}

} // namespace como
