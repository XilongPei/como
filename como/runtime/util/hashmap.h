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

#ifndef __COMO_HASHMAP_H__
#define __COMO_HASHMAP_H__

#include "comoarray.h"
#include <cstdlib>
#include <cstring>
#include <assert.h>

namespace como {

using HashMapCacheCmp2PVoid = Boolean(*)(void*,void*);

static const unsigned int prime_list[11] =
{
    5ul,    11ul,   23ul,   53ul,   97ul,   193ul,
    389ul,  769ul,  1543ul, 3079ul, 6151ul
};

static unsigned int get_lower_bound(const unsigned int* first, const unsigned int* last, int n)
{
    if (n <= *first) {
        return *first;
    }
    if (n >= *last) {
        return *last;
    }
    for (unsigned int i = 0;  first + i != last;  i++) {
        unsigned int l = *(first + i);
        unsigned int r = *(first + i + 1);
        if ((l <= n) && (n < r)) {
            return (n - l) < (r - n) ? l : r;
        }
    }
    return *last;
}

inline unsigned int get_next_prime(int n)
{
    return get_lower_bound(&prime_list[0], &prime_list[10], n);
}

template<typename Key, typename Val>
class HashMap
{
private:
    struct Bucket
    {
        Bucket()
            : mNext(nullptr)
        {
            InitFunc<Key> initKeyF;
            InitFunc<Val> initValF;
            initKeyF(&mKey, this);
            initValF(&mValue, this);
        }

        ~Bucket()
        {
            DeleteFunc<Key> deleteKeyF;
            DeleteFunc<Val> deleteValF;
            deleteKeyF(&mKey, this);
            deleteValF(&mValue, this);
            mNext = nullptr;
        }

        // Even if the hash value calculated in HashFunc is negative, it is forced
        // to be positive when assigned to unsigned long
        unsigned long mHash;
        Key mKey;
        Val mValue;
        struct Bucket* mNext;
    };

public:

    // Walker for struct Bucket
    using HashMapWalker = void(*)(String&,Key&,Val&);

    explicit HashMap(
        /* [in] */ unsigned int size = 50u)
        : mCount(0u)
    {
        mBucketSize = get_next_prime(size);
        mBuckets = static_cast<Bucket**>(calloc(sizeof(Bucket*), mBucketSize));
        if (nullptr == mBuckets) {
            mBucketSize = 0u;
        }
        mThreshold = (unsigned int)((double)mBucketSize * LOAD_FACTOR);
    }

