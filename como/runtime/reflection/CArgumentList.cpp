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

#include "reflection/CArgumentList.h"
#include "util/comolog.h"
#include <cstdlib>

namespace como {

COMO_INTERFACE_IMPL_LIGHT_1(CArgumentList, LightRefBase, IArgumentList)

CArgumentList::CArgumentList(
    /* [in] */ const Array<IMetaParameter*>& parameters)
    : mParameterNumber(parameters.GetLength())
    , mHasOutArguments(0)
    , mHotCode(0)
{
    mParameterBufferSize = Init(parameters);

    /**
     * Judging whether member mParameterBuffer is nullptr determines whether the
     * object is successfully constructed.
     */
}

CArgumentList::CArgumentList(
    /* [in] */ const Array<IMetaParameter*>& parameters,
    /* [in] */ Integer hasOutArguments)
    : mParameterNumber(parameters.GetLength())
    , mHasOutArguments(hasOutArguments)
    , mHotCode(0)
{
    mParameterBufferSize = Init(parameters);

    /**
     * Judging ......
     * (see above)
     */
}

/**
 * Construct an object with reference to an existing CArgumentList(argList).
 */
CArgumentList::CArgumentList(
    /* [in] */ IArgumentList* argList)
    : mHotCode(0)
{
    mParameterNumber = From(argList)->mParameterNumber;
    mHasOutArguments = From(argList)->mHasOutArguments;
    mParameterBufferSize = Init_FromMemory(From(argList));

    /**
     * Judging ......
     * (see above)
     */
}

CArgumentList::~CArgumentList()
{
    if (mParameterInfos != nullptr) {
        free(mParameterInfos);
    }
    if (mParameterBuffer != nullptr) {
        free(mParameterBuffer);
    }
}

template<typename T>
static inline T Get(Byte* buffer, Integer pos)
{
    return *reinterpret_cast<T*>(buffer + pos);
}

template<typename T>
static inline void Put(Byte* buffer, Integer pos, T value)
{
    *reinterpret_cast<T*>(buffer + pos) = value;
}

ECode CArgumentList::GetInputArgumentOfByte(
    /* [in] */ Integer index,
    /* [out] */ Byte& value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Byte) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    value = Get<Byte>(mParameterBuffer, mParameterInfos[index].mPos);
    return NOERROR;
}

ECode CArgumentList::SetInputArgumentOfByte(
    /* [in] */ Integer index,
    /* [in] */ Byte value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Byte) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<Byte>(mParameterBuffer, mParameterInfos[index].mPos, value);
    return NOERROR;
}

ECode CArgumentList::GetInputArgumentOfShort(
    /* [in] */ Integer index,
    /* [out] */ Short& value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Short) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    value = Get<Short>(mParameterBuffer, mParameterInfos[index].mPos);
    return NOERROR;
}

ECode CArgumentList::SetInputArgumentOfShort(
    /* [in] */ Integer index,
    /* [in] */ Short value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Short) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<Short>(mParameterBuffer, mParameterInfos[index].mPos, value);
    return NOERROR;
}

ECode CArgumentList::GetInputArgumentOfInteger(
    /* [in] */ Integer index,
    /* [out] */ Integer& value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Integer) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    value = Get<Integer>(mParameterBuffer, mParameterInfos[index].mPos);
    return NOERROR;
}

ECode CArgumentList::SetInputArgumentOfInteger(
    /* [in] */ Integer index,
    /* [in] */ Integer value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Integer) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<Integer>(mParameterBuffer, mParameterInfos[index].mPos, value);
    return NOERROR;
}

ECode CArgumentList::GetInputArgumentOfLong(
    /* [in] */ Integer index,
    /* [out] */ Long& value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Long) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    value = Get<Long>(mParameterBuffer, mParameterInfos[index].mPos);
    return NOERROR;
}

ECode CArgumentList::SetInputArgumentOfLong(
    /* [in] */ Integer index,
    /* [in] */ Long value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Long) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<Long>(mParameterBuffer, mParameterInfos[index].mPos, value);
    return NOERROR;
}

ECode CArgumentList::GetInputArgumentOfFloat(
    /* [in] */ Integer index,
    /* [out] */ Float& value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Float) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    value = Get<Float>(mParameterBuffer, mParameterInfos[index].mPos);
    return NOERROR;
}

ECode CArgumentList::SetInputArgumentOfFloat(
    /* [in] */ Integer index,
    /* [out] */ Float value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Float) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<Float>(mParameterBuffer, mParameterInfos[index].mPos, value);
    return NOERROR;
}

