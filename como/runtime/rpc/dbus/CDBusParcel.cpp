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

/*
 * Copyright (C) 2005 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "rpc/comorpc.h"
#include "rpc/dbus/CDBusParcel.h"
#include "util/comosp.h"
#include "util/comolog.h"
#include <stdlib.h>
#include "ComoConfig.h"

namespace como {

const CoclassID CID_CDBusParcel =
        {{0x28208469,0x8814,0x49af,0x80f7,{0x8b,0x14,0x73,0x91,0xfb,0x94}}, &CID_COMORuntime};

COMO_INTERFACE_IMPL_1(CDBusParcel, Object, IParcel);

COMO_OBJECT_IMPL(CDBusParcel);

CDBusParcel::CDBusParcel()
    : mError(NOERROR)
    , mData(mBuffer)        // mBuffer points to the space within the object
    , mDataSize(0)
    , mDataCapacity(MAX_BUFFER_SIZE)
    , mDataPos(0)
    , mProxyId(0)
{}

CDBusParcel::~CDBusParcel()
{
    if (mData != mBuffer) {
        free(mData);
    }
}

CMemPool *CDBusParcel::memPool = new CMemPool(nullptr, ComoConfig::POOL_SIZE_Parcel,
                                              sizeof(CDBusParcel));
void *CDBusParcel::MemPoolAlloc(size_t ulSize)
{
    return memPool->Alloc(ulSize, MUST_USE_MEM_POOL);
}

void CDBusParcel::MemPoolFree(Short id, const void *p)
{
    memPool->Free((void *)p);
}

ECode CDBusParcel::ReadChar(
    /* [out] */ Char& value)
{
    Integer i;
    ECode ec = ReadInteger(i);
    value = (Char)i;
    return ec;
}

ECode CDBusParcel::WriteChar(
    /* [in] */ Char value)
{
    return WriteInteger((Integer)value);
}

ECode CDBusParcel::ReadByte(
    /* [out] */ Byte& value)
{
    Integer i;
    ECode ec = ReadInteger(i);
    value = (Byte)i;
    return ec;
}

ECode CDBusParcel::WriteByte(
    /* [in] */ Byte value)
{
    return WriteInteger((Integer)value);
}

ECode CDBusParcel::ReadShort(
    /* [out] */ Short& value)
{
    Integer i;
    ECode ec = ReadInteger(i);
    value = (Short)i;
    return ec;
}

ECode CDBusParcel::WriteShort(
    /* [in] */ Short value)
{
    return WriteInteger((Integer)value);
}

ECode CDBusParcel::ReadInteger(
    /* [out] */ Integer& value)
{
    return ReadAligned<Integer>(&value);
}

ECode CDBusParcel::WriteInteger(
    /* [in] */ Integer value)
{
    return WriteAligned<Integer>(value);
}

ECode CDBusParcel::ReadLong(
    /* [out] */ Long& value)
{
    return ReadAligned<Long>(&value);
}

ECode CDBusParcel::WriteLong(
    /* [in] */ Long value)
{
    return WriteAligned<Long>(value);
}

ECode CDBusParcel::ReadFloat(
    /* [out] */ Float& value)
{
    return ReadAligned<Float>(&value);
}

ECode CDBusParcel::WriteFloat(
    /* [in] */ Float value)
{
    return WriteAligned<Float>(value);
}

ECode CDBusParcel::ReadDouble(
    /* [out] */ Double& value)
{
    return ReadAligned<Double>(&value);
}

ECode CDBusParcel::WriteDouble(
    /* [in] */ Double value)
{
    return WriteAligned<Double>(value);
}

ECode CDBusParcel::ReadBoolean(
    /* [out] */ Boolean& value)
{
    Integer i;
    ECode ec = ReadInteger(i);
    value = (Boolean)i;
    return ec;
}

ECode CDBusParcel::WriteBoolean(
    /* [in] */ Boolean value)
{
    return WriteInteger((Integer)value);
}

ECode CDBusParcel::ReadString(
    /* [out] */ String& value)
{
    value = nullptr;

    Integer size;
    ECode ec = ReadInteger(size);
    if (FAILED(ec)) {
        return ec;
    }

    if (size < 0) {
        value = nullptr;
        return NOERROR;
    }

    if (size == 0) {
        value = "";
        return NOERROR;
    }

    const char* str = (const char*)ReadInplace(size + 1);
    if (nullptr == str) {
        return E_RUNTIME_EXCEPTION;
    }

    value = str;
    return NOERROR;
}

