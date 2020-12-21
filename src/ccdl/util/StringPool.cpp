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

#include "Logger.h"
#include "StringPool.h"

namespace ccdl {

template<>
StringMap<char*>::~StringMap<char*>()
{
    for (int i = 0; i < mBucketSize; i++) {
        if (mBuckets[i] != nullptr) {
            Bucket* curr = mBuckets[i];
            while (curr != nullptr) {
                Bucket* next = curr->mNext;
                free(curr->mValue);
                delete curr;
                curr = next;
            }
        }
    }
}

StringPool::StringPool()
    : mData(nullptr)
    , mDataCapacity(256)
    , mDataOffset(0)
    , mOffsets(3000)
{
    mData = (char*)calloc(1, mDataCapacity);
    Add(String(""));
}

StringPool::~StringPool()
{
    if (mData != nullptr) {
        free(mData);
    }
}

void StringPool::Add(
    /* [in] */ const String& string)
{
    if (string.IsNull() || mOffsets.ContainsKey(string)) {
        return;
    }

    ptrdiff_t offset = AddInternal(string);
    if (offset == -1) return;
    mOffsets.Put(string, offset);
}

char* StringPool::FindAddress(
    /* [in] */ const String& string)
{
    if (!mOffsets.ContainsKey(string)) return nullptr;

    ptrdiff_t offset = mOffsets.Get(string);
    return mData + offset;
}

ptrdiff_t StringPool::FindOffset(
    /* [in] */ const String& string)
{
    return mOffsets.Get(string);
}

ptrdiff_t StringPool::AddInternal(
    /* [in] */ const String& string)
{
    if (!EnsureCapacity(string.GetLength() + 1)) {
        return -1;
    }

    char* target = mData + mDataOffset;
    strcpy(target, string.string());
    mDataOffset += string.GetLength() + 1;
    return target - mData;
}

bool StringPool::EnsureCapacity(
    /* [in] */ size_t expand)
{
    int newSize = mDataOffset + expand;

    if (newSize < mDataCapacity) {
        return true;
    }

    newSize = 10 * mDataCapacity > newSize ?
            10 * mDataCapacity : 10 * mDataCapacity + newSize;
    char* newData = (char*)calloc(1, newSize);
    if (newData == nullptr) {
        Logger::E("StringPool", "Out of memory.");
        return false;
    }
    memcpy(newData, mData, mDataOffset);
    free(mData);
    mData = newData;
    mDataCapacity = newSize;
    return true;
}

void StringPool::Dump()
{
    int i = 1;
    char* begin = mData;
    while ((begin - mData) <= mDataOffset - 1) {
        Logger::D("StringPool", "[%d] %s", i++, begin);
        begin = strchr(begin, '\0') + 1;
    }
}

}
