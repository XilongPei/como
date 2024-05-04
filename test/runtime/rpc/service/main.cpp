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
#include "ComoContext.h"

using namespace como;
using como::test::rpc::CService;
using como::test::rpc::IService;
using como::test::rpc::IID_IService;
using jing::ServiceManager;

int main(int argv, char** argc)
{
    AutoPtr<IService> srv;

    ECode ec = CService::New(IID_IService, (IInterface**)&srv);
    if (FAILED(ec)) {
        printf("Failed, CService::New, ECode: 0x%X\n", ec);
        return 1;
    }

    AutoPtr<ServiceManager> instance = ServiceManager::GetInstance();
    instance->AddService(String("rpcservice"), srv);

    printf("==== rpc service wait for calling ====\n");

    if (FAILED(instance->WaitForRuntimeToEnd())) {
        while (true) {
            printf(".");
            sleep(5);
        }
        printf("\n");
    }

    return 0;
}
