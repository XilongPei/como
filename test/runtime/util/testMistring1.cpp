// Some test problems:
// 1. After calling MiString::WordBreak, num isn't 0 if return nullptr.
// 2. In MiString::shrink, no test of src = nullptr or dst = nullstr.
// 3. In MiString::cnStrToStr, the transformation of octal number is not correct.
// 4. In MiString::cnStrToStr, some escape characters don't be transformed, such as '\r', '\\', '\''.

#include <comosp.h>
#include <comoobj.h>
#include <mistring.h>
#include <gtest/gtest.h>

using namespace como;

#define MAX_SIZE 256
#define WORD_MAX_SIZE 10

// Test 1.1: MiString::WordBreak (Special condition)
TEST(Mistring, testWordBreak1)
{
    const char* testStr = "abcdefg hijklmn opq rst uvwxyz";
    char str[MAX_SIZE];
    char br[MAX_SIZE] = " ";         // breakChar
    char* word[WORD_MAX_SIZE];
    char* ret;
    int num;

    // string = nullptr
    memset(word, 0, sizeof(word));
    num = 5;
    ret = MiString::WordBreak(nullptr, num, word, br);
    EXPECT_EQ(ret, nullptr);

    // word = nullptr
    strcpy(str, testStr);
    num = 5;
    ret = MiString::WordBreak(str, num, nullptr, br);
    EXPECT_EQ(ret, nullptr);

    // breakChar = nullptr
    strcpy(str, testStr);
    memset(word, 0, sizeof(word));
    num = 5;
    ret = MiString::WordBreak(str, num, word, nullptr);
    EXPECT_EQ(ret, nullptr);

    // string = ""
    str[0] = '\0';
    memset(word, 0, sizeof(word));
    num = 5;
    ret = MiString::WordBreak(str, num, word, br);
    EXPECT_EQ(num, 0);
    EXPECT_STREQ(ret, "");
}

// Test 1.2: MiString::WordBreak (Different num value)
TEST(Mistring, testWordBreak2)
{
    const char* testStr = "abcdefg hijklmn opq rst uvwxyz";
    char str[MAX_SIZE];
    char br[MAX_SIZE] = " ";         // breakChar
    char* word[WORD_MAX_SIZE];
    char* ret;
    int num;

    // num < 0
    strcpy(str, testStr);
    memset(word, 0, sizeof(word));
    num = -5;
    ret = MiString::WordBreak(str, num, word, br);
    EXPECT_EQ(num, 1);
    EXPECT_STREQ(word[0], "abcdefg");

    // num = 0
    strcpy(str, testStr);
    memset(word, 0, sizeof(word));
    num = 0;
    ret = MiString::WordBreak(str, num, word, br);
    EXPECT_EQ(num, 1);
    EXPECT_STREQ(word[0], "abcdefg");

    // 0 < num < 5
    strcpy(str, testStr);
    memset(word, 0, sizeof(word));
    num = 3;
    ret = MiString::WordBreak(str, num, word, br);
    EXPECT_EQ(num, 3);
    EXPECT_STREQ(word[0], "abcdefg");
    EXPECT_STREQ(word[1], "hijklmn");
    EXPECT_STREQ(word[2], "opq");

    // num = 5
    strcpy(str, testStr);
    memset(word, 0, sizeof(word));
    num = 5;
    ret = MiString::WordBreak(str, num, word, br);
    EXPECT_EQ(num, 5);
    EXPECT_STREQ(word[0], "abcdefg");
    EXPECT_STREQ(word[1], "hijklmn");
    EXPECT_STREQ(word[2], "opq");
    EXPECT_STREQ(word[3], "rst");
    EXPECT_STREQ(word[4], "uvwxyz");

    // num > 5
    strcpy(str, testStr);
    memset(word, 0, sizeof(word));
    num = 10;
    ret = MiString::WordBreak(str, num, word, br);
    EXPECT_EQ(num, 5);
    EXPECT_STREQ(word[0], "abcdefg");
    EXPECT_STREQ(word[1], "hijklmn");
    EXPECT_STREQ(word[2], "opq");
    EXPECT_STREQ(word[3], "rst");
    EXPECT_STREQ(word[4], "uvwxyz");
}

