//=========================================================================
// Copyright (C) 2024 The C++ Component Model(COMO) Open Source Project
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

#include <comosp.h>
#include <comoobj.h>
#include <ThreadStack.h>
#include <gtest/gtest.h>
#include <iostream>

using namespace como;
using namespace std;

TEST(ThreadStack, testThreadStack)
{
    void *addr;
    size_t size;

    int ret = ThreadStack::GetStack(&addr, &size);
    EXPECT_EQ(ret, 0);
    printf("GetStack addr: %p  size: %lx\n", addr, size);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
