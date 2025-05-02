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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netdb.h>
#include "IP_Utils.h"

namespace como {

static bool match_ipv6_prefix(struct in6_addr *a, struct in6_addr *b, int prefix_len)
{
    int full_bytes = prefix_len / 8;
    int remaining_bits = prefix_len % 8;

    if (full_bytes > 16) full_bytes = 16;

    if (memcmp(a->s6_addr, b->s6_addr, full_bytes) != 0) {
        return false;
    }

    if (remaining_bits > 0) {
        uint8_t mask = 0xFF << (8 - remaining_bits);
        if ((a->s6_addr[full_bytes] & mask) != (b->s6_addr[full_bytes] & mask)) {
            return false;
        }
    }

    return true;
}

static int get_prefix_length(struct sockaddr *netmask)
{
    struct sockaddr_in6 *mask6 = (struct sockaddr_in6 *)netmask;
    int bits = 0;
    for (int i = 0;  i < 16;  i++) {
        uint8_t byte = mask6->sin6_addr.s6_addr[i];
        while (byte & 0x80) {
            bits++;
            byte <<= 1;
        }
        if (byte != 0) {
            break;
        }
    }
    return bits;
}

IP_Relation IP_Utils::check_ip_relation(const char *ip_str)
{
    struct in_addr ipv4;
    struct in6_addr ipv6;
    struct ifaddrs *ifaddr, *ifa;
    char host[NI_MAXHOST];

    if (nullptr == ip_str) {
        return IP_BAD_ADDRESS;
    }

    // If the IP address contains port information, like this: 127.0.0.1:8088
    // remove the port information.
    char *p = strchr((char *)ip_str, ':');
    if (nullptr != p) {
        strncpy(host, ip_str, sizeof(host));
        host[sizeof(host) - 1] = '\0';

        // Prevent the IP address in ip_str from being too long, Reposition the ':'
        p = strchr(host, ':');

        *p = '\0';
        ip_str = host;
    }

    if (strcmp(ip_str, "127.0.0.1") == 0 || strcmp(ip_str, "::1") == 0) {
        return IP_IS_LOOPBACK;
    }

    if (strcmp(ip_str, "255.255.255.255") == 0) {
        return IP_IS_BROADCAST;
    }

    if (inet_pton(AF_INET, ip_str, &ipv4) == 1) {
        if ((ntohl(ipv4.s_addr) & 0xf0000000) == 0xe0000000) {
            return IP_IS_MULTICAST;
        }
    }
    else if (inet_pton(AF_INET6, ip_str, &ipv6) == 1) {
        if (ipv6.s6_addr[0] == 0xff) {
            return IP_IS_MULTICAST;
        }
    }
    else {
        return IP_NOT_LOCAL;
    }

    if (getifaddrs(&ifaddr) == -1) {
        return IP_NOT_LOCAL;
    }

    int is_local = 0;
    int is_subnet = 0;

    for (ifa = ifaddr;  ifa != nullptr;  ifa = ifa->ifa_next) {
        if ((! ifa->ifa_addr) || ! (ifa->ifa_netmask)) {
            continue;
        }

        int family = ifa->ifa_addr->sa_family;

        if ((family == AF_INET) && (inet_pton(AF_INET, ip_str, &ipv4) == 1)) {
            struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
            struct sockaddr_in *mask = (struct sockaddr_in *)ifa->ifa_netmask;
            uint32_t input = ntohl(ipv4.s_addr);
            uint32_t local = ntohl(addr->sin_addr.s_addr);
            uint32_t netmask = ntohl(mask->sin_addr.s_addr);

            if (input == local) {
                is_local = 1;
                break;
            }
            if ((input & netmask) == (local & netmask)) {
                is_subnet = 1;
            }

            if ((ifa->ifa_flags & IFF_BROADCAST) && ifa->ifa_broadaddr) {
                struct sockaddr_in *bcast = (struct sockaddr_in *)ifa->ifa_broadaddr;
                if (bcast->sin_addr.s_addr == ipv4.s_addr) {
                    freeifaddrs(ifaddr);
                    return IP_IS_BROADCAST;
                }
            }
        }
        else if (family == AF_INET6 && inet_pton(AF_INET6, ip_str, &ipv6) == 1) {
            struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)ifa->ifa_addr;
            struct sockaddr_in6 *mask6 = (struct sockaddr_in6 *)ifa->ifa_netmask;

            if (memcmp(&addr6->sin6_addr, &ipv6, sizeof(struct in6_addr)) == 0) {
                is_local = 1;
                break;
            }

            int prefix_len = get_prefix_length((struct sockaddr *)mask6);

            if (match_ipv6_prefix(&addr6->sin6_addr, &ipv6, prefix_len)) {
                is_subnet = 1;
            }
        }
    }

    freeifaddrs(ifaddr);

    if (is_local) {
        return IP_IS_LOCAL;
    }
    else if (is_subnet) {
        return IP_SAME_SUBNET;
    }

    return IP_NOT_LOCAL;
}

bool IP_Utils::is_loopback_ip(const char *ip_str)
{
    if (nullptr == ip_str) {
        return false;
    }

    return (strcmp(ip_str, "127.0.0.1") == 0 || strcmp(ip_str, "::1") == 0);
}

bool IP_Utils::is_multicast_ip(const char *ip_str)
{
    struct in_addr ipv4;
    struct in6_addr ipv6;

    if (nullptr == ip_str) {
        return false;
    }

    if (inet_pton(AF_INET, ip_str, &ipv4) == 1) {
        return ((ntohl(ipv4.s_addr) & 0xf0000000) == 0xe0000000);
    }
    else if (inet_pton(AF_INET6, ip_str, &ipv6) == 1) {
        return ipv6.s6_addr[0] == 0xff;
    }

    return 0;
}

bool IP_Utils::is_broadcast_ip(const char *ip_str)
{
    if (nullptr == ip_str) {
        return false;
    }

    return strcmp(ip_str, "255.255.255.255") == 0;
}

bool IP_Utils::is_same_subnet_ip(const char *ip_str)
{
    if (nullptr == ip_str) {
        return false;
    }

    return check_ip_relation(ip_str) == IP_SAME_SUBNET;
}

bool IP_Utils::is_local_ip(const char *ip_str)
{
    if (nullptr == ip_str) {
        return false;
    }

    return check_ip_relation(ip_str) == IP_IS_LOCAL;
}

} // namespace como