// Test 1.3: MiString::WordBreak (Different breakChar)
TEST(Mistring, testWordBreak3)
{
    const char* testStr = "11111,12345.11111 12345.11111";
    char str[MAX_SIZE];
    char br[MAX_SIZE];         // breakChar
    char* word[WORD_MAX_SIZE];
    char* ret;
    int num;

    // breakChar = ","
    strcpy(br, ",");
    strcpy(str, testStr);
    memset(word, 0, sizeof(word));
    num = 5;
    ret = MiString::WordBreak(str, num, word, br);
    EXPECT_EQ(num, 3);
    EXPECT_STREQ(word[0], "11111");
    EXPECT_STREQ(word[1], "12345.11111");
    EXPECT_STREQ(word[2], "12345.11111");

    // breakChar = "1"
    strcpy(br, "1");
    strcpy(str, testStr);
    memset(word, 0, sizeof(word));
    num = 5;
    ret = MiString::WordBreak(str, num, word, br);
    EXPECT_EQ(num, 3);
    EXPECT_STREQ(word[0], ",");
    EXPECT_STREQ(word[1], "2345.");
    EXPECT_STREQ(word[2], "2345.");

    // breakChar = "6"
    strcpy(br, "6");
    strcpy(str, testStr);
    memset(word, 0, sizeof(word));
    num = 5;
    ret = MiString::WordBreak(str, num, word, br);
    EXPECT_EQ(num, 2);
    EXPECT_STREQ(word[0], "11111,12345.11111");
    EXPECT_STREQ(word[1], "12345.11111");

    // breakChar = ". ,"
    strcpy(br, ". ,");
    strcpy(str, testStr);
    memset(word, 0, sizeof(word));
    num = 5;
    ret = MiString::WordBreak(str, num, word, br);
    EXPECT_EQ(num, 5);
    EXPECT_STREQ(word[0], "11111");
    EXPECT_STREQ(word[1], "12345");
    EXPECT_STREQ(word[2], "11111");
    EXPECT_STREQ(word[3], "12345");
    EXPECT_STREQ(word[4], "11111");

    // breakChar = "12345"
    strcpy(br, "12345");
    strcpy(str, testStr);
    memset(word, 0, sizeof(word));
    num = 5;
    ret = MiString::WordBreak(str, num, word, br);
    EXPECT_EQ(num, 3);
    EXPECT_STREQ(word[0], ",");
    EXPECT_STREQ(word[1], ".");
    EXPECT_STREQ(word[2], ".");
}

// Test 2.1: MiString::SeperateStr (Return char **) (Special condition)
TEST(Mistring, testSeperateStr1)
{
    const char* testStr = "abcdefg hijklmn  opq   rst    uvwxyz";
    char str[MAX_SIZE];
    char seperator = ' ';
    char* seeds[WORD_MAX_SIZE];
    char** ret;
    int seedsCapacity;

    // str = nullptr
    memset(seeds, 0, sizeof(seeds));
    seedsCapacity = 5;
    ret = MiString::SeperateStr(nullptr, seperator, seeds, seedsCapacity);
    printf("OK\n");
    EXPECT_EQ(seedsCapacity, 0);

    // seeds = nullptr
    strcpy(str, testStr);
    char** nullSeeds = nullptr;
    seedsCapacity = 5;
    ret = MiString::SeperateStr(str, seperator, nullSeeds, seedsCapacity);
    printf("OK\n");
    EXPECT_EQ(seedsCapacity, 5);
    EXPECT_STREQ(seeds[0], "abcdefg");
    EXPECT_STREQ(seeds[1], "hijklmn");
    EXPECT_STREQ(seeds[2], "opq");
    EXPECT_STREQ(seeds[3], "rst");
    EXPECT_STREQ(seeds[4], "uvwxyz");
    if (nullSeeds != nullptr)
        free(nullSeeds);
}

