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
    string str;
    bitset<8> bits;
    bits = (int)'u';

    str = bits.to_string();

    const char *p = str.c_str();
    unsigned long L1, L2;

    L1 = VerifiedU64Pointer::encodeUnsignedLong((unsigned long)p);
    L2 = VerifiedU64Pointer::decodeUnsignedLong(L1);

    EXPECT_EQ(p, (char *)L2);
}

TEST(VerifiedU64Pointer, testVerifiedU64Pointer2)
{
    struct timespec tsTime;
    clock_gettime(CLOCK_REALTIME, &tsTime);
    srand(tsTime.tv_nsec);
    for(int i = 0; i < 1000; i++){
        string str;
        const char *p = str.c_str();
        unsigned long L1, L2;
        L1 = VerifiedU64Pointer::encodeUnsignedLong((unsigned long)p);
        int a = rand() % 6, b = rand() % 8;
        char x = 0;
        x |= (1 << b);
        *((char *)&L1 + a) ^= x;
        L2 = VerifiedU64Pointer::decodeUnsignedLong(L1);
        EXPECT_EQ(p, (char *)L2);
    }
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
