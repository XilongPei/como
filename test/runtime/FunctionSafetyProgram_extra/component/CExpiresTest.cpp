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

#include "CExpiresTest.h"
#include <cstdio>
#include <time.h>

namespace como {
namespace test {
namespace expires {

COMO_INTERFACE_IMPL_1(CExpiresTest, Object, IExpiresTest);

CExpiresTest::CExpiresTest()
{
    printf("==== Call CExpiresTest() ====\n");
}

CExpiresTest::~CExpiresTest()
{
    printf("==== Call ~CExpiresTest() ====\n");
}

ECode CExpiresTest::Add(
    /* [in] */ Integer arg1,
    /* [in] */ Integer arg2,
    /* [out] */ Integer& result)
{
    result = arg1 + arg2;

    printf("==== Call CExpiresTest::Add, arg1 is %d ====\n", arg1);
    printf("====                         arg2 is %d ====\n", arg2);
    printf("====                       result is %d ====\n", result);
    return NOERROR;
}

ECode CExpiresTest::Add_Wait(
    /* [in] */ Integer arg1,
    /* [in] */ Integer arg2,
    /* [in] */ Integer wait_time,
    /* [out] */ Integer& result)
{
    result = arg1 + arg2;

    timespec t, remain;
    t.tv_sec = 0;
    t.tv_nsec = wait_time;
    nanosleep(&t, &remain);

    printf("==== Call CExpiresTest::Add_Wait, arg1 is %d ====\n", arg1);
    printf("====                              arg2 is %d ====\n", arg2);
    printf("====                         wait_time is %d ====\n", wait_time);
    printf("====                            result is %d ====\n", result);
    return NOERROR;
}

ECode CExpiresTest::Sub(
    /* [in] */ Integer arg1,
    /* [in] */ Integer arg2,
    /* [out] */ Integer& result)
{
    result = arg1 - arg2;

    printf("==== Call CExpiresTest::Sub, arg1 is %d ====\n", arg1);
    printf("====                         arg2 is %d ====\n", arg2);
    printf("====                       result is %d ====\n", result);
    return NOERROR;
}

ECode CExpiresTest::Sub_Wait(
    /* [in] */ Integer arg1,
    /* [in] */ Integer arg2,
    /* [in] */ Integer wait_time,
    /* [out] */ Integer& result)
{
    result = arg1 - arg2;

    timespec t, remain;
    t.tv_sec = 0;
    t.tv_nsec = wait_time;
    nanosleep(&t, &remain);

    printf("==== Call CExpiresTest::Sub_Wait, arg1 is %d ====\n", arg1);
    printf("====                              arg2 is %d ====\n", arg2);
    printf("====                         wait_time is %d ====\n", wait_time);
    printf("====                            result is %d ====\n", result);
    return NOERROR;
}

}
}
}
