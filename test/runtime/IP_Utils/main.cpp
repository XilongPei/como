//=========================================================================
// Copyright (C) 2025 The C++ Component Model(COMO) Open Source Project
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

#include <gtest/gtest.h>
#include "IP_Utils.h"

using namespace como;

TEST(IP_UtilsTest, IP_Utils_test)
{
    const char *ips[] = {
        "127.0.0.1", "255.255.255.255", "224.0.0.1", "192.168.1.88","172.27.72.108",
        "8.8.8.8", "fe80::1"
    };

    for (int i = 0;  i < sizeof(ips) / sizeof(ips[0]);  i++) {
        const char *ip = ips[i];
        printf("%-20s => ", ip);
        if (como::IP_Utils::is_loopback_ip(ip)) {
            printf("环回地址\n");
        }
        else if (como::IP_Utils::is_broadcast_ip(ip)) {
            printf("广播地址\n");
        }
        else if (como::IP_Utils::is_multicast_ip(ip)) {
            printf("多播地址\n");
        }
        else if (como::IP_Utils::is_local_ip(ip)) {
            printf("本机地址\n");
        }
        else if (como::IP_Utils::is_same_subnet_ip(ip)) {
            printf("同一子网\n");
        }
        else {
            printf("外部地址\n");
        }
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