// Test 2.2: MiString::SeperateStr (Return char **) (Different seedsCapacity value)
TEST(Mistring, testSeperateStr2)
{
    const char* testStr = "abcdefg hijklmn  opq   rst    uvwxyz";
    char str[MAX_SIZE];
    char seperator = ' ';
    char* seeds[WORD_MAX_SIZE];
    char** ret;
    int seedsCapacity;

    // seedsCapacity < 0
    strcpy(str, testStr);
    memset(seeds, 0, sizeof(seeds));
    seedsCapacity = -5;
    ret = MiString::SeperateStr(str, seperator, seeds, seedsCapacity);
    EXPECT_EQ(seedsCapacity, 0);

    // seedsCapacity = 0
    strcpy(str, testStr);
    memset(seeds, 0, sizeof(seeds));
    seedsCapacity = 0;
    ret = MiString::SeperateStr(str, seperator, seeds, seedsCapacity);
    EXPECT_EQ(seedsCapacity, 0);

    // 0 < seedsCapacity < 5
    strcpy(str, testStr);
    memset(seeds, 0, sizeof(seeds));
    seedsCapacity = 3;
    ret = MiString::SeperateStr(str, seperator, seeds, seedsCapacity);
    EXPECT_EQ(seedsCapacity, 3);
    EXPECT_STREQ(seeds[0], "abcdefg");
    EXPECT_STREQ(seeds[1], "hijklmn");
    EXPECT_STREQ(seeds[2], "opq");

    // seedsCapacity = 5
    strcpy(str, testStr);
    memset(seeds, 0, sizeof(seeds));
    seedsCapacity = 5;
    ret = MiString::SeperateStr(str, seperator, seeds, seedsCapacity);
    EXPECT_EQ(seedsCapacity, 5);
    EXPECT_STREQ(seeds[0], "abcdefg");
    EXPECT_STREQ(seeds[1], "hijklmn");
    EXPECT_STREQ(seeds[2], "opq");
    EXPECT_STREQ(seeds[3], "rst");
    EXPECT_STREQ(seeds[4], "uvwxyz");

    // seedsCapacity > 5
    strcpy(str, testStr);
    memset(seeds, 0, sizeof(seeds));
    seedsCapacity = 10;
    ret = MiString::SeperateStr(str, seperator, seeds, seedsCapacity);
    EXPECT_EQ(seedsCapacity, 5);
    EXPECT_STREQ(seeds[0], "abcdefg");
    EXPECT_STREQ(seeds[1], "hijklmn");
    EXPECT_STREQ(seeds[2], "opq");
    EXPECT_STREQ(seeds[3], "rst");
    EXPECT_STREQ(seeds[4], "uvwxyz");
}

// Test 2.3: MiString::SeperateStr (Return char **) (Different seperator)
TEST(Mistring, testSeperateStr3)
{
    const char* testStr = "11111,12345.11111 12345.11111";
    char str[MAX_SIZE];
    char seperator;
    char* seeds[WORD_MAX_SIZE];
    char** ret;
    int seedsCapacity;

    // seperator = ','
    strcpy(str, testStr);
    memset(seeds, 0, sizeof(seeds));
    seperator = ',';
    seedsCapacity = 5;
    ret = MiString::SeperateStr(str, seperator, seeds, seedsCapacity);
    EXPECT_EQ(seedsCapacity, 2);
    EXPECT_STREQ(seeds[0], "11111");
    EXPECT_STREQ(seeds[1], "12345.11111 12345.11111");

    // seperator = '1'
    strcpy(str, testStr);
    memset(seeds, 0, sizeof(seeds));
    seperator = '1';
    seedsCapacity = 5;
    ret = MiString::SeperateStr(str, seperator, seeds, seedsCapacity);
    EXPECT_EQ(seedsCapacity, 4);
    EXPECT_STREQ(seeds[0], ",");
    EXPECT_STREQ(seeds[1], "2345.");
    EXPECT_STREQ(seeds[2], " ");
    EXPECT_STREQ(seeds[3], "2345.");

    // seperator = '6'
    strcpy(str, testStr);
    memset(seeds, 0, sizeof(seeds));
    seperator = '6';
    seedsCapacity = 5;
    ret = MiString::SeperateStr(str, seperator, seeds, seedsCapacity);
    // EXPECT_EQ(seedsCapacity, 1);
    EXPECT_STREQ(seeds[0], "11111,12345.11111 12345.11111");
}

