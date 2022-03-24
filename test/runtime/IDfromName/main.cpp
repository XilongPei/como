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

#include <comosp.h>
#include <comoobj.h>
#include <gtest/gtest.h>
#include "_Namespace1_Namespace2_Namespace3_CFooBar.h"
#include "CFooBar.h"

using namespace como;
using namespace Namespace1::Namespace2::Namespace3;

TEST(TestIDfromName, testInterfaceIDfromName)
{
    const ComponentID componentID = ComponentIDfromName("TestModule",
                                    "http://como.org/components/test/IDfromName.so");
    InterfaceID iid = InterfaceIDfromName("String::namespaceAndName", &componentID);
    EXPECT_STREQ(DumpUUID(iid.mUuid), "8e174f8b-5563-f9ba-6010-7c8c594bf8b0");
}

TEST(TestIDfromName, testNewWithoutIID)
{
    const ComponentID componentID = ComponentIDfromName("TestModuleIDfromName", nullptr);
    InterfaceID iid = InterfaceIDfromName("Namespace1::Namespace2::Namespace3::CFooBar", &componentID);
    AutoPtr<IFooBar> fooBar;
    ECode ec = CFooBar::New(iid, (IInterface**)&fooBar);
    EXPECT_EQ(ec, NOERROR);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
