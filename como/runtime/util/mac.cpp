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

#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include "mac.h"

namespace como {

static Long g_lMacAddr = 0;

/**
 * The first three bytes (upper 24 bits) of MAC are assigned to different
 * manufacturers by the registration management organization RA of IEEE, also
 * known as "Organizational Unique Identifier (OUI)". The last three bytes
 * (lower 24 bits) are assigned to the adapter interface produced by each
 * manufacturer, which is called extended identifier (uniqueness).
 */
Long Mac::GetMacAddress(Long& lMacAddr)
{
    struct ifreq ifr;
    struct ifconf ifc;
    char buf[1024];
    int success = 0;

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1) { /* handle error*/ };

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) { /* handle error */ }

    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

    for (;  it != end;  ++it) {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
            if (! (ifr.ifr_flags & IFF_LOOPBACK)) {     // don't count loopback
                if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
                    memcpy(&lMacAddr, ifr.ifr_hwaddr.sa_data, 6);
                    return lMacAddr;
                }
            }
        }
        else { /* handle error */ }
    }

    struct timespec tsTime;
    clock_gettime(CLOCK_REALTIME, &tsTime);

    srand(tsTime.tv_nsec);
    lMacAddr = ((Long)rand() << 32) | rand();

    return 0;
}

Long Mac::GetThisServiceId(unsigned short port)
{
    if (0 == g_lMacAddr) {
        Mac::GetMacAddress(g_lMacAddr);
    }

    Long lMacAddr = g_lMacAddr;
    *(unsigned short *)((unsigned char *)&lMacAddr + 6) = port;

    return lMacAddr;
}

Long Mac::GetUuid64(Long& uuid64)
{
    struct timespec tsTime;
    clock_gettime(CLOCK_REALTIME, &tsTime);

    // millisecond
    uuid64 = tsTime.tv_sec * 1000 + tsTime.tv_nsec / 1000000;
                                                   // 654321
    if (0 == g_lMacAddr)
        Mac::GetMacAddress(g_lMacAddr);

    // This value is not repeated within 49.710 days.
    // 0xFFFFFFFF(millisecond) / 3600,000(millisecond/hour) / 24(hour/day) ~= 49.710(day)
    //    4 3 2 1
    uuid64 = g_lMacAddr << 32 | (uuid64 & 0xFFFFFFFF);
                                          // 4 3 2 1
    return uuid64;
}

} // namespace como


#if 0
int main()
{
    Long lMacAddr = Mac::GetThisServiceId(lMacAddr, 1239);
    if (0 != lMacAddr) {
        unsigned char *mac_addr = (unsigned char *)&lMacAddr;
        printf("local mac: %02x:%02x:%02x:%02x:%02x:%02x  port: %d\n",
                            mac_addr[0], mac_addr[1], mac_addr[2],
                            mac_addr[3], mac_addr[4], mac_addr[5],
                            *(unsigned short *)((unsigned char *)&lMacAddr + 6));
    }
}
#endif