ECode CDBusParcel::WriteString(
    /* [in] */ const String& value)
{
    if (value.IsNull())
        return WriteInteger(-1);

    ECode ec = WriteInteger(value.GetByteLength());
    if (value.GetByteLength() > 0 && SUCCEEDED(ec)) {
        ec = Write(value.string(), value.GetByteLength() + 1);
    }
    return ec;
}

ECode CDBusParcel::ReadCoclassID(
    /* [out] */ CoclassID& value)
{
    ECode ec = Read((void*)&value, sizeof(CoclassID));
    if (FAILED(ec)) {
        return ec;
    }

    Integer tag;
    ReadInteger(tag);
    if (tag == TAG_NULL) {
        return NOERROR;
    }

    ComponentID* cid = (ComponentID*)malloc(sizeof(ComponentID));
    if (nullptr == cid) {
        return E_OUT_OF_MEMORY_ERROR;
    }
    value.mCid = cid;
    return ReadComponentID(*cid);
}

ECode CDBusParcel::WriteCoclassID(
    /* [in] */ const CoclassID& value)
{
    CoclassID* cid = (CoclassID*)WriteInplace(sizeof(CoclassID));
    if (nullptr == cid) {
        return E_RUNTIME_EXCEPTION;
    }
    memcpy(cid, &value, sizeof(CoclassID));
    cid->mCid = nullptr;

    if (nullptr == value.mCid) {
        return WriteInteger(TAG_NULL);
    }

    ECode ec = WriteInteger(TAG_NOT_NULL);
    if (SUCCEEDED(ec)) {
        ec = WriteComponentID(*value.mCid);
    }
    return ec;
}

ECode CDBusParcel::ReadComponentID(
    /* [out] */ ComponentID& value)
{
    ECode ec = Read((void*)&value, sizeof(ComponentID));
    if (FAILED(ec)) {
        return ec;
    }

    Integer size;
    ReadInteger(size);
    if (size == 0) {
        return NOERROR;
    }

    if (size < 0) {
        return E_RUNTIME_EXCEPTION;
    }

    const char* uri = (const char*)ReadInplace(size + 1);
    if (nullptr == uri)
        return E_OUT_OF_MEMORY_ERROR;

    value.mUri = (const char*)malloc(size + 1);
    if (nullptr != value.mUri) {
        memcpy(const_cast<char*>(value.mUri), uri, size + 1);
    }
    return NOERROR;
}

ECode CDBusParcel::WriteComponentID(
    /* [in] */ const ComponentID& value)
{
    ComponentID* cid = (ComponentID*)WriteInplace(sizeof(ComponentID));
    if (nullptr == cid) {
        return E_RUNTIME_EXCEPTION;
    }
    memcpy(cid, &value, sizeof(ComponentID));
    cid->mUri = nullptr;

    Integer size = ((nullptr == value.mUri) ? 0 : strlen(value.mUri));
    ECode ec = WriteInteger(size);
    if ((size > 0) && SUCCEEDED(ec)) {
        ec = Write(value.mUri, size + 1);
    }
    return ec;
}

ECode CDBusParcel::ReadInterfaceID(
    /* [out] */ InterfaceID& value)
{
    ECode ec = Read((void*)&value, sizeof(InterfaceID));
    if (FAILED(ec)) {
        return ec;
    }

    Integer tag;
    ReadInteger(tag);
    if (tag == TAG_NULL) {
        return NOERROR;
    }

    ComponentID *cid = (ComponentID*)malloc(sizeof(ComponentID));
    if (nullptr == cid) {
        return E_OUT_OF_MEMORY_ERROR;
    }
    value.mCid = cid;
    return ReadComponentID(*cid);
}

ECode CDBusParcel::WriteInterfaceID(
    /* [in] */ const InterfaceID& value)
{
    InterfaceID *iid = (InterfaceID*)WriteInplace(sizeof(InterfaceID));
    if (nullptr == iid) {
        return E_RUNTIME_EXCEPTION;
    }
    memcpy(iid, &value, sizeof(InterfaceID));
    iid->mCid = nullptr;

    if (nullptr == value.mCid) {
        return WriteInteger(TAG_NULL);
    }

    ECode ec = WriteInteger(TAG_NOT_NULL);
    if (SUCCEEDED(ec)) {
        ec = WriteComponentID(*value.mCid);
    }
    return ec;
}

