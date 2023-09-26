#include <comosp.h>
#include <comoobj.h>
#include <mistring.h>
#include <gtest/gtest.h>
#include <iostream>

using namespace como;
using namespace std;

TEST(Mistring, testWordBreak1)
{
    char* str = nullptr;
    char* word[256];
    char br[256];
    int num = 5;
    strcpy(br, "-");
    //test when str == nullptr
    char *ret = MiString::WordBreak(str, num, word, br);
    EXPECT_EQ(ret, nullptr);

    //test when word == nullptr
    char ** word1 = nullptr;
    char str1[256] = "187-1778-9376";
    ret = MiString::WordBreak(str1, num, word1, br);
    EXPECT_EQ(ret, nullptr);

    //test when breakChar == nullptr
    char *word2[256];
    char str2[256] = "187-1778-9376";
    char* br2 = nullptr;
    ret = MiString::WordBreak(str2, num, word2, br2);
    EXPECT_EQ(ret, nullptr);
}

TEST(Mistring, testWordBreak2)
{
    //test when str is ""
    char str[256] = "";
    char *word[256];
    char br[256] = "_";
    int num = 5;
    MiString::WordBreak(str, num, word, br);
    EXPECT_EQ(num, 0);
    EXPECT_STREQ(word[0], "");

    //test when num <= 0
    strcpy(str, "hello_world");
    num = 0;
    MiString::WordBreak(str, num, word, br);
    EXPECT_STREQ(word[0], "hello");

    num = -25;
    MiString::WordBreak(str, num, word, br);
    EXPECT_STREQ(word[0], "hello");

    //test space conditions
    num = 10;
    strcpy(str, "Tiger in me, sniff  the   rose");
    strcpy(br, ",");
    MiString::WordBreak(str, num, word, br);
    EXPECT_EQ(num, 6);
}

TEST(Mistring, testWordBreak3)
{
    char str[256] = "  name: lsx_email: 1215291835@qq.com_phone-number: 187-1778-9376";
    char *word[256];
    char br[256] = "_:";
    int num = 10;
    MiString::WordBreak(str, num, word, br);
    EXPECT_EQ(num, 6);
    EXPECT_STREQ(word[0], "name");
    EXPECT_STREQ(word[1], "lsx");
    EXPECT_STREQ(word[2], "email");
    EXPECT_STREQ(word[3], "1215291835@qq.com");
    EXPECT_STREQ(word[4], "phone-number");
    EXPECT_STREQ(word[5], "187-1778-9376");
}

TEST(Mistring, testWordBreak4)
{
    char str[256] = "____123:_:_:2333:__::qwerr";
    char *word[256];
    char br[256] = "_:";
    int num = 10;

    //printf("%s\n", str);
    MiString::WordBreak(str, num, word, br);
    //printf("%s\n%s\n%s\n", word[0], word[1], word[2]);
    EXPECT_EQ(num, 3);
}

TEST(Mistring, testSeperateStr1)
{
    // test when str has nothing
    char str[256] = "";
    int volume;
    char** ret = MiString::SeperateStr(str, '-', nullptr, volume);
    EXPECT_EQ(volume, 0);

    //test when str == nullptr
    char *str1 = nullptr;
    ret = MiString::SeperateStr(str, '-', nullptr, volume);
    EXPECT_EQ(volume, 0);
}

TEST(Mistring, testSeperateStr2)
{
    char str[256] = "-12-34-5678-90-";
    int volume;

    //cout << str << endl;
    char** ret = MiString::SeperateStr(str, '-', nullptr, volume);
    /*
    for (int i = 0; i < volume; i++) {
        cout << ret[i] << endl;
    }
    */

    EXPECT_EQ(volume, 4);
}

TEST(Mistring, testSeperateStr3)
{
    char str[256] = "---12-34--5678--90--";
    int volume;
    char** ret = MiString::SeperateStr(str, '-', nullptr, volume);
    EXPECT_EQ(volume, 4);
}

TEST(Mistring, testSeperateStr4)
{
    char str[256] = "--12-34-----56-78---90";
    Integer limit = 5;
    Array<String>* arr = new Array<String>();
    auto ret = MiString::SeperateStr(str, '-', limit, arr);
    EXPECT_EQ(ret, NOERROR);
}

TEST(Mistring, testShrink1)
{
    //test when src == nullptr
    char buf[256];
    // const char* str = nullptr;
    // MiString::shrink(buf, 256, str);

    //test when dst == nullptr


    //test when src is composed with space
    const char *str1 = "           ";
    MiString::shrink(buf, 256, str1);
    EXPECT_STREQ(buf, "");

    //test when dstSize is not enough
    const char* str2 = "t  e  s  t   c a   s    e";
    MiString::shrink(buf, 5, str2);
    EXPECT_STREQ(buf, "test");
}

TEST(Mistring, testShrink2)
{
    char buf[256];
    const char str[256] = "   abd 123 544   1d3 &&  @#$  iiol 57123**";
    MiString::shrink(buf, 256, str);
    EXPECT_STREQ(buf, "abd1235441d3&&@#$iiol57123**");
}

TEST(Mistring, testShrink3)
{
    char buf[256];
    memset(buf, '\0', sizeof buf);
    const char str[256] = "  abc  bc a";
    MiString::shrink(buf, 11, str);
    EXPECT_STREQ(buf, "abcbca");
}

TEST(Mistring, testcnStrToStr)
{
    char t[256], s[256];
    strcpy(s, "abd_t_n");
    MiString::cnStrToStr(t, s, '_', 256);
    EXPECT_STREQ(t, "abd\t\n");
}

TEST(Mistring, teststrZcpy)
{
    char t[256], s[256];
    strcpy(s, "hello world!");
    MiString::strZcpy(t, s, 256);
    EXPECT_STREQ(t, "hello world!");
    EXPECT_EQ(strlen(t), 12);

    //test when s == nullptr
    // char t1[256];
    // char *s1 = nullptr;
    // MiString::strZcpy(t1, s1, 256);
    // EXPECT_EQ(strlen(t1), 0);

    //test when s == ""
    char t2[256], s2[256];
    strcpy(s2, "");
    MiString::strZcpy(t2, s2, 256);
    EXPECT_EQ(strlen(t2), 0);
}

TEST(Mistring, testMemNewBlockOnce)
{
    char *s1, *s2, *s3, *s4, *s;
    size_t poolSize;
    s = MiString::memNewBlockOnce(nullptr, &poolSize, &s1, 10, &s2, 20, &s3, 30, &s4, 40, nullptr);
    EXPECT_NE(s, nullptr);

    poolSize = 4096;
    char buf[4096];
    s = MiString::memNewBlockOnce(buf, &poolSize, &s1, 10, &s2, 20, &s3, 30, &s4, 40, nullptr);
    EXPECT_NE(s, nullptr);

    poolSize = 8;
    s = MiString::memNewBlockOnce(buf, &poolSize, &s1, 10, &s2, 20, &s3, 30, &s4, 40, nullptr);
    EXPECT_EQ(s, nullptr);
}

TEST(Mistring, memGetBlockOnce)
{
    char *s;
    int  i;
    char buf[4096];
    char buf_s1[32];

    strcpy(buf, "tongji");
    *(int *)&buf[7] = 4800;
    s = MiString::memGetBlockOnce(buf, 4096, buf_s1, 0, (char*)&i, sizeof(int), nullptr);
    EXPECT_NE(s, nullptr);

    //printf("===: %s %d\n", buf_s1, i);
    EXPECT_STREQ("tongji", (char*)buf_s1);
    EXPECT_EQ(4800, i);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
