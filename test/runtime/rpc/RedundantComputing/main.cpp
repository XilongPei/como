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

#include "RPCTestUnit.h"
#include <stdio.h>
#include <comoapi.h>
#include <comosp.h>
#include <ServiceManager.h>
#include <rpcHelpers.h>
#include <gtest/gtest.h>
#include "ComoConfig.h"
#include "CZMQUtils.h"

using como::test::rpc::CID_CService;
using como::test::rpc::IService;
using como::test::rpc::ITestInterface;
using jing::ServiceManager;
using jing::RpcHelpers;

static AutoPtr<IService> SERVICE;

TEST(RedundantComputing, TestGetRPCService)
{
    AutoPtr<IInterface> obj;
    ServiceManager::GetInstance()->GetRemoteService("ServiceManager", "rpcserviceZMQ", obj);
    SERVICE = IService::Probe(obj);
    EXPECT_TRUE(SERVICE != nullptr);
}

TEST(RedundantComputing, TestCMD_Client_Activate_InvokeMethod)
{
    EXPECT_TRUE(SERVICE != nullptr);
    IProxy* proxy = IProxy::Probe(SERVICE);
    EXPECT_TRUE(proxy != nullptr);

    void *publisher = CZMQUtils::CzmqGetPubSocket(nullptr, "tcp://127.0.0.1:8086");

    ECode ec = proxy->SetPubSocket(reinterpret_cast<HANDLE>(publisher));
    EXPECT_EQ(NOERROR, ec);

    ec = E_REMOTE_EXCEPTION;
    AutoPtr<ITestInterface> obj;
    ec = SERVICE->TestMethod4(obj);
    EXPECT_EQ(ec, NOERROR);
    EXPECT_TRUE(obj != nullptr);
    EXPECT_TRUE(IProxy::Probe(obj) != nullptr);
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
