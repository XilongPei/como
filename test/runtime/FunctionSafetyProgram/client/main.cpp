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

#include "como.demo.CFoo.h"
#include "como.demo.IFoo.h"
#include "como.demo.IBar.h"
#include "como.demo.CFooBar.h"
#include "FooBarDemo.h"
#include <comoapi.h>
#include <comosp.h>
#include <comoobj.h>
#include <cstdio>
#include <cstdlib>
#include <gtest/gtest.h>
#include <ComoContext.h>

using namespace como;

using como::demo::CFoo;
using como::demo::IFoo;
using como::demo::IBar;
using como::demo::CFooBar;
using como::demo::IID_IFoo;
using como::demo::IID_IBar;


TEST(TestFunctionSafetyProgram, testNewInMemArea)
{
    InterfaceID iid = InterfaceIDfromNameWithMemArea("como::demo::IBar", 1);
    AutoPtr<IBar> bar;
    ECode ec = CFooBar::New(iid, (IInterface**)&bar);
    EXPECT_EQ(ec, NOERROR);
    bar->Bar(String("testNewInMemArea: Tongji University"));
}

TEST(TestFunctionSafetyProgram, testNewInMemAreaWithIID)
{
    InterfaceID iid = InterfaceIDWithMemArea(IID_IBar, 1);
    AutoPtr<IBar> bar;
    ECode ec = CFooBar::New(iid, (IInterface**)&bar);
    EXPECT_EQ(ec, NOERROR);
    bar->Bar(String("testNewInMemAreaWithIID: Tongji University"));
}

TEST(TestFunctionSafetyProgram, testGetChecksum)
{
    InterfaceID iid = InterfaceIDWithMemArea(IID_IBar, 1);
    AutoPtr<IBar> bar;
    ECode ec = CFooBar::New(iid, (IInterface**)&bar);
    EXPECT_EQ(ec, NOERROR);

    Long lastChecksum, currentChecksum;
    IComoFunctionSafetyObject *icfso = IComoFunctionSafetyObject::Probe(bar);
    EXPECT_NE(nullptr, icfso);

    icfso->GetChecksum(lastChecksum, currentChecksum);
    EXPECT_EQ(lastChecksum, 0);

    icfso->GetChecksum(lastChecksum, currentChecksum);
    EXPECT_EQ(lastChecksum, currentChecksum);
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

// TODO, fix it
return;
    free((void*)ptr);
}

int main(int argc, char **argv)
{
    ECode ec = ComoContext::SetGctxMemFun(area_malloc, area_free);
    EXPECT_EQ(NOERROR, ec);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