ECode CArgumentList::GetInputArgumentOfDouble(
    /* [in] */ Integer index,
    /* [out] */ Double& value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Double) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    value = Get<Double>(mParameterBuffer, mParameterInfos[index].mPos);
    return NOERROR;
}

ECode CArgumentList::SetInputArgumentOfDouble(
    /* [in] */ Integer index,
    /* [in] */ Double value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Double) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<Double>(mParameterBuffer, mParameterInfos[index].mPos, value);
    return NOERROR;
}

ECode CArgumentList::GetInputArgumentOfChar(
    /* [in] */ Integer index,
    /* [out] */ Char& value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Char) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    value = Get<Char>(mParameterBuffer, mParameterInfos[index].mPos);
    return NOERROR;
}

ECode CArgumentList::SetInputArgumentOfChar(
    /* [in] */ Integer index,
    /* [in] */ Char value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Char) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<Char>(mParameterBuffer, mParameterInfos[index].mPos, value);
    return NOERROR;
}

ECode CArgumentList::GetInputArgumentOfBoolean(
    /* [in] */ Integer index,
    /* [out] */ Boolean& value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Boolean) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    value = Get<Boolean>(mParameterBuffer, mParameterInfos[index].mPos);
    return NOERROR;
}

ECode CArgumentList::SetInputArgumentOfBoolean(
    /* [in] */ Integer index,
    /* [in] */ Boolean value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Boolean) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<Boolean>(mParameterBuffer, mParameterInfos[index].mPos, value);
    return NOERROR;
}

ECode CArgumentList::GetInputArgumentOfString(
    /* [in] */ Integer index,
    /* [out] */ String& value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::String) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    value = *Get<String*>(mParameterBuffer, mParameterInfos[index].mPos);
    return NOERROR;
}

ECode CArgumentList::SetInputArgumentOfString(
    /* [in] */ Integer index,
    /* [in] */ const String& value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::String) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<String*>(mParameterBuffer, mParameterInfos[index].mPos, const_cast<String*>(&value));
    return NOERROR;
}

ECode CArgumentList::GetInputArgumentOfHANDLE(
    /* [in] */ Integer index,
    /* [out] */ HANDLE& value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::HANDLE) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    value = Get<HANDLE>(mParameterBuffer, mParameterInfos[index].mPos);
    return NOERROR;
}

ECode CArgumentList::SetInputArgumentOfHANDLE(
    /* [in] */ Integer index,
    /* [in] */ HANDLE value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::HANDLE) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<HANDLE>(mParameterBuffer, mParameterInfos[index].mPos, value);
    return NOERROR;
}

ECode CArgumentList::GetInputArgumentOfECode(
    /* [in] */ Integer index,
    /* [out] */ ECode& value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::ECode) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    value = Get<ECode>(mParameterBuffer, mParameterInfos[index].mPos);
    return NOERROR;
}

ECode CArgumentList::SetInputArgumentOfECode(
    /* [in] */ Integer index,
    /* [in] */ ECode value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::ECode) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<ECode>(mParameterBuffer, mParameterInfos[index].mPos, value);
    return NOERROR;
}

ECode CArgumentList::GetInputArgumentOfCoclassID(
    /* [in] */ Integer index,
    /* [out] */ CoclassID& value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::CoclassID) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    value = *Get<CoclassID*>(mParameterBuffer, mParameterInfos[index].mPos);
    return NOERROR;
}

ECode CArgumentList::SetInputArgumentOfCoclassID(
    /* [in] */ Integer index,
    /* [in] */ const CoclassID& value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::CoclassID) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<CoclassID*>(mParameterBuffer, mParameterInfos[index].mPos,
                                                const_cast<CoclassID*>(&value));
    return NOERROR;
}

ECode CArgumentList::GetInputArgumentOfComponentID(
    /* [in] */ Integer index,
    /* [out] */ ComponentID& value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::ComponentID) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    value = *Get<ComponentID*>(mParameterBuffer, mParameterInfos[index].mPos);
    return NOERROR;
}

ECode CArgumentList::SetInputArgumentOfComponentID(
    /* [in] */ Integer index,
    /* [in] */ const ComponentID& value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::ComponentID) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<ComponentID*>(mParameterBuffer, mParameterInfos[index].mPos,
                                              const_cast<ComponentID*>(&value));
    return NOERROR;
}

