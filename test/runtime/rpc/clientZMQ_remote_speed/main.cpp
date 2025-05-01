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
#include <stdio.h>
#include <time.h>

using como::test::rpc::CID_CService;
using como::test::rpc::IService;
using como::test::rpc::ITestInterface;
using jing::ServiceManager;
using jing::RpcHelpers;

static AutoPtr<IService> SERVICE;
struct timespec start_time, end_time;
const int CNT = 1000000;
const int PERIOD_MS = 10;  // period, ms

TEST(ClientZmqTest, TestGetRPCService)
{
    AutoPtr<IInterface> obj;

    // struct timespec sleep_time;
    // sleep_time.tv_sec = PERIOD_MS / 1000;
    // sleep_time.tv_nsec = (PERIOD_MS % 1000) * 1000000;  // ms->ns

    // find and get service
    ServiceManager::GetInstance()->GetRemoteService("ServiceManager", "rpcserviceZMQ", obj);
    SERVICE = IService::Probe(obj);
    ASSERT_TRUE(SERVICE != nullptr);

    // testmethod params
    ECode ec = E_REMOTE_EXCEPTION;
    Integer result;
    ec = SERVICE->TestMethod1(9, result);
    EXPECT_EQ(9, result);
    EXPECT_EQ(ec, NOERROR);

    // for (int i = 0; i < CNT; ++i) {
    //     struct timespec start_time, end_time;
    
    //     // count waste time
    //     clock_gettime(CLOCK_REALTIME, &start_time);
    //     ec = SERVICE->TestMethod1(9, result);
    //     clock_gettime(CLOCK_REALTIME, &end_time);
    
    //     // compute waste time
    //     uint64_t duration = (end_time.tv_sec - start_time.tv_sec) * 1000000
    //                       + (end_time.tv_nsec - start_time.tv_nsec) / 1000;
    
    //     printf("Run %d: GetService time = %llu microseconds\n", i + 1, (unsigned long long)duration);

    //     // sleep to next period
    //     // nanosleep(&sleep_time, NULL);
    // }
    
}

int main(int argc, char **argv)
{
    std::string localhost_addr = "tcp://127.0.0.1:8085";
    std::string service_addr = "tcp://127.0.0.1:8082";
    std::string ServiceManager_addr = "tcp://127.0.0.1:8088";
    AutoPtr<IService> srv;

    if(argc >= 2) {
        localhost_addr = "tcp://" + std::string(argv[1]);
    }
    if(argc >= 3) {
        service_addr = "tcp://" + std::string(argv[2]);
    }
    if(argc == 4) {
        ServiceManager_addr = "tcp://" + std::string(argv[3]);
    }
    if(argc > 4) {
        printf("parameters error\n");
        return -1;
    }

    printf("the address of localhost: %s,\n"
        "the address of service: %s,\n"
        "the address of ServiceManager: %s\n",
        localhost_addr.c_str(), service_addr.c_str(), ServiceManager_addr.c_str());

    std::string ret = ComoConfig::AddZeroMQEndpoint(std::string("localhost"), localhost_addr);
    if (std::string("localhost") != ret) {
        printf("Failed to AddZeroMQEndpoint\n");
    }

    ret = ComoConfig::AddZeroMQEndpoint(std::string("Service1"), service_addr);
    if (std::string("Service1") != ret) {
        printf("Failed to AddZeroMQEndpoint\n");
    }

    ret = ComoConfig::AddZeroMQEndpoint(std::string("ServiceManager"), ServiceManager_addr);
    if (std::string("ServiceManager") != ret) {
        printf("Failed to AddZeroMQEndpoint\n");
    }

//    Logger::SetLevel(0);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
