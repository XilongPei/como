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

#ifndef __CCDL_STRINGMAP_H__
#define __CCDL_STRINGMAP_H__

#include "ArrayList.h"
#include "String.h"

#include <stdint.h>
#include <stdlib.h>
#include <memory>

namespace ccdl {

static const int prime_list[11] =
{
    5ul,    11ul,   23ul,   53ul,   97ul,   193ul,
    389ul,  769ul,  1543ul, 3079ul, 6151ul
};

static int get_lower_bound(const int* first, const int* last, int n)
{
    if (n < *first) return *first;
    if (n > *last) return *last;
    for (int i = 0; first + i != last; i++) {
        int l = *(first + i);
        int r = *(first + i + 1);
        if (l <= n && n < r) {
            return (n - l) < (r - n) ? l : r;
        }
    }
}

inline int get_next_prime(int n)
{
    return get_lower_bound(&prime_list[0], &prime_list[10], n);
}

template<class T>
class StringMap
{
public:
    struct Pair
    {
        String mKey;
        T mValue;
    };

private:
    struct Bucket
    {
        Bucket()
            : mNext(nullptr)
        {}

        ~Bucket()
        {
            mNext = nullptr;
        }

        int mHash;
        String mKey;
        T mValue;
        struct Bucket* mNext;
    };

public:
    StringMap(
        /* [in] */ int size = 50)
        : mDataSize(0)
        , mKeyValues(nullptr)
        , mInvalidate(true)
    {
        mBucketSize = get_next_prime(size);
        mBuckets = (Bucket**)calloc(sizeof(Bucket*), mBucketSize);
        mThreshold = mBucketSize * LOAD_FACTOR;
    }

    ~StringMap()
    {
        for (int i = 0; i < mBucketSize; i++) {
            if (mBuckets[i] != nullptr) {
                Bucket* curr = mBuckets[i];
                while (curr != nullptr) {
                    Bucket* next = curr->mNext;
                    delete curr;
                    curr = next;
                }
                mBuckets[i] = nullptr;
            }
        }
        free(mBuckets);
    }

    void Put(
        /* [in] */ const String& key,
        /* [in] */ T value)
    {
        if (key.IsNull()) return;
        if (mDataSize >= mThreshold) {
            Rehash();
        }
        int hash = HashString(key);
        int index = hash % mBucketSize;
        if (mBuckets[index] == nullptr) {
            Bucket* b = new Bucket();
            b->mHash = hash;
            b->mKey = key;
            b->mValue = value;
            mBuckets[index] = b;
            mDataSize++;
            mInvalidate = true;
            return;
        }
        else {
            Bucket* prev = mBuckets[index];
            while (prev != nullptr) {
                if (prev->mKey.Equals(key)) {
                    prev->mValue = value;
                    mInvalidate = true;
                    return;
                }
                else if (prev->mNext == nullptr) {
                    break;
                }
                prev = prev->mNext;
            }
            Bucket* b = new Bucket();
            b->mHash = hash;
            b->mKey = key;
            b->mValue = value;
            prev->mNext = b;
            mDataSize++;
            mInvalidate = true;
            return;
        }
    }

    bool ContainsKey(
        /* [in] */ const String& key)
    {
        if (key.IsNull()) return false;

        int hash = HashString(key);
        int index = hash % mBucketSize;
        Bucket* curr = mBuckets[index];
        while (curr != nullptr) {
            if (curr->mHash == hash && curr->mKey.Equals(key)) return true;
            curr = curr->mNext;
        }

        return false;
    }

    T Get(
        /* [in] */ const String& key)
    {
        if (key.IsNull()) {
            return T(0);
        }

        int hash = HashString(key);
        int index = hash % mBucketSize;
        Bucket* curr = mBuckets[index];
        while (curr != nullptr) {
            if (curr->mHash == hash && curr->mKey.Equals(key)) return curr->mValue;
            curr = curr->mNext;
        }

        return T(0);
    }

    int GetSize()
    {
        return mDataSize;
    }

    std::shared_ptr< ArrayList<Pair*> > GetKeyValues()
    {
        if (!mInvalidate) return mKeyValues;

        mKeyValues = std::make_shared< ArrayList<Pair*> >(mDataSize);

        for (int i = 0; i < mBucketSize; i++) {
            Bucket* curr = mBuckets[i];
            while (curr != nullptr) {
                Pair* p = new Pair();
                p->mKey = curr->mKey;
                p->mValue = curr->mValue;
                mKeyValues->Add(p);

                curr = curr->mNext;
            }
        }

        mInvalidate = false;
        return mKeyValues;
    }

    void Clear()
    {
        for (int i = 0; i < mBucketSize; i++) {
            if (mBuckets[i] != nullptr) {
                Bucket* curr = mBuckets[i];
                while (curr != nullptr) {
                    Bucket* next = curr->mNext;
                    delete curr;
                    curr = next;
                }
                mBuckets[i] = nullptr;
            }
        }
    }

private:
    int HashString(
        /* [in] */ const String& key)
    {
        // BKDR Hash Function
        int seed = 31; // 31 131 1313 13131 131313 etc..
        unsigned int hash = 0;

        const char* string = key.string();
        if (string) {
            for ( ; *string; ++string) {
                hash = hash * seed + (*string);
            }
        }
        return (hash & 0x7FFFFFFF);
    }

    void Rehash()
    {
        if (mBucketSize == MAX_BUCKET_SIZE) {
            return;
        }
        int oldBucketSize = mBucketSize;
        mBucketSize = oldBucketSize * 2 + 1;
        if (mBucketSize > MAX_BUCKET_SIZE || mBucketSize < 0) {
            mBucketSize = MAX_BUCKET_SIZE;
        }
        Bucket** newBuckets = (Bucket**)calloc(sizeof(Bucket*), mBucketSize);
        if (newBuckets == nullptr) {
            return;
        }
        for (int i = 0; i < oldBucketSize; i++) {
            Bucket* curr = mBuckets[i];
            while (curr != nullptr) {
                Bucket* next = curr->mNext;
                curr->mNext = nullptr;
                PutBucket(newBuckets, mBucketSize, curr);
                curr = next;
            }
        }
        free(mBuckets);
        mBuckets = newBuckets;
        mThreshold = mBucketSize * LOAD_FACTOR;
    }

    void PutBucket(
        /* [in] */ Bucket** buckets,
        /* [in] */ int bucketSize,
        /* [in] */ Bucket* bucket)
    {
        int index = bucket->mHash % bucketSize;
        if (buckets[index] == nullptr) {
            buckets[index] = bucket;
            return;
        }
        else {
            Bucket* prev = buckets[index];
            while (prev->mNext != nullptr) {
                prev = prev->mNext;
            }
            prev->mNext = bucket;
            return;
        }
    }

private:
    static constexpr float LOAD_FACTOR = 0.75;
    static constexpr int MAX_BUCKET_SIZE = INT32_MAX - 8;
    int mThreshold;
    int mDataSize;
    int mBucketSize;
    Bucket** mBuckets;
    std::shared_ptr< ArrayList<Pair*> > mKeyValues;
    bool mInvalidate;
};

}

#endif // __CCDL_STRINGMAP_H__