ECode CDBusParcel::ReadECode(
    /* [out] */ ECode& value)
{
    return ReadInteger(value);
}

ECode CDBusParcel::WriteECode(
    /* [in] */ ECode value)
{
    return WriteInteger(value);
}

ECode CDBusParcel::ReadEnumeration(
    /* [out] */ Integer& value)
{
    return ReadInteger(value);
}

ECode CDBusParcel::WriteEnumeration(
    /* [in] */ Integer value)
{
    return WriteInteger(value);
}

ECode CDBusParcel::ReadArray(
    /* [out] */ Triple* array)
{
    Triple* t = array;
    VALIDATE_NOT_NULL(t);

    Integer value;
    ECode ec = ReadInteger(value);
    if (FAILED(ec)) {
        return ec;
    }

    TypeKind kind = (TypeKind)value;
    Long size;
    ec = ReadLong(size);
    if ((size <= 0) || FAILED(ec)) {
        t->mData = nullptr;
        t->mSize = 0;
        t->mType = kind;
        return FAILED(ec) ? ec
                          : ((size < 0) ? E_RUNTIME_EXCEPTION
                                        : NOERROR);
    }

    switch (kind) {
        case TypeKind::Char: {
            t->mSize = size;
            t->mType = TypeKind::Char;
            t->AllocData(sizeof(Char) * size);
            if (t->mData != nullptr) {
                ec = Read(t->mData, sizeof(Char) * size);
            }
            else {
                ec = E_OUT_OF_MEMORY_ERROR;
            }
            break;
        }
        case TypeKind::Byte: {
            t->mSize = size;
            t->mType = TypeKind::Byte;
            t->AllocData(sizeof(Byte) * size);
            if (t->mData != nullptr) {
                ec = Read(t->mData, sizeof(Byte) * size);
            }
            else {
                ec = E_OUT_OF_MEMORY_ERROR;
            }
            break;
        }
        case TypeKind::Short: {
            t->mSize = size;
            t->mType = TypeKind::Short;
            t->AllocData(sizeof(Short) * size);
            if (t->mData != nullptr) {
                ec = Read(t->mData, sizeof(Short) * size);
            }
            else {
                ec = E_OUT_OF_MEMORY_ERROR;
            }
            break;
        }
        case TypeKind::Integer: {
            t->mSize = size;
            t->mType = TypeKind::Integer;
            t->AllocData(sizeof(Integer) * size);
            if (t->mData != nullptr) {
                ec = Read(t->mData, sizeof(Integer) * size);
            }
            else {
                ec = E_OUT_OF_MEMORY_ERROR;
            }
            break;
        }
        case TypeKind::Long: {
            t->mSize = size;
            t->mType = TypeKind::Long;
            t->AllocData(sizeof(Long) * size);
            if (t->mData != nullptr) {
                ec = Read(t->mData, sizeof(Long) * size);
            }
            else {
                ec = E_OUT_OF_MEMORY_ERROR;
            }
            break;
        }
        case TypeKind::Float: {
            t->mSize = size;
            t->mType = TypeKind::Float;
            t->AllocData(sizeof(Float) * size);
            if (t->mData != nullptr) {
                ec = Read(t->mData, sizeof(Float) * size);
            }
            else {
                ec = E_OUT_OF_MEMORY_ERROR;
            }
            break;
        }
        case TypeKind::Double: {
            t->mSize = size;
            t->mType = TypeKind::Double;
            t->AllocData(sizeof(Double) * size);
            if (t->mData != nullptr) {
                ec = Read(t->mData, sizeof(Double) * size);
            }
            else {
                ec = E_OUT_OF_MEMORY_ERROR;
            }
            break;
        }
        case TypeKind::Boolean: {
            t->mSize = size;
            t->mType = TypeKind::Boolean;
            t->AllocData(sizeof(Boolean) * size);
            if (t->mData != nullptr) {
                ec = Read(t->mData, sizeof(Boolean) * size);
            }
            else {
                ec = E_OUT_OF_MEMORY_ERROR;
            }
            break;
        }
        case TypeKind::String: {
            Array<String> strArray(size);
            for (Long i = 0; i < size; i++) {
                String str;
                ec = ReadString(str);
                if (FAILED(ec)) {
                    t->mData = nullptr;
                    t->mSize = 0;
                    t->mType = kind;
                    return ec;
                }
                strArray[i] = str;
            }
            *t = strArray;
            break;
        }
        case TypeKind::CoclassID: {
            Array<CoclassID> cidArray(size);
            for (Long i = 0; i < size; i++) {
                CoclassID& cid = cidArray[i];
                ec = ReadCoclassID(cid);
                if (FAILED(ec)) {
                    t->mData = nullptr;
                    t->mSize = 0;
                    t->mType = kind;
                    return ec;
                }
            }
            *t = cidArray;
            break;
        }
        case TypeKind::ComponentID: {
            Array<ComponentID> cidArray(size);
            for (Long i = 0; i < size; i++) {
                ComponentID& cid = cidArray[i];
                ec = ReadComponentID(cid);
                if (FAILED(ec)) {
                    t->mData = nullptr;
                    t->mSize = 0;
                    t->mType = kind;
                    return ec;
                }
            }
            *t = cidArray;
            break;
        }
        case TypeKind::InterfaceID: {
            Array<InterfaceID> iidArray(size);
            for (Long i = 0; i < size; i++) {
                InterfaceID& iid = iidArray[i];
                ec = ReadInterfaceID(iid);
                if (FAILED(ec)) {
                    t->mData = nullptr;
                    t->mSize = 0;
                    t->mType = kind;
                    return ec;
                }
            }
            *t = iidArray;
            break;
        }
        case TypeKind::ECode: {
            t->mSize = size;
            t->mType = TypeKind::ECode;
            t->AllocData(sizeof(ECode) * size);
            if (t->mData != nullptr) {
                ec = Read(t->mData, sizeof(ECode) * size);
            }
            else {
                ec = E_OUT_OF_MEMORY_ERROR;
            }
            break;
        }
        case TypeKind::Enum: {
            t->mSize = size;
            t->mType = TypeKind::Enum;
            t->AllocData(sizeof(Integer) * size);
            if (t->mData != nullptr) {
                ec = Read(t->mData, sizeof(Integer) * size);
            }
            else {
                ec = E_OUT_OF_MEMORY_ERROR;
            }
            break;
        }
        case TypeKind::Array: {
            Array<Triple> triArray(size);
            for (Long i = 0;  i < size;  i++) {
                ec = ReadArray(&(triArray[i]));
                if (FAILED(ec)) {
                    t->mData = nullptr;
                    t->mSize = 0;
                    t->mType = kind;
                    return ec;
                }
            }
            t->mType = TypeKind::Array;
            *t = triArray;
            break;
        }
        case TypeKind::Interface: {
            Array<IInterface*> intfArray(size);
            for (Long i = 0;  i < size;  i++) {
                AutoPtr<IInterface> obj;
                ec = ReadInterface(obj);
                if (FAILED(ec)) {
                    t->mData = nullptr;
                    t->mSize = 0;
                    t->mType = kind;
                    return ec;
                }
                intfArray.Set(i, obj);
            }
            *t = intfArray;
            break;
        }
        default:
            Logger::E("CDBusParcel", "Cannot read array with %d type from parcel", t->mType);
            break;
    }

    return ec;
}

