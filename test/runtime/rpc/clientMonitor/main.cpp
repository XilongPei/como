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

#include "RPCTestUnit.h"
#include <comoapi.h>
#include <comosp.h>
#include <ServiceManager.h>
#include <rpcHelpers.h>
#include <gtest/gtest.h>
#include "ComoConfig.h"
#include "RuntimeMonitor.h"
#include <stdio.h>

using como::test::rpc::CID_CService;
using como::test::rpc::IService;
using como::test::rpc::ITestInterface;
using jing::ServiceManager;
using jing::RpcHelpers;

static AutoPtr<IService> SERVICE;

TEST(ClientMonitor, TestGetRPCService)
{
    AutoPtr<IInterface> obj;
    ServiceManager::GetInstance()->GetRemoteService("ServiceManager", "rpcserviceZMQ", obj);
    SERVICE = IService::Probe(obj);
    EXPECT_TRUE(SERVICE != nullptr);
}

TEST(ClientMonitor, TestCallTestMethod4)
{
    EXPECT_TRUE(SERVICE != nullptr);
    ECode ec = E_REMOTE_EXCEPTION;
    AutoPtr<ITestInterface> obj;
    ec = SERVICE->TestMethod4(obj);
    EXPECT_EQ(ec, NOERROR);
    EXPECT_TRUE(obj != nullptr);
    EXPECT_TRUE(IProxy::Probe(obj) != nullptr);
}

TEST(ClientMonitor, TestMonitorRuntime)
{
    EXPECT_TRUE(SERVICE != nullptr);
    IProxy* proxy = IProxy::Probe(SERVICE);
    EXPECT_TRUE(proxy != nullptr);

    RTM_Command *rtmCommand = RuntimeMonitor::GenRtmCommand(
                                              RTM_CommandType::CMD_BY_STRING, 0,
                                                (const char *)"ExportObject=1");

    Array<Byte> response;
    Array<Byte> request(rtmCommand->length);
    request.Copy((Byte*)rtmCommand, rtmCommand->length);
    free(rtmCommand);

    ECode ec = proxy->MonitorRuntime(request, response);
    printf("Monitor Response:\n%s\n", response.GetPayload());
    EXPECT_EQ(0, ec);
}

TEST(ClientMonitor, TestCMD_Server_InvokeMethod)
{
    EXPECT_TRUE(SERVICE != nullptr);
    IProxy* proxy = IProxy::Probe(SERVICE);
    EXPECT_TRUE(proxy != nullptr);

    RTM_Command *rtmCommand = RuntimeMonitor::GenRtmCommand(
                                    RTM_CommandType::CMD_Server_InvokeMethod, 0,
                                                              (const char *)"");
    EXPECT_NE(nullptr, rtmCommand);

    Array<Byte> response;
    Array<Byte> request(rtmCommand->length);
    request.Copy((Byte*)rtmCommand, rtmCommand->length);
    free(rtmCommand);

    ECode ec = proxy->MonitorRuntime(request, response);
/*
typedef struct tagRTM_InvokeMethod {
    Long            length;             // total length of this struct
    struct timespec time;
    UUID            coclassID_mUuid;
    UUID            interfaceID_mUuid;
    Long            serverObjectId;
    Integer         methodIndexPlus4;
    Integer         in_out;             // 0: in; 1: out
    Byte            parcel[0];          // from here, Byte *parcel;
} RTM_InvokeMethod;
*/
    RTM_InvokeMethod *rtmInvokeMethod = (RTM_InvokeMethod*)response.GetPayload();
    EXPECT_NE(nullptr, rtmInvokeMethod);

    RuntimeMonitor::DeserializeRtmInvokeMethod(rtmInvokeMethod);

    String str = DumpUUID(rtmInvokeMethod->coclassID.mUuid);
    printf("    TestCMD_Server_InvokeMethod: RTM_InvokeMethod coclassID_mUuid: %s\n", str.string());

    str = DumpUUID(rtmInvokeMethod->coclassID.mCid->mUuid);
    printf("    TestCMD_Server_InvokeMethod: RTM_InvokeMethod ComponentID mUuid: %s\n", str.string());
    printf("    TestCMD_Server_InvokeMethod: RTM_InvokeMethod ComponentID mUri: %s\n",
                                                            rtmInvokeMethod->coclassID.mCid->mUri);

    String strBuffer;
    RuntimeMonitor::DumpRtmInvokeMethod(rtmInvokeMethod, strBuffer);
    printf("DumpRtmInvokeMethod: %s\n", strBuffer.string());

    EXPECT_EQ(0, ec);
}

TEST(ClientMonitor, TestCMD_Client_InvokeMethod)
{
    EXPECT_TRUE(SERVICE != nullptr);
    IProxy* proxy = IProxy::Probe(SERVICE);
    EXPECT_TRUE(proxy != nullptr);

    RTM_Command *rtmCommand = RuntimeMonitor::GenRtmCommand(
                                    RTM_CommandType::CMD_Client_InvokeMethod, 0,
                                                              (const char *)"");
    Array<Byte> response;
    Array<Byte> request(rtmCommand->length);
    request.Copy((Byte*)rtmCommand, rtmCommand->length);
    free(rtmCommand);

    ECode ec = proxy->MonitorRuntime(request, response);

    RTM_InvokeMethod *rtmInvokeMethod = (RTM_InvokeMethod*)response.GetPayload();
    EXPECT_NE(nullptr, rtmInvokeMethod);

    RuntimeMonitor::DeserializeRtmInvokeMethod(rtmInvokeMethod);

    String str = DumpUUID(rtmInvokeMethod->coclassID.mUuid);
    printf("    TestCMD_Client_InvokeMethod, RTM_InvokeMethod coclassID_mUuid: %s\n", str.string());

    str = DumpUUID(rtmInvokeMethod->coclassID.mCid->mUuid);
    printf("    TestCMD_Client_InvokeMethod, RTM_InvokeMethod ComponentID mUuid: %s\n", str.string());
    printf("    TestCMD_Client_InvokeMethod, RTM_InvokeMethod ComponentID mUri: %s\n",
                                                            rtmInvokeMethod->coclassID.mCid->mUri);

    EXPECT_EQ(0, ec);
}

int main(int argc, char **argv)
{
    std::string ret = ComoConfig::AddZeroMQEndpoint(std::string("localhost"),
                                            std::string("tcp://127.0.0.1:8085"));
    if (std::string("localhost") != ret) {
        printf("Failed to AddZeroMQEndpoint\n");
    }

    ret = ComoConfig::AddZeroMQEndpoint(std::string("ServiceManager"),
                                            std::string("tcp://127.0.0.1:8088"));
    if (std::string("ServiceManager") != ret) {
        printf("Failed to AddZeroMQEndpoint\n");
    }

//    Logger::SetLevel(0);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
