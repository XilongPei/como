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

#ifndef __COMO_CZMQPARCEL_H__
#define __COMO_CZMQPARCEL_H__

#include "util/comoobj.h"
#include "util/MemPool.h"

namespace como {

extern const CoclassID CID_CZMQParcel;

COCLASS_ID(abc0f905-d6f3-6696-8a0a-31037f69cddd)
class CZMQParcel
    : public Object
    , public IParcel
{
public:
    CZMQParcel();

    ~CZMQParcel();

    COMO_INTERFACE_DECL();

    COMO_OBJECT_DECL();

    static void *MemPoolAlloc(size_t ulSize);
    static void MemPoolFree(Short id, const void *p);

    ECode ReadChar(
        /* [out] */ Char& value) override;

    ECode WriteChar(
        /* [in] */ Char value) override;

    ECode ReadByte(
        /* [out] */ Byte& value) override;

    ECode WriteByte(
        /* [in] */ Byte value) override;

    ECode ReadShort(
        /* [out] */ Short& value) override;

    ECode WriteShort(
        /* [in] */ Short value) override;

    ECode ReadInteger(
        /* [out] */ Integer& value) override;

    ECode WriteInteger(
        /* [in] */ Integer value) override;

    ECode ReadLong(
        /* [out] */ Long& value) override;

    ECode WriteLong(
        /* [in] */ Long value) override;

    ECode ReadFloat(
        /* [out] */ Float& value) override;

    ECode WriteFloat(
        /* [in] */ Float value) override;

    ECode ReadDouble(
        /* [out] */ Double& value) override;

    ECode WriteDouble(
        /* [in] */ Double value) override;

    ECode ReadBoolean(
        /* [out] */ Boolean& value) override;

    ECode WriteBoolean(
        /* [in] */ Boolean value) override;

    ECode ReadString(
        /* [out] */ String& value) override;

    ECode WriteString(
        /* [in] */ const String& value) override;

    ECode ReadCoclassID(
        /* [out] */ CoclassID& value) override;

    ECode WriteCoclassID(
        /* [in] */ const CoclassID& value) override;

    ECode ReadComponentID(
        /* [out] */ ComponentID& value) override;

    ECode WriteComponentID(
        /* [in] */ const ComponentID& value) override;

    ECode ReadInterfaceID(
        /* [out] */ InterfaceID& value) override;

    ECode WriteInterfaceID(
        /* [in] */ const InterfaceID& value) override;

    ECode ReadECode(
        /* [out] */ ECode& value) override;

    ECode WriteECode(
        /* [in] */ ECode value) override;

    ECode ReadEnumeration(
        /* [out] */ Integer& value) override;

    ECode WriteEnumeration(
        /* [in] */ Integer value) override;

    ECode ReadArray(
        /* [out] */ Triple* array) override;

    ECode WriteArray(
        /* [in] */ const Triple& array) override;

    ECode ReadInterface(
        /* [out] */ AutoPtr<IInterface>& value) override;

    ECode WriteInterface(
        /* [in] */ IInterface* value) override;

    ECode GetData(
        /* [out] */ HANDLE& data) override;

    ECode GetDataSize(
        /* [out] */ Long& size) override;

    ECode SetData(
        /* [in] */ HANDLE data,
        /* [in] */ Long size) override;

    ECode GetDataPosition(
        /* [out] */ Long& pos) override;

    ECode SetDataPosition(
        /* [in] */ Long pos) override;

    ECode GetPayload(
        /* [out] */ HANDLE& payload) override;

    ECode SetPayload(
        /* [in] */ HANDLE payload,
        /* [in] */ Boolean release) override;

    ECode SetServerInfo(
        /* [in] */ const String& serverName) override;

    ECode SetProxyInfo(
        /* [in] */ Long proxyId) override;

    static ECode CreateObject(
        /* [out] */ AutoPtr<IParcel>& parcel);

    inline static CZMQParcel* From(
        /* [in] */ IParcel* parcel);

private:
    ECode Read(
        /* [in] */ void* outData,
        /* [in] */ Long len) const;

    const void* ReadInplace(
        /* [in] */ Long len) const;

    ECode Write(
        /* [in] */ const void* data,
        /* [in] */ Long len);

    void* WriteInplace(
        /* [in] */ Long len);

    ECode FinishWrite(
        /* [in] */ Long len);

    ECode GrowData(
        /* [in] */ Long len);

    ECode RestartWrite(
        /* [in] */ Long desired);

    ECode ContinueWrite(
        /* [in] */ Long desired);

    template<typename T>
    ECode ReadAligned(
        /* [out] */ T* value) const;

    template<typename T>
    ECode WriteAligned(
        /* [in] */ T value);

private:
    static constexpr Integer TAG_NULL = 0;
    static constexpr Integer TAG_NOT_NULL = 1;
    static constexpr Integer MAX_BUFFER_SIZE = 128;

    static CMemPool *memPool;

    ECode           mError;
    Byte           *mData;
    Long            mDataSize;
    Long            mDataCapacity;
    mutable Long    mDataPos;
    Byte            mBuffer[MAX_BUFFER_SIZE];

    String          mServerName;
    Long            mProxyId;
};

inline static CZMQParcel* From(
    /* [in] */ IParcel* parcel)
{
    return static_cast<CZMQParcel*>(parcel);
}

} // namespace como

#endif //__COMO_CZMQPARCEL_H__