ECode CDBusParcel::WriteArray(
    /* [in] */ const Triple& array)
{
    const Triple* t = &array;
    VALIDATE_NOT_NULL(t);

    ECode ec = WriteInteger((Integer)t->mType);
    if (FAILED(ec)) {
        return ec;
    }

    ec = WriteLong(t->mSize);
    if ((0 == t->mSize) || FAILED(ec)) {
        return ec;
    }

    switch (t->mType) {
        case TypeKind::Char:
            ec = Write(t->mData, sizeof(Char) * t->mSize);
            break;
        case TypeKind::Byte:
            ec = Write(t->mData, sizeof(Byte) * t->mSize);
            break;
        case TypeKind::Short:
            ec = Write(t->mData, sizeof(Short) * t->mSize);
            break;
        case TypeKind::Integer:
            ec = Write(t->mData, sizeof(Integer) * t->mSize);
            break;
        case TypeKind::Long:
            ec = Write(t->mData, sizeof(Long) * t->mSize);
            break;
        case TypeKind::Float:
            ec = Write(t->mData, sizeof(Float) * t->mSize);
            break;
        case TypeKind::Double:
            ec = Write(t->mData, sizeof(Double) * t->mSize);
            break;
        case TypeKind::Boolean:
            ec = Write(t->mData, sizeof(Boolean) * t->mSize);
            break;
        case TypeKind::String: {
            for (Long i = 0;  i < t->mSize;  i++) {
                const String& str = reinterpret_cast<String*>(t->mData)[i];
                ec = WriteString(str);
                if (FAILED(ec)) {
                    break;
                }
            }
            break;
        }
        case TypeKind::CoclassID: {
            for (Long i = 0;  i < t->mSize;  i++) {
                const CoclassID& cid = reinterpret_cast<CoclassID*>(t->mData)[i];
                ec = WriteCoclassID(cid);
                if (FAILED(ec)) {
                    break;
                }
            }
            break;
        }
        case TypeKind::ComponentID: {
            for (Long i = 0; i < t->mSize; i++) {
                const ComponentID& cid = reinterpret_cast<ComponentID*>(t->mData)[i];
                ec = WriteComponentID(cid);
                if (FAILED(ec)) {
                    break;
                }
            }
            break;
        }
        case TypeKind::InterfaceID: {
            for (Long i = 0; i < t->mSize; i++) {
                const InterfaceID& iid = reinterpret_cast<InterfaceID*>(t->mData)[i];
                ec = WriteInterfaceID(iid);
                if (FAILED(ec)) {
                    break;
                }
            }
            break;
        }
        case TypeKind::ECode:
            ec = Write(t->mData, sizeof(ECode) * t->mSize);
            break;
        case TypeKind::Enum:
            ec = Write(t->mData, sizeof(Integer) * t->mSize);
            break;
        case TypeKind::Array: {
            for (Long i = 0; i < t->mSize; i++) {
                const Triple& tt = reinterpret_cast<Triple*>(t->mData)[i];
                ec = WriteArray(tt);
                if (FAILED(ec)) {
                    break;
                }
            }
            break;
        }
        case TypeKind::Interface: {
            for (Long i = 0; i < t->mSize; i++) {
                IInterface* intf = reinterpret_cast<IInterface**>(t->mData)[i];
                ec = WriteInterface(intf);
                if (FAILED(ec)) {
                    break;
                }
            }
            break;
        }
        default:
            Logger::E("CDBusParcel", "Cannot write array with %d type into parcel", t->mType);
            break;
    }

    return ec;
}

