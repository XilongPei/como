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

#include "RPCTestUnit.h"
#include <comoapi.h>
#include <comosp.h>
#include <ServiceManager.h>
#include <rpcHelpers.h>
#include <gtest/gtest.h>
#include <stdio.h>
#include <time.h>
#include "debugbreak.h"

using como::test::rpc::CID_CService;
using como::test::rpc::IService;
using como::test::rpc::ITestInterface;
using jing::ServiceManager;
using jing::RpcHelpers;

static AutoPtr<IService> SERVICE;
struct timespec start_time, end_time;

TEST(RPCTest, TestGetRPCService)
{
    AutoPtr<IInterface> obj;
    // count waste time
    clock_gettime(CLOCK_REALTIME, &start_time);
    ServiceManager::GetInstance()->GetService("rpcservice", obj);
    clock_gettime(CLOCK_REALTIME, &end_time);

    // computing waste time
    uint64_t duration = (end_time.tv_sec - start_time.tv_sec) * 1000000
                        + (end_time.tv_nsec - start_time.tv_nsec) / 1000;
    printf("GetService time: %llu microseconds\n", (unsigned long long)duration);

    SERVICE = IService::Probe(obj);
    EXPECT_TRUE(SERVICE != nullptr);
}

TEST(RPCTest, TestCallTestMethod1)
{
    ASSERT_TRUE(SERVICE != nullptr);
    ECode ec = E_REMOTE_EXCEPTION;
    Integer result;
    ec = SERVICE->TestMethod1(9, result);
    EXPECT_EQ(9, result);
    EXPECT_EQ(ec, NOERROR);
}

TEST(RPCTest, TestCallTestMethod2)
{
    ASSERT_TRUE(SERVICE != nullptr);
    Integer arg1 = 9, result1;
    Long arg2 = 99, result2;
    Boolean arg3 = true, result3;
    Char arg4 = U'C', result4;
    Short arg5 = 999, result5;
    Double arg6 = 9.9, result6;
    Float arg7 = 9.99, result7;
    Integer arg8 = 999, result8;
    Float arg9 = 99.9, result9;
    Double arg10 = 9.009, result10;
    Double arg11 = 0.009, result11;
    Float arg12 = 9.09, result12;
    Float arg13 = 0.09, result13;
    Double arg14 = 99.009, result14;
    Double arg15 = -999.009, result15;
    Float arg16 = -0.09, result16;
    ECode ec = E_REMOTE_EXCEPTION;
    ec = SERVICE->TestMethod2(
            arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8,
            arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16,
            result1, result2, result3, result4, result5, result6, result7, result8,
            result9, result10, result11, result12, result13, result14, result15, result16);
    EXPECT_EQ(ec, NOERROR);
    EXPECT_EQ(arg1, result1);
    EXPECT_EQ(arg2, result2);
    EXPECT_EQ(arg3, result3);
    EXPECT_EQ(arg4, result4);
    EXPECT_EQ(arg5, result5);
    EXPECT_DOUBLE_EQ(arg6, result6);
    EXPECT_FLOAT_EQ(arg7, result7);
    EXPECT_EQ(arg8, result8);
    EXPECT_FLOAT_EQ(arg9, result9);
    EXPECT_DOUBLE_EQ(arg10, result10);
    EXPECT_DOUBLE_EQ(arg11, result11);
    EXPECT_FLOAT_EQ(arg12, result12);
    EXPECT_FLOAT_EQ(arg13, result13);
    EXPECT_DOUBLE_EQ(arg14, result14);
    EXPECT_DOUBLE_EQ(arg15, result15);
    EXPECT_FLOAT_EQ(arg16, result16);
}

TEST(RPCTest, TestCallTestMethod3)
{
    ASSERT_TRUE(SERVICE != nullptr);
    ECode ec = E_REMOTE_EXCEPTION;
    Integer arg1 = 9;
    Integer result1;
    String arg2 = "Hello World!";
    String result2;
    ec = SERVICE->TestMethod3(arg1, arg2, result1, result2);
    EXPECT_EQ(ec, NOERROR);
    EXPECT_EQ(arg1, result1);
    EXPECT_STREQ(arg2.string(), result2.string());
}

