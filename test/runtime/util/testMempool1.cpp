//=========================================================================
// Copyright (C) 2022 The C++ Component Model(COMO) Open Source Project
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

// Some test problems:
// 1. TryToUseMemPool::DONT_USE_MEM_POOL isn't used in MemPool.cpp.
// 2. When CMemPoolSet object is deleted, g_MemPool、m_g_MemPoolSet could not be freed.
// 3. In CMemPoolSet::Alloc line 372，what is the meaning of input "ulUnitNum=ulElemSize, ulUnitSize=m_g_lUnitSize/ulElemSize"？

#define __IN_COMOTEST__

#include <comosp.h>
#include <comoobj.h>
#include <MemPool.h>
#include <gtest/gtest.h>

using namespace como;

// Test 1: CMemPool constructor
TEST(CMemPool, TestMemPool1)
{
    const size_t unitNum = 10;
    const size_t unitSize = 64;
    CMemPool *memPool1, *memPool2;

    memPool1 = new CMemPool();
    EXPECT_NE(nullptr, memPool1);
    memPool2 = new CMemPool(nullptr, unitNum, unitSize);
    EXPECT_NE(nullptr, memPool2);

    delete memPool1;
    delete memPool2;
}

// Test 2.1: CMemPool::Alloc, CMemPool::Free (Mempool is null)
TEST(CMemPool, TestMemPool2_1)
{
    const size_t unitNum = 10;
    const size_t unitSize = 64;
    void *p;
    CMemPool *memPool;

    memPool = new CMemPool();
    ASSERT_NE(nullptr, memPool);
    p = memPool->Alloc(unitSize, MUST_USE_MEM_POOL);
    EXPECT_EQ(nullptr, p);
    memPool->Free(p);

    p = memPool->Alloc(unitSize, TRY_TO_USE_MEM_POOL);
    EXPECT_NE(nullptr, p);
    memPool->Free(p);

    delete memPool;
}

// Test 2.2: CMemPool::Alloc, CMemPool::Free (Different memory allocation)
TEST(CMemPool, TestMemPool2_2)
{
    const size_t unitNum = 10;
    const size_t unitSize = 64; 
    const size_t smallerAllocSize = 32;
    const size_t largerAllocSize = 128;
    void *p;
    CMemPool *memPool;

    memPool = new CMemPool(nullptr, unitNum, unitSize);
    ASSERT_NE(nullptr, memPool);
    p = memPool->Alloc(smallerAllocSize, MUST_USE_MEM_POOL);
    EXPECT_NE(nullptr, p);
    memPool->Free(p);

    p = memPool->Alloc(smallerAllocSize, TRY_TO_USE_MEM_POOL);
    EXPECT_NE(nullptr, p);
    memPool->Free(p);

    p = memPool->Alloc(unitSize, MUST_USE_MEM_POOL);
    EXPECT_NE(nullptr, p);
    memPool->Free(p);

    p = memPool->Alloc(unitSize, TRY_TO_USE_MEM_POOL);
    EXPECT_NE(nullptr, p);
    memPool->Free(p);

    p = memPool->Alloc(largerAllocSize, MUST_USE_MEM_POOL);
    EXPECT_EQ(nullptr, p);
    memPool->Free(p);

    p = memPool->Alloc(largerAllocSize, TRY_TO_USE_MEM_POOL);
    EXPECT_NE(nullptr, p);
    memPool->Free(p);

    delete memPool;
}

// Test2.3: CMemPool::Alloc, CMemPool::Free (Mempool is full)
TEST(CMemPool, TestMemPool2_3)
{
    const size_t unitNum = 10;
    const size_t unitSize = 64;
    void *p;
    CMemPool *memPool;

    memPool = new CMemPool(nullptr, unitNum, unitSize);
    ASSERT_NE(nullptr, memPool);
    for (int i = 0;  i < 20;  i++) {
        p = memPool->Alloc(unitSize, MUST_USE_MEM_POOL);
        if (i < 10) {               // Mempool is not full.
            EXPECT_NE(nullptr, p);
        }
        else {                      // Mempool is full.
            EXPECT_EQ(nullptr, p);
        }
    }

    p = memPool->Alloc(unitSize, TRY_TO_USE_MEM_POOL);
    EXPECT_NE(nullptr, p);
    memPool->Free(p);

    delete memPool;
}