ECode CDBusParcel::ReadInterface(
    /* [out] */ AutoPtr<IInterface>& value)
{
    Integer tag;
    ReadInteger(tag);
    if (tag == TAG_NOT_NULL) {
        AutoPtr<IInterfacePack> ipack;
        ECode ec = CoCreateInterfacePack(RPCType::Local, ipack);
        if (FAILED(ec)) {
            Logger::E("CDBusParcel::ReadInterface",
                                               "CoCreateInterfacePack failed.");
            return ec;
        }

        ec = IParcelable::Probe(ipack)->ReadFromParcel(this);
        if (FAILED(ec)) {
            Logger::E("CDBusParcel::ReadInterface", "ReadFromParcel failed.");
            return ec;
        }
        ipack->SetServerName(mServerName);
        ipack->SetProxyInfo(mProxyId);

        ec = CoUnmarshalInterface(ipack, RPCType::Local, value);
        if (FAILED(ec)) {
            Logger::E("CDBusParcel::ReadInterface",
              "Unmarshal the interface in ReadInterface failed. ECode: 0x%X", ec);
            return ec;
        }

        Boolean parcelable;
        ipack->IsParcelable(parcelable);
        if (parcelable) {
            IParcelable::Probe(value)->ReadFromParcel(this);
        }
    }
    else {
        value = nullptr;
    }
    return NOERROR;
}

ECode CDBusParcel::WriteInterface(
    /* [in] */ IInterface* value)
{
    WriteInteger(value != nullptr ? TAG_NOT_NULL : TAG_NULL);

    if (value != nullptr) {
        AutoPtr<IInterfacePack> ipack;
        ECode ec = CoMarshalInterface(value, RPCType::Local, ipack);
        if (FAILED(ec)) {
            Logger::E("CDBusParcel::WriteInterface",
                             "Marshal the interface in WriteInterface failed.");
            return ec;
        }

        ec = IParcelable::Probe(ipack)->WriteToParcel(this);
        if (FAILED(ec)) {
            Logger::E("CDBusParcel::WriteInterface", "WriteToParcel failed.");
            return ec;
        }

        IParcelable* p = IParcelable::Probe(value);
        if (p != nullptr) {
            p->WriteToParcel(this);
        }
    }

    return NOERROR;
}