ECode CArgumentList::GetInputArgumentOfInterfaceID(
    /* [in] */ Integer index,
    /* [out] */ InterfaceID& value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::InterfaceID) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    value = *Get<InterfaceID*>(mParameterBuffer, mParameterInfos[index].mPos);
    return NOERROR;
}

ECode CArgumentList::SetInputArgumentOfInterfaceID(
    /* [in] */ Integer index,
    /* [in] */ const InterfaceID& value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::InterfaceID) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<InterfaceID*>(mParameterBuffer, mParameterInfos[index].mPos,
                                              const_cast<InterfaceID*>(&value));
    return NOERROR;
}

ECode CArgumentList::GetInputArgumentOfArray(
    /* [in] */ Integer index,
    /* [out] */ Triple& value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Array) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    value = *Get<Triple*>(mParameterBuffer, mParameterInfos[index].mPos);
    return NOERROR;
}

ECode CArgumentList::SetInputArgumentOfArray(
    /* [in] */ Integer index,
    /* [in] */ const Triple& value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Array) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<Triple*>(mParameterBuffer, mParameterInfos[index].mPos, const_cast<Triple*>(&value));
    return NOERROR;
}

ECode CArgumentList::GetInputArgumentOfEnumeration(
    /* [in] */ Integer index,
    /* [out] */ Integer& value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Enum) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    value = Get<Integer>(mParameterBuffer, mParameterInfos[index].mPos);
    return NOERROR;
}

ECode CArgumentList::SetInputArgumentOfEnumeration(
    /* [in] */ Integer index,
    /* [in] */ Integer value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Enum) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<Integer>(mParameterBuffer, mParameterInfos[index].mPos, value);
    return NOERROR;
}

ECode CArgumentList::GetInputArgumentOfInterface(
    /* [in] */ Integer index,
    /* [out] */ AutoPtr<IInterface>& value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Interface) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    value = Get<IInterface*>(mParameterBuffer, mParameterInfos[index].mPos);
    return NOERROR;
}

ECode CArgumentList::SetInputArgumentOfInterface(
    /* [in] */ Integer index,
    /* [in] */ IInterface* value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Interface) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if (mParameterInfos[index].mIOAttr != IOAttribute::IN) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<IInterface*>(mParameterBuffer, mParameterInfos[index].mPos, value);
    return NOERROR;
}

ECode CArgumentList::AssignOutputArgumentOfByte(
    /* [in] */ Integer index,
    /* [in] */ Byte value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Byte) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Byte* addr = Get<Byte*>(mParameterBuffer, mParameterInfos[index].mPos);
    *addr = value;
    return NOERROR;
}

ECode CArgumentList::SetOutputArgumentOfByte(
    /* [in] */ Integer index,
    /* [in] */ HANDLE addr)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Byte) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<HANDLE>(mParameterBuffer, mParameterInfos[index].mPos, addr);
    return NOERROR;
}

ECode CArgumentList::AssignOutputArgumentOfShort(
    /* [in] */ Integer index,
    /* [in] */ Short value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Short) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Short* addr = Get<Short*>(mParameterBuffer, mParameterInfos[index].mPos);
    *addr = value;
    return NOERROR;
}

ECode CArgumentList::SetOutputArgumentOfShort(
    /* [in] */ Integer index,
    /* [in] */ HANDLE addr)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Short) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<HANDLE>(mParameterBuffer, mParameterInfos[index].mPos, addr);
    return NOERROR;
}

ECode CArgumentList::AssignOutputArgumentOfInteger(
    /* [in] */ Integer index,
    /* [in] */ Integer value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Integer) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Integer* addr = Get<Integer*>(mParameterBuffer, mParameterInfos[index].mPos);
    *addr = value;
    return NOERROR;
}

ECode CArgumentList::SetOutputArgumentOfInteger(
    /* [in] */ Integer index,
    /* [in] */ HANDLE addr)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Integer) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<HANDLE>(mParameterBuffer, mParameterInfos[index].mPos, addr);
    return NOERROR;
}

ECode CArgumentList::AssignOutputArgumentOfLong(
    /* [in] */ Integer index,
    /* [in] */ Long value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Long) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Long* addr = Get<Long*>(mParameterBuffer, mParameterInfos[index].mPos);
    *addr = value;
    return NOERROR;
}

ECode CArgumentList::SetOutputArgumentOfLong(
    /* [in] */ Integer index,
    /* [in] */ HANDLE addr)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Long) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<HANDLE>(mParameterBuffer, mParameterInfos[index].mPos, addr);
    return NOERROR;
}

