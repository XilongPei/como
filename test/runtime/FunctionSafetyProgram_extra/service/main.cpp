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

#include "ExpiresTestUnit.h"
#include <comoapi.h>
#include <comosp.h>
#include <ServiceManager.h>
#include <cstdio>
#include <unistd.h>

using como::test::expires::CExpiresTest;
using como::test::expires::IExpiresTest;
using como::test::expires::IID_IExpiresTest;
using jing::ServiceManager;

int main(int argv, char** argc)
{
    AutoPtr<IExpiresTest> srv;

    ECode ec = CExpiresTest::New(IID_IExpiresTest, (IInterface**)&srv);
    if (FAILED(ec)) {
        printf("Failed, CExpiresTest::New, ECode: 0x%X\n", ec);
        return 1;
    }

    String uuidStr;
    uuidStr = DumpUUID(IID_IExpiresTest.mUuid);
    printf("CExpiresTest, IID_IExpiresTest: %s\n", uuidStr.string());

    ServiceManager::GetInstance()->AddService(String("expiretest"), srv);

    printf("==== expiretest service wait for calling ====\n");
    while (true) {
        sleep(5);
    }

    return 0;
}