// Test 2.4: MiString::SeperateStr (Return ECode) (Special condition)
TEST(Mistring, testSeperateStr4)
{
    const char* testStr = "abcdefg hijklmn  opq   rst    uvwxyz";
    char str[MAX_SIZE];
    char seperator = ' ';
    Integer limit;
    Array<String> arr;
    ECode ret;

    // str = nullptr
    limit = 5;
    ret = MiString::SeperateStr(nullptr, seperator, limit, &arr);
    EXPECT_NE(ret, NOERROR);

    // strArray = nullptr
    strcpy(str, testStr);
    Array<String>* nullArr = nullptr;
    limit = 5;
    ret = MiString::SeperateStr(str, seperator, limit, nullArr);
    EXPECT_NE(ret, NOERROR);
}

// Test 2.5: MiString::SeperateStr (Return ECode) (Different limit value)
TEST(Mistring, testSeperateStr5)
{
    const char* testStr = "abcdefg hijklmn  opq   rst    uvwxyz";
    char str[MAX_SIZE];
    char seperator = ' ';
    Integer limit;
    Array<String> arr;
    ECode ret;

    // limit < 0
    strcpy(str, testStr);
    arr.Clear();
    limit = -5;
    ret = MiString::SeperateStr(str, seperator, limit, &arr);
    EXPECT_EQ(ret, NOERROR);
    EXPECT_EQ(arr.GetLength(), 5);
    EXPECT_STREQ(arr[0], "abcdefg");
    EXPECT_STREQ(arr[1], "hijklmn");
    EXPECT_STREQ(arr[2], "opq");
    EXPECT_STREQ(arr[3], "rst");
    EXPECT_STREQ(arr[4], "uvwxyz");

    // limit = 0
    strcpy(str, testStr);
    arr.Clear();
    limit = 0;
    ret = MiString::SeperateStr(str, seperator, limit, &arr);
    EXPECT_EQ(ret, NOERROR);
    EXPECT_EQ(arr.GetLength(), 5);
    EXPECT_STREQ(arr[0], "abcdefg");
    EXPECT_STREQ(arr[1], "hijklmn");
    EXPECT_STREQ(arr[2], "opq");
    EXPECT_STREQ(arr[3], "rst");
    EXPECT_STREQ(arr[4], "uvwxyz");

    // 0 < limit < 5
    strcpy(str, testStr);
    arr.Clear();
    limit = 3;
    ret = MiString::SeperateStr(str, seperator, limit, &arr);
    EXPECT_EQ(ret, NOERROR);
    EXPECT_EQ(arr.GetLength(), 3);
    EXPECT_STREQ(arr[0], "abcdefg");
    EXPECT_STREQ(arr[1], "hijklmn");
    EXPECT_STREQ(arr[2], "opq");

    // limit = 5
    strcpy(str, testStr);
    arr.Clear();
    limit = 5;
    ret = MiString::SeperateStr(str, seperator, limit, &arr);
    EXPECT_EQ(ret, NOERROR);
    EXPECT_EQ(arr.GetLength(), 5);
    EXPECT_STREQ(arr[0], "abcdefg");
    EXPECT_STREQ(arr[1], "hijklmn");
    EXPECT_STREQ(arr[2], "opq");
    EXPECT_STREQ(arr[3], "rst");
    EXPECT_STREQ(arr[4], "uvwxyz");

    // limit > 5
    strcpy(str, testStr);
    arr.Clear();
    limit = 10;
    ret = MiString::SeperateStr(str, seperator, limit, &arr);
    EXPECT_EQ(ret, NOERROR);
    EXPECT_EQ(arr.GetLength(), 5);
    EXPECT_STREQ(arr[0], "abcdefg");
    EXPECT_STREQ(arr[1], "hijklmn");
    EXPECT_STREQ(arr[2], "opq");
    EXPECT_STREQ(arr[3], "rst");
    EXPECT_STREQ(arr[4], "uvwxyz");
}