    ~HashMap()
    {
        for (unsigned int i = 0u;  i < mBucketSize;  i++) {
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

    int Put(
        /* [in] */ const Key& key,
        /* [in] */ Val value)
    {
        CompareFunc<Key> compareF;
        AssignFunc<Key> assignKeyF;
        AssignFunc<Val> assignValF;

        if (mCount >= mThreshold) {
            Rehash();
        }

        unsigned long hash = HashKey(key);
        unsigned int index = hash % mBucketSize;
        if (mBuckets[index] == nullptr) {
            Bucket* b = new Bucket();
            if (nullptr == b) {
                return -2;
            }

            b->mHash = hash;
            assignKeyF(&b->mKey, key, this);
            assignValF(&b->mValue, value, this);
            mBuckets[index] = b;
            mCount++;
            return 0;
        }
        else {
            Bucket* prev = mBuckets[index];
            while (prev != nullptr) {
                if (! compareF(prev->mKey, key)) {
                    assignValF(&prev->mValue, value, this);
                    return 0;
                }
                else if (prev->mNext == nullptr) {
                    break;
                }
                prev = prev->mNext;
            }

            if (nullptr == prev) {
                return -3;
            }

            Bucket* b = new Bucket();
            if (nullptr == b) {
                return -2;
            }

            b->mHash = hash;
            assignKeyF(&b->mKey, key, this);
            assignValF(&b->mValue, value, this);
            prev->mNext = b;
            mCount++;
            return 0;
        }
    }

    bool ContainsKey(
        /* [in] */ const Key& key)
    {
        CompareFunc<Key> compareF;

        unsigned long hash = HashKey(key);
        unsigned int index = hash % mBucketSize;
        Bucket* curr = mBuckets[index];
        while (curr != nullptr) {
            if ((curr->mHash == hash) && (! compareF(curr->mKey, key))) {
                return true;
            }
            curr = curr->mNext;
        }

        return false;
    }

    Val Get(
        /* [in] */ const Key& key)
    {
        CompareFunc<Key> compareF;

        unsigned long hash = HashKey(key);
        unsigned int index = hash % mBucketSize;
        Bucket* curr = mBuckets[index];
        while (curr != nullptr) {
            if ((curr->mHash == hash) && (! compareF(curr->mKey, key))) {
                return curr->mValue;
            }
            curr = curr->mNext;
        }

        return Val(0);
    }

    int Remove(
        /* [in] */ const Key& key)
    {
        CompareFunc<Key> compareF;

        unsigned long hash = HashKey(key);
        unsigned int index = hash % mBucketSize;
        Bucket* curr = mBuckets[index];
        Bucket* prev = curr;
        while (curr != nullptr) {
            if ((curr->mHash == hash) && (! compareF(curr->mKey, key))) {
                if (curr == mBuckets[index]) {
                    mBuckets[index] = curr->mNext;
                }
                else {
                    prev->mNext = curr->mNext;
                }
                delete curr;
                mCount--;
                return 1;
            }
            prev = curr;
            curr = prev->mNext;
        }

        return 0;
    }

    void Clear()
    {
        for (unsigned int i = 0u;  i < mBucketSize;  i++) {
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
        mCount = 0;
    }

    Val GetValue(HashMapCacheCmp2PVoid fun, void* param)
    {
        for (unsigned int i = 0u;  i < mBucketSize;  i++) {
            if (mBuckets[i] != nullptr) {
                Bucket* curr = mBuckets[i];
                while (curr != nullptr) {
                    if (fun(curr->mValue, param)) {
                        return curr->mValue;
                    }
                    curr = curr->mNext;
                }
            }
        }
        return Val(nullptr);
    }

    Array<Val> GetValues()
    {
        Long N = 0;
        for (unsigned int i = 0u; i < mBucketSize; i++) {
            if (mBuckets[i] != nullptr) {
                Bucket* curr = mBuckets[i];
                while (curr != nullptr) {
                    N++;
                    curr = curr->mNext;
                }
            }
        }
        assert(mCount == N);

        Array<Val> values(N);
        if (values.IsNull()) {
            return values;
        }

        if (N > 0) {
            Long idx = 0;
            for (unsigned int i = 0;  i < mBucketSize;  i++) {
                if (mBuckets[i] != nullptr) {
                    Bucket* curr = mBuckets[i];
                    while (curr != nullptr) {
                        values.Set(idx, curr->mValue);
                        idx++;
                        curr = curr->mNext;
                    }
                }
            }
        }
        return values;
    }

    void GetValues(String& str, HashMapWalker walker)
    {
        for (unsigned int i = 0u;  i < mBucketSize;  i++) {
            if (mBuckets[i] != nullptr) {
                Bucket* curr = mBuckets[i];
                while (curr != nullptr) {
                    walker(str, curr->mKey, curr->mValue);
                    curr = curr->mNext;
                }
            }
        }
    }

    unsigned int GetDataNumber()
    {
        return mCount;
    }

private:
    unsigned long HashKey(
        /* [in] */ const Key& key)
    {
        HashFunc<Key> hashF;
        return hashF(key);
    }

    void Rehash()
    {
        if (mBucketSize == MAX_BUCKET_SIZE) {
            return;
        }
        unsigned int oldBucketSize = mBucketSize;
        mBucketSize = oldBucketSize * 2u + 1u;
        if (mBucketSize > MAX_BUCKET_SIZE || mBucketSize < 0) {
            mBucketSize = MAX_BUCKET_SIZE;
        }
        Bucket** newBuckets = static_cast<Bucket**>(calloc(sizeof(Bucket*), mBucketSize));
        if (newBuckets == nullptr) {
            return;
        }

        for (unsigned int i = 0u;  i < oldBucketSize;  i++) {
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
        mThreshold = (unsigned int)((double)mBucketSize * LOAD_FACTOR);
    }

    void PutBucket(
        /* [in] */ Bucket** buckets,
        /* [in] */ unsigned int bucketSize,
        /* [in] */ Bucket* bucket)
    {
        unsigned int index = bucket->mHash % bucketSize;
        if (buckets[index] == nullptr) {
            buckets[index] = bucket;
        }
        else {
            Bucket* prev = buckets[index];
            while (prev->mNext != nullptr) {
                prev = prev->mNext;
            }
            prev->mNext = bucket;
        }
    }

private:
    static constexpr float LOAD_FACTOR = 0.75;
    static constexpr int MAX_BUCKET_SIZE = INT32_MAX - 8;
    unsigned int mThreshold;
    unsigned int mCount;
    unsigned int mBucketSize;
    Bucket     **mBuckets;
};

} // namespace como

#endif // __COMO_HASHMAP_H__
