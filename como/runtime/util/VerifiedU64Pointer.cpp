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

#include "VerifiedU64Pointer.h"

namespace como {

int gUnfixedMemError = 0;
int gFixedMemError = 0;

/*
A 64-bit Linux, the upper 2 bytes of the user mode pointer are always 0, and
these 2 bytes are used to store the check value of the remaining 6 bytes.
Errors of 1 bit can be corrected, and errors of 2 or more bits can be detected.

 long integer(8 bytes): 6 bytes, C_Byte(2 bytes, C_byte0, C_byte1) for checksum

   7 6 5 4 3 2 1 0
0: | | | | | | | |   => bit7 ^ bit6 ^ ... bit0 => C_byte0[0]
1: . . . . . . . .
2:                   ......
3:                   ......
4: . . . . . . . .
5: | | | | | | | |   => bit7 ^ bit6 ^ ... bit0 => C_byte0[5]
   7 6 5 4 3 2 1 0
   |             +-- => bit5 ^ ... bit0 => C_byte1[0]
   | ......
   +-- => bit5 ^ ... bit0 => C_byte1[7]

   the check bits of the 2 check bytes
   C_byte0[6] => C_byte1[0..7]
   C_byte0[7] => C_byte0[0..5]
*/

struct _byte {
    unsigned b7:1;      unsigned b6:1;      unsigned b5:1;      unsigned b4:1;
    unsigned b3:1;      unsigned b2:1;      unsigned b1:1;      unsigned b0:1;
};

struct _long {
    unsigned b0_7:1;    unsigned b0_6:1;    unsigned b0_5:1;    unsigned b0_4:1;
    unsigned b0_3:1;    unsigned b0_2:1;    unsigned b0_1:1;    unsigned b0_0:1;

    unsigned b1_7:1;    unsigned b1_6:1;    unsigned b1_5:1;    unsigned b1_4:1;
    unsigned b1_3:1;    unsigned b1_2:1;    unsigned b1_1:1;    unsigned b1_0:1;

    unsigned b2_7:1;    unsigned b2_6:1;    unsigned b2_5:1;    unsigned b2_4:1;
    unsigned b2_3:1;    unsigned b2_2:1;    unsigned b2_1:1;    unsigned b2_0:1;

    unsigned b3_7:1;    unsigned b3_6:1;    unsigned b3_5:1;    unsigned b3_4:1;
    unsigned b3_3:1;    unsigned b3_2:1;    unsigned b3_1:1;    unsigned b3_0:1;

    unsigned b4_7:1;    unsigned b4_6:1;    unsigned b4_5:1;    unsigned b4_4:1;
    unsigned b4_3:1;    unsigned b4_2:1;    unsigned b4_1:1;    unsigned b4_0:1;

    unsigned b5_7:1;    unsigned b5_6:1;    unsigned b5_5:1;    unsigned b5_4:1;
    unsigned b5_3:1;    unsigned b5_2:1;    unsigned b5_1:1;    unsigned b5_0:1;

    unsigned b6_7:1;    unsigned b6_6:1;    unsigned b6_5:1;    unsigned b6_4:1;
    unsigned b6_3:1;    unsigned b6_2:1;    unsigned b6_1:1;    unsigned b6_0:1;

