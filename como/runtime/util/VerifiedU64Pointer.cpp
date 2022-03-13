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

#include <cstdio>
#include <bitset>
#include <string>

using namespace std;

int gMemError = 0;

/*
一个64位Linux，用户态指针高位2个字节一直为0，利用这2个字节用来存其余6个字节的校验值，
可以修正1个比特的错，检测出2个比特的错。

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

   C_byte0[6]
   C_byte0[7]
*/

struct _byte {
    unsigned b7:1;      unsigned b6:1;      unsigned b5:1;      unsigned b4:1;
    unsigned b3:1;      unsigned b2:1;      unsigned b1:1;      unsigned b0:1;
};

static unsigned get_bit_count(unsigned char b)
{
    struct _byte *by = (struct _byte*)&b;
    return by->b7 + by->b6 + by->b5 + by->b4 + by->b3 + by->b2 + by->b1 + by->b0;
}

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


unsigned long encodeUnsignedLong(unsigned long l)
{
    struct _long *L = (struct _long *)&l;

    L->b6_5 = L->b0_7 ^ L->b1_7 ^ L->b2_7 ^ L->b3_7 ^ L->b4_7 ^ L->b5_7 ^ L->b6_7 ^ L->b7_7;

    return l;
}

unsigned long decodeUnsignedLong(unsigned long l)
{
    unsigned long L;

    // found error, set global error information, don't clear it for others could set it before
    gMemError = 1;

    return L;
}


int main()
{
    string str;
    bitset<8> bits;
    bits = (int)'u';

    str = bits.to_string();
    printf("bit count: %d of %s 'u'\n", get_bit_count('u'), str.c_str());

    const char *p = str.c_str();
    unsigned long L1, L2;

    L1 = encodeUnsignedLong((unsigned long)p);
    L2 = decodeUnsignedLong(L1);

    printf("L1: %ld   L2: %ld   L1-L2: %ld\n", L1, L2, L1-L2);

    printf("sizeof struct _long %ld\n", sizeof(struct _long));
}