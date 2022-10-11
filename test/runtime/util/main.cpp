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

#define __IN_COMOTEST__

#include <comosp.h>
#include <comoobj.h>
#include <MemPool.h>
#include <gtest/gtest.h>

using namespace como;

TEST(MemPool, TestMemPool1)
{
    CMemPoolSet::MemPoolItem items[] = {{10, 64, nullptr}, {10, 128, nullptr}, {10, 256, nullptr}};
    CMemPoolSet *memPoolSet = new CMemPoolSet(items, 3);
    EXPECT_NE(memPoolSet, nullptr);

    // std::cout << memPoolSet << std::endl;

    void *p;
    for (int i = 0;  i < 100;  i++) {
        p = memPoolSet->Alloc(28, MUST_USE_MEM_POOL);
        if (i < 20) {
            EXPECT_NE(p, nullptr);
        }

        if ((i % 2) == 0) {
            bool b = memPoolSet->Free(p);
            if (i < 20) {
                EXPECT_EQ(b, true);
            }
        }
    }

    bool b = memPoolSet->Free(p);
    EXPECT_EQ(b, false);
}


// TEST(MemPool, TestMemPool2)
// {
//     CMemPoolSet::MemPoolItem items[] = {{10, 64, nullptr}, {10, 128, nullptr}, {10, 256, nullptr}};
//     CMemPoolSet *memPoolSet = new CMemPoolSet(items, 3);
//     EXPECT_NE(memPoolSet, nullptr);

//     // std::cout << memPoolSet << std::endl;

//     void *p;
//     for (int i = 0;  i < 100;  i++) {
//         p = memPoolSet->Alloc(28, MUST_USE_MEM_POOL);
//         if(i < 10) EXPECT_NE(nullptr, p);
//         else EXPECT_EQ(nullptr, p);
//     }

// }

TEST(MemPool, TestMemPool2)
{
    CMemPoolSet::MemPoolItem items[] = {{10, 64, nullptr}, {10, 128, nullptr}, {10, 256, nullptr}};
    CMemPoolSet *memPoolSet = new CMemPoolSet(items, 3);
    EXPECT_NE(memPoolSet, nullptr);

    // std::cout << memPoolSet << std::endl;

    void *p;
    for (int i = 0;  i < 100;  i++) {
        if (i % 3 == 0)
            p = memPoolSet->Alloc(64, MUST_USE_MEM_POOL);
        else if (i % 3 == 1)
            p = memPoolSet->Alloc(128, MUST_USE_MEM_POOL);
        else
            p = memPoolSet->Alloc(256, MUST_USE_MEM_POOL);

        if (i < 30) {
            EXPECT_NE(nullptr, p);
        }
        else {
            EXPECT_EQ(nullptr, p);
        }
    }

}


TEST(MemPool, TestMemPool3)
{
    CMemPoolSet::MemPoolItem items[] = {{100, 64, nullptr}};
    CMemPoolSet *memPoolSet = new CMemPoolSet(items, 1);
    EXPECT_NE(nullptr, memPoolSet);
    void *p;
    for (int i = 0; i < 150; i++) {
        p = memPoolSet->Alloc(64, MUST_USE_MEM_POOL);
        if (i % 2 == 0) {
            bool b = memPoolSet->Free(p);
            EXPECT_EQ(true, b);
            // b = memPoolSet->Free(p);
            // EXPECT_NE(true, b);
        }
    }

    bool b1 = memPoolSet->m_MemPoolSet[0].memPool->CheckFull();
    EXPECT_EQ(false, b1);

    for(int i = 0; i < 50; i++) {
        p = memPoolSet->Alloc(64, MUST_USE_MEM_POOL);
        if (i < 25) {
            EXPECT_NE(nullptr, p);
        }
        else {
            EXPECT_EQ(nullptr, p);
        }
    }
    b1 = memPoolSet->m_MemPoolSet[0].memPool->CheckFull();
    EXPECT_EQ(true, b1);
}


int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
