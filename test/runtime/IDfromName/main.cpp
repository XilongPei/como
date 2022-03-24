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

using namespace como;

TEST(testIDfromName, testIDfromName)
{
    ComponentID componentID = ComponentIDfromName("String name", nullptr);
    InterfaceID iid = InterfaceIDfromName("String::namespaceAndName", componentID);
    EXPECT_EQ(DumpUUID(iid.mUuid), "8b4f178e-6355-baf9-1060-7c8c594bf8b0");
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