// Test2.4: CMemPool::Alloc, CMemPool::Free （Alloc and Free call combination）
TEST(CMemPool, TestMemPool2_4)
{
    const size_t unitNum = 10;
    const size_t unitSize = 64;
    void *p;
    CMemPool *memPool;

    // Alloc->Alloc->Free->Alloc->Alloc->Free->...
    memPool = new CMemPool(nullptr, unitNum, unitSize);
    ASSERT_NE(nullptr, memPool);
    for (int i = 0;  i < 30;  i++) {
        p = memPool->Alloc(unitSize, MUST_USE_MEM_POOL);
        if (i < 20) {
            EXPECT_NE(nullptr, p);

            if (i % 2 == 0) {
                memPool->Free(p);
            }
        }
        else {
            EXPECT_EQ(nullptr, p);
        }
    }

    delete memPool;
}

// Test3: CMemPool::CheckExist
TEST(CMemPool, TestMemPool3)
{
    const size_t unitNum = 10;
    const size_t unitSize = 64;
    const size_t largerAllocSize = 128;
    void *p;
    bool b;
    CMemPool *memPool;

    memPool = new CMemPool(nullptr, unitNum, unitSize);
    ASSERT_NE(nullptr, memPool);
    p = memPool->Alloc(unitSize, MUST_USE_MEM_POOL);
    b = memPool->CheckExist(p);
    EXPECT_EQ(true, b);
    memPool->Free(p);

    p = memPool->Alloc(largerAllocSize, MUST_USE_MEM_POOL);
    b = memPool->CheckExist(p);
    EXPECT_EQ(false, b);
    memPool->Free(p);

    delete memPool;
}

// Test4: CMemPool::CheckFull
TEST(CMemPool, TestMemPool4)
{
    const size_t unitNum = 10;
    const size_t unitSize = 64;
    void *p;
    bool b;
    CMemPool *memPool;

    // Mempool is null.
    memPool = new CMemPool();
    ASSERT_NE(nullptr, memPool);
    b = memPool->CheckFull();
    EXPECT_EQ(true, b);
    
    delete memPool;

    // Mempool is not null.
    memPool = new CMemPool(nullptr, unitNum, unitSize);
    ASSERT_NE(nullptr, memPool);
    for (int i = 0;  i < 10;  i++) {
        p = memPool->Alloc(unitSize, MUST_USE_MEM_POOL);
        b = memPool->CheckFull();
        if (i < 9) {
            EXPECT_EQ(false, b);
        }
        else {
            EXPECT_EQ(true, b);
        }
    }

    delete memPool;
}

// Test5.1: CMemPool::Setbuffer
TEST(CMemPool, TestMemPool5)
{
    const size_t unitNum = 10;
    const size_t unitSize = 64;
    const size_t bufferSize = unitNum * (unitSize + 16);        // Size of buffer. 16 is size of struct _Unit.
    void *buffer = ::malloc(bufferSize);                        // Existing buffer
    void *p;
    bool b;
    CMemPool *memPool;

    // Don't use buffer.
    memPool = new CMemPool(nullptr, unitNum, unitSize);
    ASSERT_NE(nullptr, memPool);
    p = memPool->Alloc(unitSize, MUST_USE_MEM_POOL);
    EXPECT_NE(nullptr, p);
    b = (buffer < p) && (p < (void *)((char *)buffer + bufferSize));
    EXPECT_NE(true, b);
    memPool->Free(p);
    
    delete memPool;

    // Use buffer.
    memPool = new CMemPool(buffer, unitNum, unitSize);
    ASSERT_NE(nullptr, memPool);
    p = memPool->Alloc(unitSize, MUST_USE_MEM_POOL);
    EXPECT_NE(nullptr, p);
    b = (buffer < p) && (p < (void *)((char *)buffer + bufferSize));
    EXPECT_EQ(true, b);
    memPool->Free(p);
    
    delete memPool;

    ::free(buffer);
}