    unsigned b7_7:1;    unsigned b7_6:1;    unsigned b7_5:1;    unsigned b7_4:1;
    unsigned b7_3:1;    unsigned b7_2:1;    unsigned b7_1:1;    unsigned b7_0:1;
};


unsigned long VerifiedU64Pointer::encodeUnsignedLong(unsigned long l)
{
    struct _long *L = (struct _long *)&l;

    L->b6_7 = L->b0_7 ^ L->b1_7 ^ L->b2_7 ^ L->b3_7 ^ L->b4_7 ^ L->b5_7;
    L->b6_6 = L->b0_6 ^ L->b1_6 ^ L->b2_6 ^ L->b3_6 ^ L->b4_6 ^ L->b5_6;
    L->b6_5 = L->b0_5 ^ L->b1_5 ^ L->b2_5 ^ L->b3_5 ^ L->b4_5 ^ L->b5_5;
    L->b6_4 = L->b0_4 ^ L->b1_4 ^ L->b2_4 ^ L->b3_4 ^ L->b4_4 ^ L->b5_4;
    L->b6_3 = L->b0_3 ^ L->b1_3 ^ L->b2_3 ^ L->b3_3 ^ L->b4_3 ^ L->b5_3;
    L->b6_2 = L->b0_2 ^ L->b1_2 ^ L->b2_2 ^ L->b3_2 ^ L->b4_2 ^ L->b5_2;
    L->b6_1 = L->b0_1 ^ L->b1_1 ^ L->b2_1 ^ L->b3_1 ^ L->b4_1 ^ L->b5_1;
    L->b6_0 = L->b0_0 ^ L->b1_0 ^ L->b2_0 ^ L->b3_0 ^ L->b4_0 ^ L->b5_0;

    L->b7_5 = L->b5_7 ^ L->b5_6 ^ L->b5_5 ^ L->b5_4 ^ L->b5_3 ^ L->b5_2 ^ L->b5_1 ^ L->b5_0;
    L->b7_4 = L->b4_7 ^ L->b4_6 ^ L->b4_5 ^ L->b4_4 ^ L->b4_3 ^ L->b4_2 ^ L->b4_1 ^ L->b4_0;
    L->b7_3 = L->b3_7 ^ L->b3_6 ^ L->b3_5 ^ L->b3_4 ^ L->b3_3 ^ L->b3_2 ^ L->b3_1 ^ L->b3_0;
    L->b7_2 = L->b2_7 ^ L->b2_6 ^ L->b2_5 ^ L->b2_4 ^ L->b2_3 ^ L->b2_2 ^ L->b2_1 ^ L->b2_0;
    L->b7_1 = L->b1_7 ^ L->b1_6 ^ L->b1_5 ^ L->b1_4 ^ L->b1_3 ^ L->b1_2 ^ L->b1_1 ^ L->b1_0;
    L->b7_0 = L->b0_7 ^ L->b0_6 ^ L->b0_5 ^ L->b0_4 ^ L->b0_3 ^ L->b0_2 ^ L->b0_1 ^ L->b0_0;

    // Calculate the check bit of the check byte
    L->b7_7 = L->b7_5 ^ L->b7_4 ^ L->b7_3 ^ L->b7_2 ^ L->b7_1 ^ L->b7_0;
    L->b7_6 = L->b6_7 ^ L->b6_6 ^ L->b6_5 ^ L->b6_4 ^ L->b6_3 ^ L->b6_2 ^ L->b6_1 ^ L->b6_0;

    return l;
}

unsigned long VerifiedU64Pointer::decodeUnsignedLong(unsigned long l)
{
    struct _long *L = (struct _long *)&l;
    unsigned char b1, b0;
    struct _byte *by1 = (struct _byte*)&b1;
    struct _byte *by0 = (struct _byte*)&b0;

    by0->b7 = L->b6_7 ^ L->b0_7 ^ L->b1_7 ^ L->b2_7 ^ L->b3_7 ^ L->b4_7 ^ L->b5_7;
    by0->b6 = L->b6_6 ^ L->b0_6 ^ L->b1_6 ^ L->b2_6 ^ L->b3_6 ^ L->b4_6 ^ L->b5_6;
    by0->b5 = L->b6_5 ^ L->b0_5 ^ L->b1_5 ^ L->b2_5 ^ L->b3_5 ^ L->b4_5 ^ L->b5_5;
    by0->b4 = L->b6_4 ^ L->b0_4 ^ L->b1_4 ^ L->b2_4 ^ L->b3_4 ^ L->b4_4 ^ L->b5_4;
    by0->b3 = L->b6_3 ^ L->b0_3 ^ L->b1_3 ^ L->b2_3 ^ L->b3_3 ^ L->b4_3 ^ L->b5_3;
    by0->b2 = L->b6_2 ^ L->b0_2 ^ L->b1_2 ^ L->b2_2 ^ L->b3_2 ^ L->b4_2 ^ L->b5_2;
    by0->b1 = L->b6_1 ^ L->b0_1 ^ L->b1_1 ^ L->b2_1 ^ L->b3_1 ^ L->b4_1 ^ L->b5_1;
    by0->b0 = L->b6_0 ^ L->b0_0 ^ L->b1_0 ^ L->b2_0 ^ L->b3_0 ^ L->b4_0 ^ L->b5_0;

    by1->b5 = L->b7_5 ^ L->b5_7 ^ L->b5_6 ^ L->b5_5 ^ L->b5_4 ^ L->b5_3 ^ L->b5_2 ^ L->b5_1 ^ L->b5_0;
    by1->b4 = L->b7_4 ^ L->b4_7 ^ L->b4_6 ^ L->b4_5 ^ L->b4_4 ^ L->b4_3 ^ L->b4_2 ^ L->b4_1 ^ L->b4_0;
    by1->b3 = L->b7_3 ^ L->b3_7 ^ L->b3_6 ^ L->b3_5 ^ L->b3_4 ^ L->b3_3 ^ L->b3_2 ^ L->b3_1 ^ L->b3_0;
    by1->b2 = L->b7_2 ^ L->b2_7 ^ L->b2_6 ^ L->b2_5 ^ L->b2_4 ^ L->b2_3 ^ L->b2_2 ^ L->b2_1 ^ L->b2_0;
    by1->b1 = L->b7_1 ^ L->b1_7 ^ L->b1_6 ^ L->b1_5 ^ L->b1_4 ^ L->b1_3 ^ L->b1_2 ^ L->b1_1 ^ L->b1_0;
    by1->b0 = L->b7_0 ^ L->b0_7 ^ L->b0_6 ^ L->b0_5 ^ L->b0_4 ^ L->b0_3 ^ L->b0_2 ^ L->b0_1 ^ L->b0_0;

    // Verify the check bit of the check byte
    by1->b7 = L->b7_7 ^ L->b7_5 ^ L->b7_4 ^ L->b7_3 ^ L->b7_2 ^ L->b7_1 ^ L->b7_0;
    by1->b6 = L->b7_6 ^ L->b6_7 ^ L->b6_6 ^ L->b6_5 ^ L->b6_4 ^ L->b6_3 ^ L->b6_2 ^ L->b6_1 ^ L->b6_0;

    if (('\0' == b1) && ('\0' == b0)) {
        *((unsigned short *)&l + 3) = 0;
        return l;
    }

    // one is 0, another is not 0
    if (b1 ^ b0) {
        *((unsigned short *)&l + 3) = 0;
        gUnfixedMemError++;
        return l;
    }

    // fix the pointer
    int n1, n0;
    unsigned char *b;
    for (n1 = 0;  n1 < 6;  n1++) {
        if ((1 << n1) & b1) {
            for (n0 = 0;  n0 < 8;  n0++) {
                if ((1 << n0) & b0) {
                    b = (unsigned char *)&l + n1;
                    *b ^= (1 << n0);
                    gFixedMemError++;
                }
            }
        }
    }

    // set highest 2 bytes of l to 0
    *((unsigned short *)&l + 3) = 0;

    return l;
}

} // namespace como
