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

#ifndef __COMO_ARRAY_H__
#define __COMO_ARRAY_H__

#include "comoshbuf.h"
#include "comolog.h"
#include <initializer_list>

namespace como {

/**
 * 1. After `arr = new Array<?>()`, object is constructed, test whether the
 * construction is successful by whether arr->IsNull() is true or false.
 *
 * 2. Array manages memory by reference counting, because its memory comes from
 * SharedBuffer
 */
template<typename T>
class Array : public Triple
{
public:
    Array();

    Array(
        /* [in] */ Long size);

    Array(
        /* [in] */ const Array<T>& other);

    Array(
        /* [in] */ Array<T>&& other);

    Array(
        /* [in] */ std::initializer_list<T> list);

    ~Array();

    Long GetLength() const;

    T* GetPayload() const;

    Boolean IsNull() const;

    Boolean IsEmpty() const;

    Long Copy(
        /* [in] */ T const* srcData,
        /* [in] */ Long length);

    Long Copy(
        /* [in] */ Long thisPos,
        /* [in] */ T const* srcData,
        /* [in] */ Long length);

    Long Copy(
        /* [in] */ Long thisPos,
        /* [in] */ T const* srcData,
        /* [in] */ Long srcPos,
        /* [in] */ Long length);

    Long Copy(
        /* [in] */ const Array<T>& srcArray);

    Long Copy(
        /* [in] */ const Array<T>& srcArray,
        /* [in] */ Long length);

    Long Copy(
        /* [in] */ const Array<T>& srcArray,
        /* [in] */ Long srcPos,
        /* [in] */ Long length);

    Long Copy(
        /* [in] */ Long thisPos,
        /* [in] */ const Array<T>& srcArray);

    Long Copy(
        /* [in] */ Long thisPos,
        /* [in] */ const Array<T>& srcArray,
        /* [in] */ Long length);

    Long Copy(
        /* [in] */ Long thisPos,
        /* [in] */ const Array<T>& srcArray,
        /* [in] */ Long srcPos,
        /* [in] */ Long length);

    Long CopyRaw(
        /* [in] */ Byte const* srcData,
        /* [in] */ Long byteLength);

    void Set(
        /* [in] */ Long index,
        /* [in] */ T value);

    inline Boolean Equals(
        /* [in] */ const Array<T>& other) const;

    inline Boolean operator==(
        /* [in] */ const Array<T>& other) const;

    Array& operator=(
        /* [in] */ const Array<T>& other);

    Array& operator=(
        /* [in] */ Array<T>&& other);

    Array& operator=(
        /* [in] */ std::initializer_list<T> list);

    inline T& operator[](
        /* [in] */ Long index);

    inline const T& operator[](
        /* [in] */ Long index) const;

    T* begin();

    const T* begin() const;

    T* end();

    const T* end() const;

    void Clear();

    Array Clone() const;

    static Array Allocate(
        /* [in] */ Long size);

    static Array Null();

    operator Array<IInterface*>();

