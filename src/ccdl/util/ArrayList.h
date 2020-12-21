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

#ifndef __CCDL_ARRAYLIST_H__
#define __CCDL_ARRAYLIST_H__

#include <stdlib.h>

namespace ccdl {

template<class T>
class ArrayList
{
public:
    ArrayList(
        /* [in] */ int initCapacity = 10,
        /* [in] */ bool release = true)
        : mCapacity(initCapacity)
        , mIndex(0)
        , mElements(nullptr)
        , mRelease(release)
    {
        if (mCapacity <= 0) mCapacity = 10;
        mElements = (T*)calloc(sizeof(T), mCapacity);
    }

    ~ArrayList()
    {
        if (mRelease) {
            for (int i = 0; i < mIndex; i++) {
                T data = mElements[i];
                delete data;
            }
        }
        if (mElements != nullptr) free(mElements);
        mElements = nullptr;
    }

    bool Add(
        /* [in] */ T data)
    {
        if (!EnsureCapacity()) return false;

        mElements[mIndex++] = data;
        return true;
    }

    bool Remove(
        /* [in] */ T data)
    {
        int i = 0;
        while (mElements[i] != data) {
            i++;
        }
        if (i >= mIndex) return false;
        for (; i < mIndex - 1; i++) {
            mElements[i] = mElements[i + 1];
        }
        mElements[mIndex - 1] = nullptr;
        mIndex--;
        if (mRelease) {
            delete data;
        }
        return true;
    }

    T Get(
        /* [in] */ int index)
    {
        if (index < 0 || index >= mIndex) return T(0);
        return mElements[index];
    }

    int GetSize()
    {
        return mIndex;
    }

    int IndexOf(
        /* [in] */ T data)
    {
        for (int i = 0; i < mIndex; i++) {
            if (mElements[i] == data) return i;
        }
        return -1;
    }

    void Clear(
        /* [in] */ bool release)
    {
        for (int i = 0; i < mIndex; i++) {
            if (release && mRelease) {
                T data = mElements[i];
                delete data;
            }
            mElements[i] = nullptr;
        }
        mIndex = 0;
    }

private:
    bool EnsureCapacity()
    {
        if (mIndex < mCapacity) return true;

        int newSize = mCapacity * 2;
        T* newArray = (T*)calloc(sizeof(T), newSize);
        if (newArray == nullptr) return false;

        memcpy(newArray, mElements, sizeof(T) * mCapacity);
        free(mElements);
        mElements = newArray;
        mCapacity = newSize;
        return true;
    }

private:
    int mCapacity;
    int mIndex;
    T* mElements;
    bool mRelease;
};

}

#endif // __CCDL_ARRAYLIST_H__