TEST(RPCTest, TestCallTestMethod4)
{
    ASSERT_TRUE(SERVICE != nullptr);
    ECode ec = E_REMOTE_EXCEPTION;
    AutoPtr<ITestInterface> obj;
    ec = SERVICE->TestMethod4(obj);
    EXPECT_EQ(ec, NOERROR);
    EXPECT_TRUE(obj != nullptr);
    EXPECT_TRUE(IProxy::Probe(obj) != nullptr);

    Integer arg1 = 6;
    Integer result1;
    obj->TestMethod1(arg1, result1);
    EXPECT_EQ(result1, arg1 + 3);
    AutoPtr<IMetaCoclass> mk;
    IProxy::Probe(obj)->GetTargetCoclass(mk);
    EXPECT_TRUE(mk != nullptr);
    AutoPtr<IMetaComponent> mc;
    mk->GetComponent(mc);
    EXPECT_TRUE(mc != nullptr);
    Boolean onlyMetadata;
    mc->IsOnlyMetadata(onlyMetadata);
    EXPECT_TRUE(onlyMetadata);

    //
    // It does not implement the COMO class of IParcelable interface.
    // The method execution is on the service server side.
    //
    // Probe(obj)
    IProxy* proxy = IProxy::Probe(obj);
    EXPECT_TRUE(proxy != nullptr);

    Long uuidOrder;
    proxy->GetUuidOrder(uuidOrder);
    printf("== uuidOrder == %llX\n", uuidOrder);

    AutoPtr<como::IInterfacePack> ipack;
    ec = proxy->GetIpack(ipack);
    EXPECT_EQ(0, ec);

    Long hash;
    ipack->GetServerObjectId(hash);

    //debug_break();
    ec = proxy->ReleaseObject(hash);
    EXPECT_EQ(0, ec);
}

TEST(RPCTest, TestCallTestMethod5)
{
    ASSERT_TRUE(SERVICE != nullptr);
    ECode ec = E_REMOTE_EXCEPTION;
    AutoPtr<ITestInterface> obj;
    ec = SERVICE->TestMethod5(obj);
    EXPECT_EQ(ec, NOERROR);
    EXPECT_TRUE(obj != nullptr);
    EXPECT_TRUE(IProxy::Probe(obj) == nullptr);

    Integer arg1 = 6;
    Integer result1;
    obj->TestMethod1(arg1, result1);
    EXPECT_EQ(result1, arg1 + 6);
    Integer value;
    obj->GetValue(value);
    EXPECT_EQ(value, 23);
    AutoPtr<IMetaCoclass> mk;
    IObject::Probe(obj)->GetCoclass(mk);
    EXPECT_TRUE(mk != nullptr);
    AutoPtr<IMetaComponent> mc;
    mk->GetComponent(mc);
    EXPECT_TRUE(mc != nullptr);
    Boolean onlyMetadata;
    mc->IsOnlyMetadata(onlyMetadata);
    EXPECT_FALSE(onlyMetadata);

    //
    // It implements the COMO class of IParcelable interface. Its method
    // execution is executed on the client side.
    //
    // Probe(SERVICE)
    IProxy* proxy = IProxy::Probe(SERVICE);
    EXPECT_TRUE(proxy != nullptr);
    Long hash;
    IObject::Probe(obj)->GetHashCode(hash);
    ec = proxy->ReleaseObject(hash);
    EXPECT_NE(0, ec);
}

TEST(RPCTest, TestIsStubAlive)
{
    ASSERT_TRUE(SERVICE != nullptr);
    IProxy* proxy = IProxy::Probe(SERVICE);
    EXPECT_TRUE(proxy != nullptr);
    Boolean alive;
    Long channelState = 1239;
    proxy->IsStubAlive(channelState, alive);
    EXPECT_TRUE(alive);
}

TEST(RPCTest, TestReleaseObject)
{
    ASSERT_TRUE(SERVICE != nullptr);
    IProxy* proxy = IProxy::Probe(SERVICE);
    EXPECT_TRUE(proxy != nullptr);

    Long hash;
    AutoPtr<IInterfacePack> ipack;
    proxy->GetIpack(ipack);
    ipack->GetServerObjectId(hash);

    // free SERVICE's IStub
    ECode ec = proxy->ReleaseObject(hash);
    EXPECT_EQ(0, ec);
}

TEST(RPCTest, TestRpcHelpers)
{
    ASSERT_TRUE(SERVICE != nullptr);
    ECode ec = E_REMOTE_EXCEPTION;
    AutoPtr<ITestInterface> obj;
    ec = SERVICE->TestMethod4(obj);
    EXPECT_EQ(ec, NOERROR);
    EXPECT_TRUE(obj != nullptr);
    EXPECT_TRUE(IProxy::Probe(obj) != nullptr);

    ec = RpcHelpers::ReleaseRemoteObject(SERVICE, obj);
    EXPECT_EQ(0, ec);

    ec = RpcHelpers::ReleaseImportObject(SERVICE, obj);
    EXPECT_EQ(0, ec);
}

/*
TEST(RPCTest, TestReleaseStub)
{
    EXPECT_TRUE(SERVICE != nullptr);
    IProxy* proxy = IProxy::Probe(SERVICE);
    EXPECT_TRUE(proxy != nullptr);
    Boolean alive;
    proxy->ReleaseStub(alive);
    EXPECT_TRUE(alive);

    ECode ec = ServiceManager::GetInstance()->RemoveService(String("rpcservice"));
    EXPECT_TRUE(SUCCEEDED(ec));

}
*/
int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