ECode CDBusParcel::GetData(
    /* [out] */ HANDLE& data)
{
    data = reinterpret_cast<HANDLE>(mData);
    return NOERROR;
}

ECode CDBusParcel::SetData(
    /* [in] */ HANDLE data,
    /* [in] */ Long size)
{
    if (size <= 0) {
        return NOERROR;
    }

    ECode ec = RestartWrite(size);
    if (SUCCEEDED(ec)) {
        memcpy(mData, reinterpret_cast<Byte*>(data), size);
        mDataSize = size;
    }
    return ec;
}

ECode CDBusParcel::GetDataSize(
    /* [out] */ Long& size)
{
    size = mDataSize;
    return NOERROR;
}

ECode CDBusParcel::GetDataPosition(
    /* [out] */ Long& pos)
{
    pos = mDataPos;
    return NOERROR;
}

ECode CDBusParcel::SetDataPosition(
    /* [in] */ Long pos)
{
    if (pos < 0) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    mDataPos = pos;
    return NOERROR;
}

ECode CDBusParcel::GetPayload(
    /* [out] */ HANDLE& payload)
{
    payload = reinterpret_cast<HANDLE>(mData);
    return NOERROR;
}

ECode CDBusParcel::SetPayload(
    /* [in] */ HANDLE payload,
    /* [in] */ Boolean release)
{
    (void)release;

    if (payload == 0) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    if ((mData != nullptr) && (mData != mBuffer)) {
        free(mData);
    }

    mData = reinterpret_cast<Byte*>(payload);
    return NOERROR;
}

ECode CDBusParcel::Read(
    /* [in] */ void* outData,
    /* [in] */ Long len) const
{
    if (len < 0) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    if ((mDataPos + ALIGN4(len)) >= mDataPos &&
              ((mDataPos + ALIGN4(len)) <= mDataSize) && (len <= ALIGN4(len))) {
        memcpy(outData, mData + mDataPos, len);
        mDataPos += ALIGN4(len);
        return NOERROR;
    }
    return E_NOT_FOUND_EXCEPTION;
}

const void* CDBusParcel::ReadInplace(
    /* [in] */ Long len) const
{
    if (len < 0) {
        return nullptr;
    }

    if ((mDataPos + ALIGN4(len)) >= mDataPos &&
                  (mDataPos + ALIGN4(len)) <= mDataSize && len <= ALIGN4(len)) {
        const void* data = mData + mDataPos;
        mDataPos += ALIGN4(len);
        return data;
    }
    return nullptr;
}

ECode CDBusParcel::Write(
    /* [in] */ const void* data,
    /* [in] */ Long len)
{
    if (len < 0) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    mError = NOERROR;

    void* const d = WriteInplace(len);
    if (d != nullptr) {
        memcpy(d, data, len);
        return NOERROR;
    }
    return mError;
}

void* CDBusParcel::WriteInplace(
    /* [in] */ Long len)
{
    if (len < 0) {
        return nullptr;
    }

    const Long padded = ALIGN4(len);

    if ((mDataPos + padded) < mDataPos) {
        return nullptr;
    }

    ECode ec;
    if ((mDataPos + padded) > mDataCapacity) {
        ec = GrowData(padded);
        if (FAILED(ec)) {
            return nullptr;
        }
    }

    Byte* const data = mData + mDataPos;

    /**
     * Set the memory used for alignment to 0U.
     */
    if (padded != len) {
#if __BYTE_ORDER == __BIG_ENDIAN
        static const uint32_t mask[4] = {
            0x00000000U, 0xffffff00U, 0xffff0000U, 0xff000000U
        };
#elif __BYTE_ORDER == __LITTLE_ENDIAN
        static const uint32_t mask[4] = {
            0x00000000U, 0x00ffffffU, 0x0000ffffU, 0x000000ffU
        };
#endif
        *reinterpret_cast<uint32_t*>(data + padded - 4) &= mask[padded - len];
    }

    ec = FinishWrite(padded);
    if (FAILED(ec)) {
        return nullptr;
    }

    return data;
}

ECode CDBusParcel::FinishWrite(
    /* [in] */ Long len)
{
    if (len < 0) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    mDataPos += len;
    if (mDataPos > mDataSize) {
        mDataSize = mDataPos;
    }
    return NOERROR;
}

