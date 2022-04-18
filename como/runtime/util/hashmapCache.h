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

#ifndef __COMO_HASHMAP_CACHE_H__
#define __COMO_HASHMAP_CACHE_H__

#include <cstdlib>
#include <cstring>
#include <assert.h>
#include "ComoConfig.h"
#include "type/comoarray.h"
#include "hashmap.h"

namespace como {

template<typename Key, typename Val>
class HashMapCache
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

        unsigned long mHash;
        Key mKey;
        Val mValue;
        struct timespec lastAccessTime;

        struct Bucket* mNext;
    };

public:

    // Walker for struct Bucket
    using HashMapWalker = void(*)(String&,Key,Val);

    HashMapCache(
        /* [in] */ unsigned int size = 50)
        : mCount(0)
    {
        mBucketSize = get_next_prime(size);
        mBuckets = (Bucket**)calloc(sizeof(Bucket*), mBucketSize);
        if (nullptr == mBuckets)
            mBucketSize = 0;
        mThreshold = mBucketSize * LOAD_FACTOR;
        clock_gettime(CLOCK_REALTIME, &mLastCheanTime);
    }

    ~HashMapCache()
    {
        for (unsigned int i = 0;  i < mBucketSize;  i++) {
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
            if (nullptr == b)
                return -2;

            b->mHash = hash;
            assignKeyF(&b->mKey, key, this);
            assignValF(&b->mValue, value, this);
            mBuckets[index] = b;
            mCount++;
            clock_gettime(CLOCK_REALTIME, &(b->lastAccessTime));
            return 0;
        }
        else {
            Bucket* prev = mBuckets[index];
            while (prev != nullptr) {
                if (!compareF(prev->mKey, key)) {
                    assignValF(&prev->mValue, value, this);
                    return 0;
                }
                else if (prev->mNext == nullptr) {
                    break;
                }
                prev = prev->mNext;
            }
            Bucket* b = new Bucket();
            if (nullptr == b)
                return -2;

            b->mHash = hash;
            assignKeyF(&b->mKey, key, this);
            assignValF(&b->mValue, value, this);
            prev->mNext = b;
            mCount++;
            clock_gettime(CLOCK_REALTIME, &(b->lastAccessTime));
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
            if (curr->mHash == hash && !compareF(curr->mKey, key)) {
                clock_gettime(CLOCK_REALTIME, &(curr->lastAccessTime));
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
            if (curr->mHash == hash && !compareF(curr->mKey, key)) {
                clock_gettime(CLOCK_REALTIME, &(curr->lastAccessTime));
                return curr->mValue;
            }
            curr = curr->mNext;
        }

        return Val(0);
    }

    void Remove(
        /* [in] */ const Key& key)
    {
        CompareFunc<Key> compareF;

        unsigned long hash = HashKey(key);
        unsigned int index = hash % mBucketSize;
        Bucket* curr = mBuckets[index];
        Bucket* prev = curr;
        while (curr != nullptr) {
            if (curr->mHash == hash && !compareF(curr->mKey, key)) {
                if (curr == mBuckets[index]) {
                    mBuckets[index] = curr->mNext;
                }
                else {
                    prev->mNext = curr->mNext;
                }
                curr->lastAccessTime.tv_sec = 0;
                return;
            }
            prev = curr;
            curr = prev->mNext;
        }
    }

    void Remove(
        /* [in] */ unsigned long hash)
    {
        unsigned int index = hash % mBucketSize;
        Bucket* curr = mBuckets[index];
        Bucket* prev = curr;
        while (curr != nullptr) {
            if (curr->mHash == hash) {
                if (curr == mBuckets[index]) {
                    mBuckets[index] = curr->mNext;
                }
                else {
                    prev->mNext = curr->mNext;
                }
                curr->lastAccessTime.tv_sec = 0;
                return;
            }
            prev = curr;
            curr = prev->mNext;
        }
    }

    void Clear()
    {
        for (unsigned int i = 0;  i < mBucketSize;  i++) {
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

    Array<Val> GetValues()
    {
        Long N = 0;
        for (unsigned int i = 0;  i < mBucketSize;  i++) {
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
        if (values.IsNull())
            return values;

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
        for (unsigned int i = 0;  i < mBucketSize;  i++) {
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

    void CleanUpExpiredData()
    {
        struct timespec currentTime;
        clock_gettime(CLOCK_REALTIME, &currentTime);

        if ((currentTime.tv_sec - mLastCheanTime.tv_sec) < CHECK_EXPIRES_PERIOD)
            return;

        mLastCheanTime = currentTime;

        for (unsigned int i = 0;  i < mBucketSize;  i++) {
            if (mBuckets[i] != nullptr) {
                Bucket* curr = mBuckets[i];
                while (curr != nullptr) {
                    if ((currentTime.tv_sec - curr->lastAccessTime.tv_sec) >
                                                               TIMEOUT_BUCKET) {
                        Bucket* next = curr->mNext;
                        delete curr;
                        mCount--;
                        curr = next;
                    }
                    curr = curr->mNext;
                }
            }
        }
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
        mBucketSize = oldBucketSize * 2 + 1;
        if (mBucketSize > MAX_BUCKET_SIZE || mBucketSize < 0) {
            mBucketSize = MAX_BUCKET_SIZE;
        }
        Bucket** newBuckets = (Bucket**)calloc(sizeof(Bucket*), mBucketSize);
        if (newBuckets == nullptr) {
            mBucketSize = oldBucketSize;
            return;
        }
        for (unsigned int i = 0; i < oldBucketSize; i++) {
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
    static constexpr int CHECK_EXPIRES_PERIOD = 300;
    static constexpr int TIMEOUT_BUCKET = 600;

    unsigned int mThreshold;
    unsigned int mCount;
    unsigned int mBucketSize;
    Bucket** mBuckets;
    struct timespec mLastCheanTime;
};

} // namespace como

#endif // __COMO_HASHMAP_CACHE_H__
