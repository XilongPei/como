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

#define __IN_COMOTEST__

#include <comosp.h>
#include <comoobj.h>
#include <mac.h>
#include <gtest/gtest.h>
#include <unordered_set>
#include <iostream>

namespace como {
Short g_iRuntimeID = 0;
}

using namespace como;

Long lMacAddr = 0;

TEST(Mac, TestMac1)
{
    Long lMacAddr = Mac::GetThisServiceId(1239);

    if (0 != lMacAddr) {
        unsigned char *mac_addr = (unsigned char *)&lMacAddr;
        printf("local mac: %02x:%02x:%02x:%02x:%02x:%02x  port: %d\n",
                            mac_addr[0], mac_addr[1], mac_addr[2],
                            mac_addr[3], mac_addr[4], mac_addr[5],
                            *(unsigned short *)((unsigned char *)&lMacAddr + 6));
    }
}

TEST(Mac, TestMac2)
{
    std::unordered_set<Long> st;
    for(int i = 0; i < 1000; i++){
        lMacAddr = Mac::GetUuid64(lMacAddr);
        std::cout << lMacAddr << std::endl;
    }
}


int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