ECode CArgumentList::AssignOutputArgumentOfFloat(
    /* [in] */ Integer index,
    /* [in] */ Float value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Float) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Float* addr = Get<Float*>(mParameterBuffer, mParameterInfos[index].mPos);
    *addr = value;
    return NOERROR;
}

ECode CArgumentList::SetOutputArgumentOfFloat(
    /* [in] */ Integer index,
    /* [in] */ HANDLE addr)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Float) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<HANDLE>(mParameterBuffer, mParameterInfos[index].mPos, addr);
    return NOERROR;
}

ECode CArgumentList::AssignOutputArgumentOfDouble(
    /* [in] */ Integer index,
    /* [in] */ Double value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Double) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Double* addr = Get<Double*>(mParameterBuffer, mParameterInfos[index].mPos);
    *addr = value;
    return NOERROR;
}

ECode CArgumentList::SetOutputArgumentOfDouble(
    /* [in] */ Integer index,
    /* [in] */ HANDLE addr)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Double) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<HANDLE>(mParameterBuffer, mParameterInfos[index].mPos, addr);
    return NOERROR;
}

ECode CArgumentList::AssignOutputArgumentOfChar(
    /* [in] */ Integer index,
    /* [in] */ Char value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Char) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Char* addr = Get<Char*>(mParameterBuffer, mParameterInfos[index].mPos);
    *addr = value;
    return NOERROR;
}

ECode CArgumentList::SetOutputArgumentOfChar(
    /* [in] */ Integer index,
    /* [in] */ HANDLE addr)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Char) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<HANDLE>(mParameterBuffer, mParameterInfos[index].mPos, addr);
    return NOERROR;
}

ECode CArgumentList::AssignOutputArgumentOfBoolean(
    /* [in] */ Integer index,
    /* [in] */ Boolean value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Boolean) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Boolean* addr = Get<Boolean*>(mParameterBuffer, mParameterInfos[index].mPos);
    *addr = value;
    return NOERROR;
}

ECode CArgumentList::SetOutputArgumentOfBoolean(
    /* [in] */ Integer index,
    /* [in] */ HANDLE addr)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Boolean) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<HANDLE>(mParameterBuffer, mParameterInfos[index].mPos, addr);
    return NOERROR;
}

ECode CArgumentList::AssignOutputArgumentOfString(
    /* [in] */ Integer index,
    /* [in] */ const String& value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::String) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    String* addr = Get<String*>(mParameterBuffer, mParameterInfos[index].mPos);
    *addr = value;
    return NOERROR;
}

ECode CArgumentList::SetOutputArgumentOfString(
    /* [in] */ Integer index,
    /* [in] */ HANDLE addr)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::String) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<HANDLE>(mParameterBuffer, mParameterInfos[index].mPos, addr);
    return NOERROR;
}

ECode CArgumentList::AssignOutputArgumentOfHANDLE(
    /* [in] */ Integer index,
    /* [in] */ HANDLE value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::HANDLE) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    HANDLE* addr = Get<HANDLE*>(mParameterBuffer, mParameterInfos[index].mPos);
    *addr = value;
    return NOERROR;
}

ECode CArgumentList::SetOutputArgumentOfHANDLE(
    /* [in] */ Integer index,
    /* [in] */ HANDLE addr)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::HANDLE) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<HANDLE>(mParameterBuffer, mParameterInfos[index].mPos, addr);
    return NOERROR;
}

ECode CArgumentList::AssignOutputArgumentOfECode(
    /* [in] */ Integer index,
    /* [in] */ ECode value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::ECode) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    ECode* addr = Get<ECode*>(mParameterBuffer, mParameterInfos[index].mPos);
    *addr = value;
    return NOERROR;
}

ECode CArgumentList::SetOutputArgumentOfECode(
    /* [in] */ Integer index,
    /* [in] */ HANDLE addr)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::ECode) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<HANDLE>(mParameterBuffer, mParameterInfos[index].mPos, addr);
    return NOERROR;
}

ECode CArgumentList::AssignOutputArgumentOfCoclassID(
    /* [in] */ Integer index,
    /* [in] */ const CoclassID& value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::CoclassID) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    CoclassID* addr = Get<CoclassID*>(mParameterBuffer, mParameterInfos[index].mPos);
    *addr = value;
    return NOERROR;
}