// Test 2.6: MiString::SeperateStr (Return ECode) (Different seperator)
TEST(Mistring, testSeperateStr6)
{
    const char* testStr = "11111,12345.11111 12345.11111";
    char str[MAX_SIZE];
    char seperator;
    Integer limit;
    Array<String> arr;
    ECode ret;

    // seperator = ','
    strcpy(str, testStr);
    arr.Clear();
    seperator = ',';
    limit = 5;
    ret = MiString::SeperateStr(str, seperator, limit, &arr);
    EXPECT_EQ(ret, NOERROR);
    EXPECT_EQ(arr.GetLength(), 2);
    EXPECT_STREQ(arr[0], "11111");
    EXPECT_STREQ(arr[1], "12345.11111 12345.11111");

    // seperator = '1'
    strcpy(str, testStr);
    arr.Clear();
    seperator = ',';
    limit = 5;
    ret = MiString::SeperateStr(str, seperator, limit, &arr);
    EXPECT_EQ(ret, NOERROR);
    EXPECT_EQ(arr.GetLength(), 4);
    EXPECT_STREQ(arr[0], ",");
    EXPECT_STREQ(arr[1], "2345.");
    EXPECT_STREQ(arr[2], " ");
    EXPECT_STREQ(arr[3], "2345.");

    // seperator = '6'
    strcpy(str, testStr);
    arr.Clear();
    seperator = ',';
    limit = 5;
    ret = MiString::SeperateStr(str, seperator, limit, &arr);
    EXPECT_EQ(ret, NOERROR);
    EXPECT_EQ(arr.GetLength(), 1);
    EXPECT_STREQ(arr[0], "11111,12345.11111 12345.11111");
}

// Test 3: MiString::shrink
TEST(Mistring, testshrink)
{
    const char* testStr = "   1234   5678   9012  3456 7890  ";
    char src[MAX_SIZE];
    char dst[MAX_SIZE];
    int dstSize;
    char* ret;

    // no test of src = nullptr or dst = nullstr 

    // src = ""
    strcpy(src, "");
    dstSize = MAX_SIZE;
    ret = MiString::shrink(dst, dstSize, src);
    EXPECT_STREQ(dst, "");

    // src is composed with space
    strcpy(src, "      ");
    dstSize = MAX_SIZE;
    ret = MiString::shrink(dst, dstSize, src);
    EXPECT_STREQ(dst, "");

    // dstSize is not enough
    strcpy(src, testStr);
    dstSize = 5;
    ret = MiString::shrink(dst, dstSize, src);
    EXPECT_STREQ(dst, "12345");

    // Normal condition
    strcpy(src, testStr);
    dstSize = MAX_SIZE;
    ret = MiString::shrink(dst, dstSize, src);
    EXPECT_STREQ(dst, "12345678901234567890");
}