// Test6: CMemPoolSet constructor
TEST(CMemPoolSet, TestMemPool6)
{
    const size_t unitNum = 10;
    const size_t unitSize = 64;
    const size_t g_unitNum = 10;
    const size_t g_unitSize = 64;
    CMemPoolSet::MemPoolItem item = {unitNum, unitSize, nullptr};
    CMemPoolSet *memPoolSet1, *memPoolSet2;

    memPoolSet1 = new CMemPoolSet(&item, 1);
    EXPECT_NE(nullptr, memPoolSet1);
    memPoolSet2 = new CMemPoolSet(&item, 1, g_unitNum, g_unitSize);
    EXPECT_NE(nullptr, memPoolSet2);

    delete memPoolSet1;
    delete memPoolSet2;
}

// Test7.1: CMemPoolSet::Alloc (Different memory allocation)
TEST(CMemPoolSet, TestMemPool7_1)
{
    const size_t unitNum1 = 10;
    const size_t unitSize1 = 64;
    const size_t unitNum2 = 10;
    const size_t unitSize2 = 128;
    void *p;
    bool b;
    CMemPoolSet::MemPoolItem items[] = {{unitNum1, unitSize1, nullptr}, {unitNum2, unitSize2, nullptr}};
    CMemPoolSet *memPoolSet = new CMemPoolSet(items, 2);
    ASSERT_NE(memPoolSet, nullptr);

    size_t allocSize = 16;
    for (int i = 0;  i < 10;  i++) {
        p = memPoolSet->Alloc(allocSize, MUST_USE_MEM_POOL);

        if (allocSize <= unitSize1) {   // Allocate memory from mempool1
            EXPECT_NE(nullptr, p);
            b = memPoolSet->m_MemPoolSet[0].memPool->CheckExist(p);
            EXPECT_EQ(true, b);
            b = memPoolSet->m_MemPoolSet[1].memPool->CheckExist(p);
            EXPECT_EQ(false, b);
        }
        else if (allocSize <= unitSize2) {   // Allocate memory from mempool2
            EXPECT_NE(nullptr, p);
            b = memPoolSet->m_MemPoolSet[0].memPool->CheckExist(p);
            EXPECT_EQ(false, b);
            b = memPoolSet->m_MemPoolSet[1].memPool->CheckExist(p);
            EXPECT_EQ(true, b);
        }
        else {     // Allocation size is too large.
            EXPECT_EQ(nullptr, p);
        }

        allocSize += 16;
    }

    delete memPoolSet;
}

// Test7.2: CMemPoolSet::Alloc（Mempool set is full）
TEST(CMemPoolSet, TestMemPool7_2)
{
    const size_t unitNum1 = 10;
    const size_t unitSize1 = 64;
    const size_t unitNum2 = 10;
    const size_t unitSize2 = 128;
    const size_t unitNum3 = 10;
    const size_t unitSize3 = 256;
    void *p;
    CMemPoolSet::MemPoolItem items[] = {{unitNum1, unitSize1, nullptr}, {unitNum2, unitSize2, nullptr}, {unitNum3, unitSize3, nullptr}};
    CMemPoolSet *memPoolSet = new CMemPoolSet(items, 3);
    ASSERT_NE(memPoolSet, nullptr);

    for (int i = 0;  i < 40;  i++) {
        if (i % 3 == 0)
            p = memPoolSet->Alloc(unitSize1, MUST_USE_MEM_POOL);
        else if (i % 3 == 1)
            p = memPoolSet->Alloc(unitSize2, MUST_USE_MEM_POOL);
        else
            p = memPoolSet->Alloc(unitSize3, MUST_USE_MEM_POOL);

        if (i < 30) {
            EXPECT_NE(nullptr, p);
        }
        else {
            EXPECT_EQ(nullptr, p);
        }
    }

    delete memPoolSet;
}

