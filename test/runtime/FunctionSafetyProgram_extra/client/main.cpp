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

#include <comoapi.h>
#include <comosp.h>
#include <comoobj.h>
#include <cstdio>
#include <cstdlib>
#include <gtest/gtest.h>
#include <ComoContext.h>
#include "ExpiresTestUnit.h"
#include "como.test.expires.IExpiresTest.h"
#include "como.test.expires.CExpiresTest.h"
#include <ComoFunctionSafetyObject.h>

using namespace como;

using como::test::expires::CExpiresTest;
using como::test::expires::IExpiresTest;
using como::test::expires::IID_IExpiresTest;

TEST(TestFunctionSafetyProgram, testExpires)
{
    InterfaceID iid = InterfaceIDWithMemArea(IID_IExpiresTest, 1);
    AutoPtr<IExpiresTest> ExpiresTest;
    ECode ec = CExpiresTest::New(iid, (IInterface**)&ExpiresTest);
    EXPECT_EQ(ec, NOERROR);

    Integer isvalid;
    IComoFunctionSafetyObject *icfso = IComoFunctionSafetyObject::Probe(ExpiresTest);
    EXPECT_NE(nullptr, icfso);

    Integer arg1, arg2, wait_time, result;
    arg1 = 10;
    arg2 = 20;
    result = 0;
    ec = ExpiresTest->Add(arg1, arg2, result);
    EXPECT_EQ(ec, NOERROR);
    EXPECT_EQ(result, 30);
    icfso->IsValid(isvalid);
    EXPECT_EQ(isvalid, CFSO_ExpireVALID);

    wait_time = 5000;   // 5000ns
    result = 0;
    ec = ExpiresTest->Add_Wait(arg1, arg2, wait_time, result);
    EXPECT_EQ(ec, NOERROR);
    EXPECT_EQ(result, 30);
    icfso->IsValid(isvalid);
    EXPECT_EQ(isvalid, CFSO_ExpireVALID);

    wait_time = 20000000;  // 20000ns
    result = 0;
    ec = ExpiresTest->Add_Wait(arg1, arg2, wait_time, result);
    EXPECT_NE(ec, 0);
    icfso->IsValid(isvalid);
    EXPECT_EQ(isvalid, CFSO_ExpireTime);
}

static void *area_malloc(Short iMemArea, size_t size)
{
    (void)iMemArea;

    void *ptr = malloc(size);
    return ptr;
}

static void area_free(Short iMemArea, const void *ptr)
{
    (void)iMemArea;

    free((void*)ptr);
}

int main(int argc, char **argv)
{
    ECode ec = ComoContext::SetGctxMemFun(area_malloc, area_free);
    EXPECT_EQ(NOERROR, ec);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