ECode CArgumentList::SetOutputArgumentOfCoclassID(
    /* [in] */ Integer index,
    /* [in] */ HANDLE addr)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::CoclassID) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<HANDLE>(mParameterBuffer, mParameterInfos[index].mPos, addr);
    return NOERROR;
}

ECode CArgumentList::AssignOutputArgumentOfComponentID(
    /* [in] */ Integer index,
    /* [in] */ const ComponentID& value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::ComponentID) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    ComponentID* addr = Get<ComponentID*>(mParameterBuffer, mParameterInfos[index].mPos);
    *addr = value;
    return NOERROR;
}

ECode CArgumentList::SetOutputArgumentOfComponentID(
    /* [in] */ Integer index,
    /* [in] */ HANDLE addr)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::ComponentID) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<HANDLE>(mParameterBuffer, mParameterInfos[index].mPos, addr);
    return NOERROR;
}

ECode CArgumentList::AssignOutputArgumentOfInterfaceID(
    /* [in] */ Integer index,
    /* [in] */ const InterfaceID& value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::InterfaceID) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    InterfaceID* addr = Get<InterfaceID*>(mParameterBuffer, mParameterInfos[index].mPos);
    *addr = value;
    return NOERROR;
}

ECode CArgumentList::SetOutputArgumentOfInterfaceID(
    /* [in] */ Integer index,
    /* [in] */ HANDLE addr)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::InterfaceID) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<HANDLE>(mParameterBuffer, mParameterInfos[index].mPos, addr);
    return NOERROR;
}

ECode CArgumentList::AssignOutputArgumentOfArray(
    /* [in] */ Integer index,
    /* [in] */ const Triple& value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Array) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Triple* addr = Get<Triple*>(mParameterBuffer, mParameterInfos[index].mPos);
    *addr = value;
    return NOERROR;
}

ECode CArgumentList::SetOutputArgumentOfArray(
    /* [in] */ Integer index,
    /* [in] */ HANDLE addr)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Array) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<HANDLE>(mParameterBuffer, mParameterInfos[index].mPos, addr);
    return NOERROR;
}

ECode CArgumentList::AssignOutputArgumentOfEnumeration(
    /* [in] */ Integer index,
    /* [in] */ Integer value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Enum) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Integer* addr = Get<Integer*>(mParameterBuffer, mParameterInfos[index].mPos);
    *addr = value;
    return NOERROR;
}

ECode CArgumentList::SetOutputArgumentOfEnumeration(
    /* [in] */ Integer index,
    /* [in] */ HANDLE addr)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Enum) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<HANDLE>(mParameterBuffer, mParameterInfos[index].mPos, addr);
    return NOERROR;
}

ECode CArgumentList::AssignOutputArgumentOfInterface(
    /* [in] */ Integer index,
    /* [in] */ IInterface* value)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Interface) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    IInterface** addr = Get<IInterface**>(mParameterBuffer, mParameterInfos[index].mPos);
    *addr = value;
    return NOERROR;
}

ECode CArgumentList::SetOutputArgumentOfInterface(
    /* [in] */ Integer index,
    /* [in] */ HANDLE addr)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }

        if (mParameterInfos[index].mKind != TypeKind::Interface) {
            return E_TYPE_MISMATCH_EXCEPTION;
        }

        if ((mParameterInfos[index].mIOAttr != IOAttribute::OUT) &&
                (mParameterInfos[index].mIOAttr != IOAttribute::IN_OUT)) {
            return E_IOATTRIBUTE_MISMATCH_EXCEPTION;
        }
    }

    Put<HANDLE>(mParameterBuffer, mParameterInfos[index].mPos, addr);
    return NOERROR;
}

ECode CArgumentList::GetArgumentAddress(
    /* [in] */ Integer index,
    /* [out] */ HANDLE& addr)
{
    if (UNLIKELY(! mHotCode)) {
        if ((index < 0) || (index >= mParameterNumber)) {
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
        }
    }

    addr = Get<HANDLE>(mParameterBuffer, mParameterInfos[index].mPos);
    return NOERROR;
}

int CArgumentList::Init(
    /* [in] */ const Array<IMetaParameter*>& parameters)
{
    Integer bufferPos = sizeof(HANDLE);     // for this pointer

    if (mParameterNumber > 0) {
        mParameterInfos = reinterpret_cast<ParameterInfo*>(calloc(
                                      sizeof(ParameterInfo), mParameterNumber));
        if (nullptr == mParameterInfos) {
            Logger::E("CArgumentList::Init", "Out of memory.");
            return -1;
        }

        for (Integer i = 0;  i < mParameterNumber;  i++) {
            InitParameterInfo(bufferPos, parameters[i], mParameterInfos[i]);
        }
    }

    mParameterBuffer = reinterpret_cast<Byte*>(calloc(sizeof(Byte), bufferPos));
    if (nullptr == mParameterBuffer) {
        Logger::E("CArgumentList::Init", "Out of memory.");
        return -2;
    }

    return bufferPos;
}