// Test8.1: CMemPoolSet::Free （Different mempool）
TEST(CMemPoolSet, TestMemPool8_1)
{
    const size_t unitNum1 = 10;
    const size_t unitSize1 = 64;
    const size_t unitNum2 = 10;
    const size_t unitSize2 = 128;
    const size_t unitNum3 = 10;
    const size_t unitSize3 = 256;
    void *p[16] = {0};
    bool b;
    CMemPoolSet::MemPoolItem items[] = {{unitNum1, unitSize1, nullptr}, {unitNum2, unitSize2, nullptr}, {unitNum3, unitSize3, nullptr}};
    CMemPoolSet *memPoolSet = new CMemPoolSet(items, 3);
    ASSERT_NE(memPoolSet, nullptr);

    size_t allocSize = 16;
    for (int i = 0;  i < 16;  i++) {
        p[i] = memPoolSet->Alloc(allocSize, MUST_USE_MEM_POOL);
        EXPECT_NE(nullptr, p);
        allocSize += 16;
    }

    for (int i = 0;  i < 16;  i++) {
        b = memPoolSet->Free(p[i]);
        EXPECT_EQ(true, b);
    }

    delete memPoolSet;
}

// Test8.2: CMemPoolSet::Free （Alloc and Free call combination）
TEST(CMemPoolSet, TestMemPool8_2)
{
    const size_t unitNum = 10;
    const size_t unitSize = 64;
    void *p;
    bool b;
    CMemPoolSet::MemPoolItem items[] = {{unitNum, unitSize, nullptr}};
    CMemPoolSet *memPoolSet = new CMemPoolSet(items, 1);
    ASSERT_NE(memPoolSet, nullptr);

    // Alloc->Alloc->Free->Alloc->Alloc->Free->...
    for (int i = 0;  i < 30;  i++) {
        p = memPoolSet->Alloc(unitSize, MUST_USE_MEM_POOL);
        if (i < 20) {
            EXPECT_NE(nullptr, p);
        }
        else {
            EXPECT_EQ(nullptr, p);
        }
        
        if (i % 2 == 0) {
            b = memPoolSet->Free(p);
            if (i < 20) {
                EXPECT_EQ(true, b);
            }
            else {
                EXPECT_EQ(false, b);
            }
        }
    }

    delete memPoolSet;
}

// Test9: CMemPoolSet::CreateGeneralMemPool
TEST(CMemPoolSet, TestMemPool9)
{
    const size_t unitNum = 10;
    const size_t unitSize = 64;
    const size_t g_unitNum = 3;
    const size_t g_unitSize = unitSize * unitSize;
    void *p;
    bool b;
    CMemPoolSet::MemPoolItem items[] = {{unitNum, unitSize, nullptr}};
    CMemPoolSet *memPoolSet = new CMemPoolSet(items, 1);
    ASSERT_NE(memPoolSet, nullptr);

    // g_unitNum = 0
    b = memPoolSet->CreateGeneralMemPool(0, g_unitSize);
    EXPECT_EQ(false, b);

    // g_unitSize = 0
    b = memPoolSet->CreateGeneralMemPool(g_unitNum, 0);
    EXPECT_EQ(false, b);

    // Normal condition
    b = memPoolSet->CreateGeneralMemPool(g_unitNum, g_unitSize);
    ASSERT_EQ(true, b);

    for (int i = 0;  i < 20;  i++) {
        p = memPoolSet->Alloc(unitSize, MUST_USE_MEM_POOL);
        EXPECT_NE(nullptr, p);
    }

    delete memPoolSet;
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
