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
#include "ComoConfig.h"
#include "comorpc.h"
#include "CZMQUtils.h"

using como::test::rpc::CService;
using como::test::rpc::IService;
using como::test::rpc::IID_IService;
using jing::ServiceManager;

int main(int argv, char** argc)
{
    AutoPtr<IService> srv;

    std::string ret = ComoConfig::AddZeroMQEndpoint(std::string("localhost"),
                                            std::string("tcp://127.0.0.1:8083"));
    if (std::string("localhost") != ret) {
        printf("Failed to AddZeroMQEndpoint\n");
    }

    ret = ComoConfig::AddZeroMQEndpoint(std::string("Service1"),
                                            std::string("tcp://127.0.0.1:8082"));
    if (std::string("Service1") != ret) {
        printf("Failed to AddZeroMQEndpoint\n");
    }

    ret = ComoConfig::AddZeroMQEndpoint(std::string("ServiceManager"),
                                            std::string("tcp://127.0.0.1:8088"));
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
    ServiceManager::GetInstance()->AddRemoteService(
                                                String("localhost"),
                                                String("ServiceManager"),
                                                String("rpcserviceZMQ"), srv);

    printf("RPC serviceZMQ waiting for calling endpoint: %s\n", endpoint.c_str());

    CZMQUtils::CzmqProxy(nullptr, endpoint.c_str(), nullptr);

    while (true) {
        sleep(5);
    }

    return 0;
}