ECode CDBusParcel::GrowData(
    /* [in] */ Long len)
{
    if (len < 0) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    Long newSize = ((mDataSize + len) * 3) / 2;
    return (newSize <= mDataSize) ?
                                E_OUT_OF_MEMORY_ERROR : ContinueWrite(newSize);
}

ECode CDBusParcel::RestartWrite(
    /* [in] */ Long desired)
{
    if ((desired < 0) || (desired > ComoConfig::MAX_SIZE_Parcel)) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    if (desired < MAX_BUFFER_SIZE) {
        if (mData != mBuffer) {
            free(mData);
        }

        mData = mBuffer;
        mDataCapacity = MAX_BUFFER_SIZE;
        mDataSize = mDataPos = 0;
        return NOERROR;
    }

    Byte* data;
    if (mData != mBuffer) {
        data = (Byte*)realloc(mData, desired);
    }
    else {
        data = (Byte*)malloc(desired);
    }

    if ((nullptr == data) && (desired > mDataCapacity)) {
        mError = E_OUT_OF_MEMORY_ERROR;
        return E_OUT_OF_MEMORY_ERROR;
    }

    if (data != nullptr) {
        mData = data;
        mDataCapacity = desired;
    }

    mDataSize = 0;
    mDataPos = 0;
    return NOERROR;
}

ECode CDBusParcel::ContinueWrite(
    /* [in] */ Long desired)
{
    if ((desired < 0) || (desired > ComoConfig::MAX_SIZE_Parcel)) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    if (mData != mBuffer) {
        if (desired > mDataCapacity) {
            Byte* data = (Byte*)realloc(mData, desired);
            if (data != nullptr) {
                mData = data;
                mDataCapacity = desired;
            }
            else {
                mError = E_OUT_OF_MEMORY_ERROR;
                return E_OUT_OF_MEMORY_ERROR;
            }
        }
        else {
            if (mDataSize > desired) {
                mDataSize = desired;
            }
            if (mDataPos > desired) {
                mDataPos = desired;
            }
        }
    }
    else {
        if (desired > mDataCapacity) {
            Byte* data = (Byte*)malloc(desired);
            if (nullptr == data) {
                mError = E_OUT_OF_MEMORY_ERROR;
                return E_OUT_OF_MEMORY_ERROR;
            }

            memcpy(data, mData, mDataSize);
            mData = data;
            mDataCapacity = desired;
        }
        else {
            if (mDataSize > desired) {
                mDataSize = desired;
            }
            if (mDataPos > desired) {
                mDataPos = desired;
            }
        }
    }

    return NOERROR;
}

template<typename T>
ECode CDBusParcel::ReadAligned(
    /* [out] */ T* value) const
{
    if (sizeof(T) == 8) {
        mDataPos = ALIGN8(mDataPos);
    }

    if ((mDataPos + sizeof(T)) <= mDataSize) {
        const void* data = mData + mDataPos;
        mDataPos += sizeof(T);
        *value = *reinterpret_cast<const T*>(data);
        return NOERROR;
    }
    else {
        *value = 0;
        return E_NOT_FOUND_EXCEPTION;
    }
}

template<typename T>
ECode CDBusParcel::WriteAligned(
    /* [in] */ T value)
{
    Long oldDataPos = mDataPos;
    if (sizeof(T) == 8) {
        mDataPos = ALIGN8(mDataPos);
    }

    if ((mDataPos + sizeof(value)) > mDataCapacity) {
        ECode ec = GrowData(mDataPos - oldDataPos + sizeof(value));
        if (FAILED(ec)) {
            return ec;
        }
    }

    *reinterpret_cast<T*>(mData + mDataPos) = value;
    return FinishWrite(sizeof(value));
}

ECode CDBusParcel::CreateObject(
    /* [out] */ AutoPtr<IParcel>& parcel)
{
    parcel = new CDBusParcel();
    if (nullptr == parcel) {
        return E_OUT_OF_MEMORY_ERROR;
    }

    return NOERROR;
}

ECode CDBusParcel::SetServerInfo(
    /* [in] */ const String& serverName)
{
    mServerName = serverName;
    return NOERROR;
}

ECode CDBusParcel::SetProxyInfo(
    /* [in] */ Long proxyId)
{
    mProxyId = proxyId;
    return NOERROR;
}

} // namespace como
