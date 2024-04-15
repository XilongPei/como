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

#include "RPCTestUnit2.h"
#include <comoapi.h>
#include <comosp.h>
#include <ServiceManager.h>
#include <gtest/gtest.h>

using como::test::rpc::CID_CService2;
using como::test::rpc::IService2;
using como::test::rpc::TestEnum;
using jing::ServiceManager;

static AutoPtr<IService2> SERVICE;

TEST(RPCTest, TestGetRPCService)
{
    AutoPtr<IInterface> obj;
    ServiceManager::GetInstance()->GetService("rpcservice2", obj);
    SERVICE = IService2::Probe(obj);
    EXPECT_TRUE(SERVICE != nullptr);
}

TEST(RPCTest, TestCallTestMethod1)
{
    EXPECT_TRUE(SERVICE != nullptr);
    Array<Integer> arg1({1, 2, 3, 4, 5, 6, 7});
    Long size = arg1.GetLength();

    Array<Integer>* result1 = new Array<Integer>(size);
    ECode ec = SERVICE->TestMethod1(arg1, result1);
    EXPECT_EQ(0, ec);

    for(int i = 0; i < size; i++)
        EXPECT_EQ((*result1)[i], arg1[size - i - 1]);

    delete result1;
}

TEST(RPCTest, TestCallTestMethod2)
{
    EXPECT_TRUE(SERVICE != nullptr);
    Integer arg1 = 0x61626364; /* a,b,c,d */
    Char result1, result2, result3, result4;
    ECode ec = SERVICE->TestMethod2(arg1, &result1, &result2, &result3, &result4);
    EXPECT_EQ(0, ec);
    if (result1 == 'd') {
        EXPECT_EQ((char)result1, 'd');
        EXPECT_EQ((char)result2, 'c');
        EXPECT_EQ((char)result3, 'b');
        EXPECT_EQ((char)result4, 'a');
    }
    else {
        EXPECT_EQ((char)result1, 'a');
        EXPECT_EQ((char)result2, 'b');
        EXPECT_EQ((char)result3, 'c');
        EXPECT_EQ((char)result4, 'd');
    }
}

TEST(RPCTest, TestCallTestMethod3)
{
    EXPECT_TRUE(SERVICE != nullptr);
    TestEnum arg1;
    String result1;
    arg1 = TestEnum::Monday;
    ECode ec = SERVICE->TestMethod3(arg1, result1);
    EXPECT_EQ(0, ec);
    EXPECT_STREQ(result1.string(), "Monday");
    arg1 = TestEnum::Sunday;
    ec = SERVICE->TestMethod3(arg1, result1);
    EXPECT_EQ(0, ec);
    EXPECT_STREQ(result1.string(), "Sunday");
}

TEST(RPCTest, TestCallTestMethod4)
{
    EXPECT_TRUE(SERVICE != nullptr);
    Triple arg1;
    String arg2 = "TestStr";
    Char* result1;
    Long arg3 = 10, result2;
    TypeKind arg4 = TypeKind::Byte, result3;
    arg1.mData = (void*)arg2.string();
    arg1.mSize = arg3;
    arg1.mType = arg4;

    ECode ec = SERVICE->TestMethod4(arg1, result1, result2, result3);
    EXPECT_EQ(0, ec);
    EXPECT_STREQ(arg2.string(), (char*)result1);
    EXPECT_EQ(arg3, result2);
    EXPECT_EQ(arg4, result3);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
