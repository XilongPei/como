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

#include <comoapi.h>
#include <comosp.h>
#include <comoobj.h>
#include <cstdio>
#include <cstdlib>
#include <gtest/gtest.h>
#include "Foo.h"

using namespace como;

using como::demo::CFoo;
using como::demo::IFoo;
using como::demo::IID_IFoo;

TEST(TestIDfromName, testPrintf)
{
    InterfaceID iid = InterfaceIDWithMemArea(IID_IFoo, 1);
    AutoPtr<IFoo> foo;
    ECode ec = CFoo::New(iid, (IInterface**)&foo);
    EXPECT_EQ(ec, NOERROR);
    foo->printf_str(String("Hello World\n"));
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
