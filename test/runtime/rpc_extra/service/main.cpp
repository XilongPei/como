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
#include <cstdio>
#include <unistd.h>

using como::test::rpc::CService2;
using como::test::rpc::IService2;
using como::test::rpc::IID_IService2;
using jing::ServiceManager;

// Before running this program, set LIB_PATH to import .so file.
// "export LIB_PATH=COMO_SOURCE_PATH/out/target/como.linux.x64.rls/test/runtime/rpc_extra/component"

int main(int argv, char** argc)
{
    AutoPtr<IService2> srv;

    ECode ec = CService2::New(IID_IService2, (IInterface**)&srv);
    if (FAILED(ec)) {
        printf("Failed, CService2::New, ECode: 0x%X\n", ec);
        return 1;
    }

    String uuidStr;
    uuidStr = DumpUUID(IID_IService2.mUuid);
    printf("CService2, IID_IService2: %s\n", uuidStr.string());

    ServiceManager::GetInstance()->AddService(String("rpcservice2"), srv);

    printf("==== rpc service wait for calling ====\n");
    while (true) {
        sleep(5);
    }

    return 0;
}