int CArgumentList::Init_FromMemory(
    /* [in] */ CArgumentList* argsMemory)
{
    if (mParameterNumber > 0) {
        mParameterInfos = reinterpret_cast<ParameterInfo*>(calloc(
                                        sizeof(ParameterInfo), mParameterNumber));
        if (nullptr == mParameterInfos) {
            Logger::E("CArgumentList::Init_FromMemory", "Out of memory.");
            return -1;
        }

        memcpy(mParameterInfos, argsMemory->mParameterInfos,
                                        sizeof(ParameterInfo) * mParameterNumber);
    }

    mParameterBuffer = reinterpret_cast<Byte*>(calloc(sizeof(Byte), argsMemory->mParameterBufferSize));
    if (nullptr == mParameterBuffer) {
        Logger::E("CArgumentList::Init_FromMemory", "Out of memory.");
        return -2;
    }

    memcpy(mParameterBuffer, argsMemory->mParameterBuffer, argsMemory->mParameterBufferSize);
    return argsMemory->mParameterBufferSize;
}

void CArgumentList::InitParameterInfo(
    /* [in, out] */ Integer& bufferPos,
    /* [in] */ IMetaParameter* parameter,
    /* [out] */ ParameterInfo& paramInfo)
{
    AutoPtr<IMetaType> type;
    (void)parameter->GetType(type);
    (void)parameter->GetIOAttribute(paramInfo.mIOAttr);
    (void)type->GetTypeKind(paramInfo.mKind);

    if (paramInfo.mIOAttr == IOAttribute::IN) {
        switch(paramInfo.mKind) {
            case TypeKind::Char:
            case TypeKind::Byte:
            case TypeKind::Short:
            case TypeKind::Integer:
            case TypeKind::Boolean:
            case TypeKind::ECode:
            case TypeKind::Enum:
            case TypeKind::TypeKind:
                paramInfo.mNumberType = NUMBER_TYPE_INTEGER;
                paramInfo.mPos = bufferPos;
                paramInfo.mSize = 4;
                break;

            case TypeKind::Float:
                paramInfo.mNumberType = NUMBER_TYPE_FLOATING_POINT;
                paramInfo.mPos = bufferPos;
                paramInfo.mSize = 4;
                break;

            case TypeKind::Long:
            case TypeKind::String:
            case TypeKind::CoclassID:
            case TypeKind::ComponentID:
            case TypeKind::InterfaceID:
            case TypeKind::HANDLE:
            case TypeKind::Array:
            case TypeKind::Interface:
            case TypeKind::Triple:
                paramInfo.mNumberType = NUMBER_TYPE_INTEGER;
                paramInfo.mPos = ALIGN8(bufferPos);
                paramInfo.mSize = 8;
                break;

            case TypeKind::Double:
                paramInfo.mNumberType = NUMBER_TYPE_FLOATING_POINT;
                paramInfo.mPos = ALIGN8(bufferPos);
                paramInfo.mSize = 8;
                break;

            default:
                CHECK(0);
                break;
        }
    }
    else {
        switch(paramInfo.mKind) {
            case TypeKind::Char:
            case TypeKind::Byte:
            case TypeKind::Short:
            case TypeKind::Integer:
            case TypeKind::Long:
            case TypeKind::Float:
            case TypeKind::Double:
            case TypeKind::Boolean:
            case TypeKind::String:
            case TypeKind::CoclassID:
            case TypeKind::ComponentID:
            case TypeKind::InterfaceID:
            case TypeKind::HANDLE:
            case TypeKind::ECode:
            case TypeKind::Enum:
            case TypeKind::Array:
            case TypeKind::Interface:
            case TypeKind::Triple:
            case TypeKind::TypeKind:
                paramInfo.mNumberType = NUMBER_TYPE_INTEGER;
                paramInfo.mPos = ALIGN8(bufferPos);
                paramInfo.mSize = 8;
                break;
            default:
                CHECK(0);
                break;
        }
    }
    bufferPos = paramInfo.mPos + paramInfo.mSize;
}

} // namespace como