// Test 4: MiString::cnStrToStr
TEST(Mistring, testcnStrToStr)
{
    const char* testStr = "abcdefg_nhijklmn_topq_\"rst_\"";
    char s[MAX_SIZE];
    char t[MAX_SIZE];
    char turnChar;
    int size;
    char* ret;

    // no test of s = nullptr and t = nullstr 

    // s = ""
    strcpy(s, "");
    turnChar = '_';
    size = MAX_SIZE;
    ret = MiString::cnStrToStr(t, s, turnChar, size);
    EXPECT_STREQ(t, "");

    // Normal condition
    strcpy(s, testStr);
    turnChar = '_';
    size = MAX_SIZE;
    ret = MiString::cnStrToStr(t, s, turnChar, size);
    EXPECT_STREQ(t, "abcdefg\nhijklmn\topq\"rst\"");

    // turnChar = 's'
    strcpy(s, testStr);
    turnChar = 's';
    size = MAX_SIZE;
    ret = MiString::cnStrToStr(t, s, turnChar, size);
    EXPECT_STREQ(t, "abcdefg_nhijklmn_topq_\"r\t_\"");

    // size is not enough
    strcpy(s, testStr);
    turnChar = '_';
    size = 20;
    ret = MiString::cnStrToStr(t, s, turnChar, size);
    EXPECT_STREQ(t, "abcdefg\nhijklmn\topq\"");

    // s includes '\0'
    strcpy(s, "abcdefg_\0hijklmn");
    turnChar = '_';
    size = MAX_SIZE;
    ret = MiString::cnStrToStr(t, s, turnChar, size);
    EXPECT_STREQ(t, "abcdefg");

    // Use number escape characters
    strcpy(s, "abc_x61_x62_x63_101_102_103");
    turnChar = '_';
    size = MAX_SIZE;
    ret = MiString::cnStrToStr(t, s, turnChar, size);
    EXPECT_STREQ(t, "abc\x61\x62\x63\101\102\103");
}

// Test 5: MiString::strZcpy
TEST(Mistring, teststrZcpy)
{
    const char* testStr = "1234567890";
    char src[MAX_SIZE];
    char dest[MAX_SIZE];
    int maxlen;
    char* ret;

    // Normal condition
    strcpy(src, testStr);
    maxlen = MAX_SIZE;
    ret = MiString::strZcpy(dest, src, maxlen);
    EXPECT_STREQ(dest, "1234567890");

    // maxlen < src length
    strcpy(src, testStr);
    maxlen = 5;
    ret = MiString::strZcpy(dest, src, maxlen);
    EXPECT_STREQ(dest, "12345");

    // src = ""
    strcpy(src, "");
    maxlen = MAX_SIZE;
    ret = MiString::strZcpy(dest, src, maxlen);
    EXPECT_STREQ(dest, "");
}

// Test 6: MiString::strToSourceC
TEST(Mistring, teststrToSourceC)
{
    const char* testStr = "12\\34\\56\\78\\90";
    char s[MAX_SIZE];
    char t[MAX_SIZE];
    char turnChar;
    char* ret;

    strcpy(s, testStr);
    turnChar = '\\';
    ret = MiString::strToSourceC(t, s, turnChar);
    EXPECT_STREQ(t, "12\\\\34\\\\56\\\\78\\\\90");
}

