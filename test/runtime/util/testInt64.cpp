//=========================================================================
// Copyright (C) 2023 The C++ Component Model(COMO) Open Source Project
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
#include <gtest/gtest.h>
#include <int64.h>

using namespace como;

TEST(Int64, TestInt64)
{
    Int64 i64;
    long long int ll = 4800;

    i64.i64Val = 0;
    i64.i32.i32_low = 1239;

    EXPECT_EQ(8, sizeof(Int64));
    EXPECT_EQ(1239, i64.i64Val);
    EXPECT_EQ(4800, (*(Int64*)&ll).i64Val);
    //printf("%ld %lld %lld\n", sizeof(Int64), i64.i64Val, (*(Int64*)&ll).i64Val);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
