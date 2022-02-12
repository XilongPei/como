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

#include <comosp.h>
#include <comoobj.h>
#include <gtest/gtest.h>
#include <string.h>

using namespace como;

TEST(StringTest, TestStringReplace)
{
    String str1("tongji");
    String str2("::university::college");
    String str3("university::college::");
    String str4("tongji::university::college");

    EXPECT_STREQ("tongji", str1.Replace("::", ".").string());
    EXPECT_STREQ(".university.college", str2.Replace("::", ".").string());
    EXPECT_STREQ("university.college.", str3.Replace("::", ".").string());
    EXPECT_STREQ("tongji.university.college", str4.Replace("::", ".").string());
}

TEST(StringTest, TestReserve)
{
    String str("tongji");

    EXPECT_EQ(str.Reserve(4096), NOERROR);
    EXPECT_EQ(str.GetCapacity(), 4096);

    str += " university";
    EXPECT_EQ(str.GetCapacity(), 4096);
    EXPECT_EQ(str.GetByteLength(), strlen("tongji university"));
    EXPECT_STREQ("tongji university", str.string());
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