// Test 7: MiString::memNewBlockOnce
TEST(Mistring, testmemNewBlockOnce)
{
    const size_t BUF_SIZE = 4096;
    char *s1, *s2, *s3, *s4, *s;
    size_t poolSize;
    char buf[BUF_SIZE];

    // pool = nullptr
    poolSize = BUF_SIZE;
    s = MiString::memNewBlockOnce(nullptr, &poolSize, &s1, 10, &s2, 20, &s3, 30, &s4, 40, nullptr);
    EXPECT_NE(s, nullptr);
    EXPECT_EQ(poolSize, 100);
    if (s != nullptr)
        free(s);

    // poolSize = nullptr
    s = MiString::memNewBlockOnce(buf, nullptr, &s1, 10, &s2, 20, &s3, 30, &s4, 40, nullptr);
    EXPECT_NE(s, nullptr);

    // ss = nullptr
    /*
    poolSize = BUF_SIZE;
    s = MiString::memNewBlockOnce(buf, &poolSize, nullptr, nullptr, nullptr);
    EXPECT_EQ(s, nullptr);
    */

    // total memSize <= 0
    poolSize = BUF_SIZE;
    s = MiString::memNewBlockOnce(buf, &poolSize, &s1, 10, &s2, -20, &s3, 30, &s4, -40, nullptr);
    EXPECT_EQ(s, nullptr);

    // Normal condition
    poolSize = BUF_SIZE;
    s = MiString::memNewBlockOnce(buf, &poolSize, &s1, 10, &s2, 20, &s3, 30, &s4, 40, nullptr);
    EXPECT_NE(s, nullptr);

    // poolSize < total memSize
    poolSize = 50;
    s = MiString::memNewBlockOnce(buf, &poolSize, &s1, 10, &s2, 20, &s3, 30, &s4, 40, nullptr);
    EXPECT_EQ(s, nullptr);
}

// Test 8: MiString::memGetBlockOnce
TEST(Mistring, testmemGetBlockOnce)
{
    const size_t BUF_SIZE = 4096;
    char input1 = 'A', output1;
    short input2 = 12345, output2;
    int input3 = 45678, output3;
    long long input4 = 91012, output4;
    char input5[20] = "Hello, world!";
    char output5[20];
    char buf[BUF_SIZE];
    size_t poolSize;
    char* s;

    char* p = buf;
    memcpy(p, &input1, sizeof(input1));
    p += sizeof(input1);
    memcpy(p, &input2, sizeof(input2));
    p += sizeof(input2);
    memcpy(p, &input3, sizeof(input3));
    p += sizeof(input3);
    memcpy(p, &input4, sizeof(input4));
    p += sizeof(input4);
    memcpy(p, &input5, sizeof(input5));

    // buf = nullptr
    poolSize = BUF_SIZE;
    output1 = 0;
    output2 = 0;
    output3 = 0;
    output4 = 0;
    memset(output5, 0, 20);
    s = MiString::memGetBlockOnce(nullptr, poolSize, &output1, sizeof(output1), (char *)&output2, sizeof(output2), 
                                  (char *)&output3, sizeof(output3), (char *)&output4, sizeof(output4), 
                                  &output5, sizeof(output5), nullptr);
    EXPECT_EQ(s, nullptr);

    // Normal condition
    poolSize = BUF_SIZE;
    output1 = 0;
    output2 = 0;
    output3 = 0;
    output4 = 0;
    memset(output5, 0, 20);
    s = MiString::memGetBlockOnce(buf, poolSize, &output1, sizeof(output1), (char *)&output2, sizeof(output2), 
                                  (char *)&output3, sizeof(output3), (char *)&output4, sizeof(output4), 
                                  &output5, sizeof(output5), nullptr);
    EXPECT_NE(s, nullptr);
    EXPECT_EQ(output1, input1);
    EXPECT_EQ(output2, input2);
    EXPECT_EQ(output3, input3);
    EXPECT_EQ(output4, input4);
    EXPECT_STREQ(output5, input5);

    // poolsize < total memSize
    poolSize = 5;
    output1 = 0;
    output2 = 0;
    output3 = 0;
    output4 = 0;
    memset(output5, 0, 20);
    s = MiString::memGetBlockOnce(buf, poolSize, &output1, sizeof(output1), (char *)&output2, sizeof(output2), 
                                  (char *)&output3, sizeof(output3), (char *)&output4, sizeof(output4), 
                                  &output5, sizeof(output5), nullptr);
    EXPECT_NE(s, nullptr);
    EXPECT_EQ(output1, input1);
    EXPECT_EQ(output2, input2);
    EXPECT_EQ(output3, input3);
    EXPECT_EQ(output4, 0);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
