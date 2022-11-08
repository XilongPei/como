#define __IN_COMOTEST__

#include <comosp.h>
#include <comoobj.h>
#include <gtest/gtest.h>
#include <VerifiedU64Pointer.h>
#include <iostream>
#include <bits/stdc++.h>
#include <string>
#include <time.h>

using namespace como;
using namespace std;

TEST(VerifiedU64Pointer, testVerifiedU64Pointer1)
{
    std::string str;
    std::bitset<8> bits;
    bits = (int)'u';

    str = bits.to_string();
    printf("bits: %s 'u'\n", str.c_str());

    const char *p = str.c_str();
    unsigned long L1, L2;

    L1 = VerifiedU64Pointer::encodeUnsignedLong((unsigned long)p);
    L2 = VerifiedU64Pointer::decodeUnsignedLong(L1);

    printf("p: %lx   L1: %lx   L2: %lx   p-L2: %lx\n", (unsigned long)p, L1, L2, (unsigned long)p-L2);
}

// TEST(VerifiedU64Pointer, testVerifiedU64Pointer2)
// {
//     struct timespec tsTime;
//     srand(tsTime.tv_nsec);
//     for(int i = 0; i < 1000; i++){
//         std::string std;
//         const char *p = str.c_str();
//         unsigned long L1, L2;

//     }
// }

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