    Boolean Alloc(
        /* [in] */ Long size);
};

/**
 * Request memory, regardless of whether the original mData is already pointing
 * to memory.
 */
template<typename T>
Boolean Array<T>::Alloc(
    /* [in] */ Long size)
{
    if ((size < 0) || (size > SIZE_MAX)) {
        Logger::E("Array", "Invalid array size %lld", size);
        mData = nullptr;
        mSize = 0;
        return false;
    }

    Long byteSize = sizeof(T) * size;
    SharedBuffer* buf = SharedBuffer::Alloc(byteSize);
    if (nullptr == buf) {
        Logger::E("Array", "Malloc array which size is %lld failed.", byteSize);
        mData = nullptr;
        mSize = 0;
        return false;
    }
    void* data = buf->GetData();
    memset(data, 0, byteSize);

    mData = reinterpret_cast<T*>(data);
    mSize = size;
    return true;
}

template<typename T>
Array<T>::Array()
{
    mData = nullptr;
    mSize = 0;
    mType = Type2Kind<T>::Kind();
}

template<typename T>
Array<T>::Array(
    /* [in] */ Long size)
{
    mType = Type2Kind<T>::Kind();

    // In case of an error, function Alloc has already printed log
    (void)Alloc(size);
}

template<typename T>
Array<T>::Array(
    /* [in] */ const Array<T>& other)
{
    if (nullptr != other.mData) {
        SharedBuffer::GetBufferFromData(other.mData)->AddRef();
    }
    mData = other.mData;
    mSize = other.mSize;
}

template<typename T>
Array<T>::Array(
    /* [in] */ Array<T>&& other)
{
    mData = other.mData;
    mSize = other.mSize;
    other.mData = nullptr;
    other.mSize = 0;
}

template<typename T>
Array<T>::Array(
    /* [in] */ std::initializer_list<T> list)
    : Array(list.size())
{
    mType = Type2Kind<T>::Kind();
    if (Alloc(list.size())) {
        Long i;
        typename std::initializer_list<T>::const_iterator it;
        for (it = list.begin(), i = 0;  it != list.end();  ++it, ++i) {
            Set(i, *it);
        }
    }
}

template<typename T>
Array<T>::~Array()
{
    if (mData != nullptr) {
        SharedBuffer* sb = SharedBuffer::GetBufferFromData(mData);
        if (sb->OnlyOwner()) {
            DeleteFunc<T> deleteF;
            for (Long i = 0;  i < mSize;  i++) {
                if (nullptr != &static_cast<T*>(mData)[i]) {
                    deleteF(&static_cast<T*>(mData)[i], this);
                }
            }
        }
        sb->Release();
        mData = nullptr;
    }
    mSize = 0;
}

template<typename T>
Long Array<T>::GetLength() const
{
    return mSize;
}

template<typename T>
T* Array<T>::GetPayload() const
{
    return static_cast<T*>(mData);
}

template<typename T>
Boolean Array<T>::IsNull() const
{
    return mData == nullptr;
}

template<typename T>
Boolean Array<T>::IsEmpty() const
{
    return (nullptr == mData) || (0 == mSize);
}

template<typename T>
Long Array<T>::Copy(
    /* [in] */ T const* srcData,
    /* [in] */ Long length)
{
    if (nullptr == srcData) {
        return 0;
    }

    Long N = MIN(mSize, length);
    if (mData == srcData) {
        return N;
    }
    T* array = static_cast<T*>(mData);
    for (Long i = 0;  i < N;  i++) {
        AssignFunc<T> assignF;
        assignF(&array[i], srcData[i], mData);
    }
    return N;
}

template<typename T>
Long Array<T>::Copy(
    /* [in] */ Long thisPos,
    /* [in] */ T const* srcData,
    /* [in] */ Long length)
{
    if (nullptr == srcData) {
        return 0;
    }

    Long N = MIN(mSize - thisPos, length);
    if ((mData == srcData) && (0 == thisPos)) {
        return N;
    }
    T* array = static_cast<T*>(mData) + thisPos;
    if (mData != srcData) {
        for (Long i = 0;  i < N;  i++) {
            AssignFunc<T> assignF;
            assignF(&array[i], srcData[i], mData);
        }
    }
    else {
        for (Long i = N - 1;  i >= 0;  i--) {
            AssignFunc<T> assignF;
            assignF(&array[i], srcData[i], mData);
        }
    }
    return N;
}

template<typename T>
Long Array<T>::Copy(
    /* [in] */ Long thisPos,
    /* [in] */ T const* srcData,
    /* [in] */ Long srcPos,
    /* [in] */ Long length)
{
    if (nullptr == srcData) {
        return 0;
    }

    Long N = MIN(mSize - thisPos, length);
    if ((mData == srcData) && (thisPos == srcPos)) {
        return N;
    }
    T* array = static_cast<T*>(mData) + thisPos;
    srcData += srcPos;
    if ((mData != srcData) || (thisPos < srcPos)) {
        for (Long i = 0;  i < N;  i++) {
            AssignFunc<T> assignF;
            assignF(&array[i], srcData[i], mData);
        }
    }
    else {
        for (Long i = N - 1;  i >= 0;  i--) {
            AssignFunc<T> assignF;
            assignF(&array[i], srcData[i], mData);
        }
    }
    return N;
}

template<typename T>
Long Array<T>::Copy(
    /* [in] */ const Array<T>& srcArray)
{
    if ((nullptr == srcArray.mData) || (0 == srcArray.mSize)) {
        return 0;
    }

    Long N = MIN(mSize, srcArray.mSize);
    if (mData == srcArray.mData) {
        return N;
    }
    T* array = static_cast<T*>(mData);
    for (Long i = 0;  i < N;  i++) {
        AssignFunc<T> assignF;
        assignF(&array[i], srcArray[i], mData);
    }
    return N;
}

template<typename T>
Long Array<T>::Copy(
    /* [in] */ const Array<T>& srcArray,
    /* [in] */ Long length)
{
    if ((nullptr == srcArray.mData) || (0 == srcArray.mSize)) {
        return 0;
    }

    Long N = MIN(mSize, MIN(srcArray.mSize, length));
    if (mData == srcArray.mData) {
        return N;
    }
    T* array = static_cast<T*>(mData);
    for (Long i = 0;  i < N;  i++) {
        AssignFunc<T> assignF;
        assignF(&array[i], srcArray[i], mData);
    }
    return N;
}

template<typename T>
Long Array<T>::Copy(
    /* [in] */ const Array<T>& srcArray,
    /* [in] */ Long srcPos,
    /* [in] */ Long length)
{
    if ((nullptr == srcArray.mData) || (0 == srcArray.mSize)) {
        return 0;
    }

    Long N = MIN(mSize, MIN(srcArray.mSize - srcPos, length));
    if ((mData == srcArray.mData) && (0 == srcPos)) {
        return N;
    }
    T* array = static_cast<T*>(mData);
    for (Long i = 0;  i < N;  i++) {
        AssignFunc<T> assignF;
        assignF(&array[i], srcArray[srcPos + i], mData);
    }
    return N;
}

template<typename T>
Long Array<T>::Copy(
    /* [in] */ Long thisPos,
    /* [in] */ const Array<T>& srcArray)
{
    if ((nullptr == srcArray.mData) || (0 == srcArray.mSize)) {
        return 0;
    }

    Long N = MIN(mSize - thisPos, srcArray.mSize);
    if ((mData == srcArray.mData) && (0 == thisPos)) {
        return N;
    }
    T* array = static_cast<T*>(mData) + thisPos;
    if (mData != srcArray.mData) {
        for (Long i = 0;  i < N;  i++) {
            AssignFunc<T> assignF;
            assignF(&array[i], srcArray[i], mData);
        }
    }
    else {
        for (Long i = N - 1;  i >= 0;  i--) {
            AssignFunc<T> assignF;
            assignF(&array[i], srcArray[i], mData);
        }
    }
    return N;
}

template<typename T>
Long Array<T>::Copy(
    /* [in] */ Long thisPos,
    /* [in] */ const Array<T>& srcArray,
    /* [in] */ Long length)
{
    if ((nullptr == srcArray.mData) || (0 == srcArray.mSize)) {
        return 0;
    }

    Long N = MIN(mSize - thisPos, MIN(srcArray.mSize, length));
    if ((mData == srcArray.mData) && (thisPos == 0)) {
        return N;
    }
    T* array = static_cast<T*>(mData) + thisPos;
    if (mData != srcArray.mData) {
        for (Long i = 0;  i < N;  i++) {
            AssignFunc<T> assignF;
            assignF(&array[i], srcArray[i], mData);
        }
    }
    else {
        for (Long i = N - 1;  i >= 0;  i--) {
            AssignFunc<T> assignF;
            assignF(&array[i], srcArray[i], mData);
        }
    }
    return N;
}

template<typename T>
Long Array<T>::Copy(
    /* [in] */ Long thisPos,
    /* [in] */ const Array<T>& srcArray,
    /* [in] */ Long srcPos,
    /* [in] */ Long length)
{
    if ((nullptr == srcArray.mData) || (0 == srcArray.mSize)) {
        return 0;
    }

    Long N = MIN(mSize - thisPos, MIN(srcArray.mSize - srcPos, length));
    if ((mData == srcArray.mData) && (thisPos == srcPos)) {
        return N;
    }
    T* array = static_cast<T*>(mData) + thisPos;
    if ((mData != srcArray.mData) || (thisPos < srcPos)) {
        for (Long i = 0;  i < N;  i++) {
            AssignFunc<T> assignF;
            assignF(&array[i], srcArray[srcPos + i], mData);
        }
    }
    else {
        for (Long i = N - 1;  i >= 0;  i--) {
            AssignFunc<T> assignF;
            assignF(&array[i], srcArray[srcPos + i], mData);
        }
    }
    return N;
}

template<typename T>
Long Array<T>::CopyRaw(
    /* [in] */ Byte const* srcData,
    /* [in] */ Long byteLength)
{
    Long byteSize = MIN(byteLength, sizeof(T) * mSize);
    memcpy(static_cast<Byte*>(mData), srcData, byteSize);
    return byteSize;
}

template<typename T>
void Array<T>::Set(
    /* [in] */ Long index,
    /* [in] */ T value)
{
    if ((index < 0) || (index >= mSize)) {
        Logger::E("Array<T>::Set",
                  "E_ILLEGAL_ARGUMENT_EXCEPTION, index: %ld", index);
        return;
    }

    T* array = static_cast<T*>(mData);
    AssignFunc<T> assignF;
    assignF(&array[index], value, mData);
}

template<typename T>
Boolean Array<T>::Equals(
    /* [in] */ const Array<T>& other) const
{
    return (mData == other.mData) && (mSize == other.mSize) &&
                                                        (mType == other.mType);
}

template<typename T>
Boolean Array<T>::operator==(
    /* [in] */ const Array<T>& other) const
{
    return (mData == other.mData) && (mSize == other.mSize) &&
                                                        (mType == other.mType);
}

template<typename T>
Array<T>& Array<T>::operator=(
    /* [in] */ const Array<T>& other)
{
    if (mData == other.mData) {
        return *this;
    }

    if (other.mData != nullptr) {
        SharedBuffer::GetBufferFromData(other.mData)->AddRef();
    }
    if (mData != nullptr) {
        SharedBuffer* sb = SharedBuffer::GetBufferFromData(mData);
        if (sb->OnlyOwner()) {
            DeleteFunc<T> deleteF;
            for (Long i = 0;  i < mSize;  i++) {
                if (nullptr != &static_cast<T*>(mData)[i]) {
                    deleteF(&static_cast<T*>(mData)[i], this);
                }
            }
        }
        sb->Release();
    }
    mData = other.mData;
    mSize = other.mSize;
    return *this;
}

template<typename T>
Array<T>& Array<T>::operator=(
    /* [in] */ Array<T>&& other)
{
    if (mData != nullptr) {
        SharedBuffer* sb = SharedBuffer::GetBufferFromData(mData);
        if (sb->OnlyOwner()) {
            DeleteFunc<T> deleteF;
            for (Long i = 0;  i < mSize;  i++) {
                if (nullptr != &static_cast<T*>(mData)[i]) {
                    deleteF(&static_cast<T*>(mData)[i], this);
                }
            }
        }
        sb->Release();
    }
    mData = other.mData;
    mSize = other.mSize;
    other.mData = nullptr;
    other.mSize = 0;
    return *this;
}

template<typename T>
Array<T>& Array<T>::operator=(
    /* [in] */ std::initializer_list<T> list)
{
    if (mData != nullptr) {
        SharedBuffer* sb = SharedBuffer::GetBufferFromData(mData);
        if (sb->OnlyOwner()) {
            DeleteFunc<T> deleteF;
            for (Long i = 0;  i < mSize;  i++) {
                if (nullptr != &static_cast<T*>(mData)[i]) {
                    deleteF(&static_cast<T*>(mData)[i], this);
                }
            }
        }
        sb->Release();
        mData = nullptr;
        mSize = 0;
    }

    if (Alloc(list.size())) {
        Long i;
        typename std::initializer_list<T>::const_iterator it;
        for (it = list.begin(), i = 0;  it != list.end();  ++it, ++i) {
            Set(i, *it);
        }
    }
    return *this;
}

template<typename T>
T& Array<T>::operator[](
    /* [in] */ Long index)
{
    T* array = static_cast<T*>(mData);

    // TODO, fix bug
    // Can not throw exceptions, how to inform the caller parameter error
    if ((index < 0) || (index >= mSize)) {
        Logger::E("Array<T>::operator[]",
                  "E_ILLEGAL_ARGUMENT_EXCEPTION, index: %ld", index);
        return array[0];
    }

    return array[index];
}

template<typename T>
const T& Array<T>::operator[](
    /* [in] */ Long index) const
{
    T* array = static_cast<T*>(mData);

    // TODO, fix bug
    // Can not throw exceptions, how to inform the caller parameter error
    if ((index < 0) || (index >= mSize)) {
        Logger::E("Array<T>::operator[]",
                  "E_ILLEGAL_ARGUMENT_EXCEPTION, index: %ld", index);
        return array[0];
    }

    return array[index];
}

template<typename T>
T* Array<T>::begin()
{
    return static_cast<T*>(mData);
}

template<typename T>
const T* Array<T>::begin() const
{
    return static_cast<T*>(mData);
}

template<typename T>
T* Array<T>::end()
{
    return static_cast<T*>(mData) + mSize;
}

template<typename T>
const T* Array<T>::end() const
{
    return static_cast<T*>(mData) + mSize;
}

template<typename T>
void Array<T>::Clear()
{
    if (mData != nullptr) {
        SharedBuffer* sb = SharedBuffer::GetBufferFromData(mData);
        if (sb->OnlyOwner()) {
            DeleteFunc<T> deleteF;
            for (Long i = 0;  i < mSize;  i++) {
                if (nullptr != &static_cast<T*>(mData)[i]) {
                    deleteF(&static_cast<T*>(mData)[i], this);
                }
            }
        }
        sb->Release();
        mData = nullptr;
    }
    mSize = 0;
}

template<typename T>
Array<T> Array<T>::Clone() const
{
    Array<T> newArray;

    if (! newArray.Alloc(mSize)) {
        return newArray;
    }

    T* src = static_cast<T*>(mData);
    T* des = static_cast<T*>(newArray.mData);
    for (Long i = 0;  i < mSize;  i++) {
        AssignFunc<T> assignF;
        assignF(&des[i], src[i], des);
    }

    return newArray;
}

template<typename T>
Array<T> Array<T>::Allocate(
    /* [in] */ Long size)
{
    return Array<T>(size);
}

template<typename T>
Array<T> Array<T>::Null()
{
    return Array<T>();
}

template<typename T, Boolean isIInterfaceSubclass>
struct ConvertImpl
{
    Array<IInterface*> operator()(
        /* [in] */ Array<T>* lvalue)
    {
        return Array<IInterface*>::Null();
    }
};

template<typename T>
struct ConvertImpl<T, true>
{
    Array<IInterface*> operator()(
        /* [in] */ Array<T>* lvalue)
    {
        return *reinterpret_cast<Array<IInterface*>*>(lvalue);
    }
};

template<typename T>
Array<T>::operator Array<IInterface*>()
{
    typedef typename TypeTraits<T>::BareType BareType;
    ConvertImpl<T, SUPERSUBCLASS(IInterface, BareType)> impl;
    return impl(this);
}

} // namespace como

#endif //__COMO_ARRAY_H__
