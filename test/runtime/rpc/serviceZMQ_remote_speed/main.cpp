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
#include <cstdio>
#include <unistd.h>
#include <string>
#include "ComoConfig.h"
#include "comorpc.h"
#include "CZMQUtils.h"

using como::test::rpc::CService;
using como::test::rpc::IService;
using como::test::rpc::IID_IService;
using jing::ServiceManager;

int main(int argc, char** argv)
{
    std::string localhost_addr = "tcp://127.0.0.1:8083";
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

    std::string name;
    std::string endpoint;
    void *socket;

    std::map<std::string, ServerNodeInfo*>::iterator iter =
               ComoConfig::ServerNameEndpointMap.find(std::string("localhost"));
    if (iter != ComoConfig::ServerNameEndpointMap.end()) {
        name = iter->first;
        endpoint = iter->second->endpoint;
    }
    else {
        Logger::E("CZMQChannel", "Hasn't register any servers");
        return 0;
    }
    ECode ec = CService::New(IID_IService, (IInterface**)&srv);
    if (FAILED(ec)) {
        printf("Failed, CService::New\n");
        return 1;
    }

    #if 0
        ECode ServiceManager::AddRemoteService(
            /* [in] */ const String& thisServerName,
            /* [in] */ const String& thatServerName,
            /* [in] */ const String& name,
            /* [in] */ IInterface* object)
    #endif
    AutoPtr<ServiceManager> instance = ServiceManager::GetInstance();
    instance->AddRemoteService(String("localhost"),
                               String("ServiceManager"),
                               String("rpcserviceZMQ"), srv);
    printf("RPC serviceZMQ waiting for calling endpoint: %s\n", endpoint.c_str());

    //endpoint += "192.168.0.8:1239";   // RedundantComputing port

    CZMQUtils::CzmqProxy(nullptr, endpoint.c_str(), nullptr);

    if (FAILED(instance->WaitForRuntimeToEnd())) {
        while (true) {
            printf(".");
            sleep(5);
        }
        printf("\n");
    }

    return 0;
}
