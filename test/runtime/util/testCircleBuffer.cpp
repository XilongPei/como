#include <comosp.h>
#include <comoobj.h>
#include <CircleBuffer.h>
#include <gtest/gtest.h>

using namespace como;

#define MAX_SIZE 200

// Test 1: CircleBuffer constructor
TEST(CircleBuffer, testConstructor)
{
    CircleBuffer<char> *circleBuf;

    circleBuf = new CircleBuffer<char>(MAX_SIZE);
    ASSERT_NE(nullptr, circleBuf);

    delete circleBuf;
}

// Test 2: CircleBuffer::Write
TEST(CircleBuffer, testWrite)
{
    CircleBuffer<char> *circleBuf;

    circleBuf = new CircleBuffer<char>(MAX_SIZE);
    ASSERT_NE(nullptr, circleBuf);

    const char testBuf[20] = "1234567890abcdeABCD";
    int ret;

    // count <= 0
    ret = circleBuf->Write(testBuf, 0);
    EXPECT_EQ(ret, 0);
    circleBuf->Clear();

    // count > 200 
    // (Only data less than half the capacity can be written)
    ret = circleBuf->Write(testBuf, 300);
    EXPECT_EQ(ret, 0);
    circleBuf->Clear();

    // Write 20-size testBuf * 10 into circleBuf.
    for (int i = 0; i < 10; i++) {
        ret = circleBuf->Write(testBuf, 20);
        EXPECT_EQ(ret, 20);
    }

    // After circleBuf is full, write 10-size testBuf * 10 into circleBuf.
    for (int i = 0; i < 10; i++) {
        ret = circleBuf->Write(testBuf, 10);
        EXPECT_EQ(ret, 10);
    }

    delete circleBuf;
}

// Test 3: CircleBuffer::Read
TEST(CircleBuffer, testRead)
{
    CircleBuffer<char> *circleBuf;

    circleBuf = new CircleBuffer<char>(MAX_SIZE);
    ASSERT_NE(nullptr, circleBuf);

    char testBuf[20] = "1234567890abcdeABCD";
    char buf[MAX_SIZE];
    int ret;

    // circleBuf is empty.
    ret = circleBuf->Read(testBuf, 20);
    EXPECT_EQ(ret, 0);
    circleBuf->Clear();

    // Write 20-size testBuf * 10 into circleBuf.
    for (int i = 0; i < 10; i++) {
        ret = circleBuf->Write(testBuf, 20);
    }

    // count <= 0
    ret = circleBuf->Read(testBuf, 0);
    EXPECT_EQ(ret, 0);
    circleBuf->Clear();

    // Read 10-size buf * 20 from circleBuf.
    buf[10] = '\0';
    for (int i = 0; i < 20; i++) {
        ret = circleBuf->Read(buf, 10);
        EXPECT_EQ(ret, 10);
        if (i % 2 == 0)
            EXPECT_STREQ(buf, "1234567890");
        else
            EXPECT_STREQ(buf, "abcdeABCD");
        if (i < 19)
            EXPECT_EQ(circleBuf->isEmpty(), false);
        else
            EXPECT_EQ(circleBuf->isEmpty(), true);
    }

    delete circleBuf;
}

// Test 4: CircleBuffer::DiscardedRead
TEST(CircleBuffer, testDiscardedRead)
{
    CircleBuffer<char> *circleBuf;

    circleBuf = new CircleBuffer<char>(MAX_SIZE);
    ASSERT_NE(nullptr, circleBuf);

    const char testBuf[20] = "1234567890abcdeABCD";
    char buf[MAX_SIZE];
    int ret;

    // (1) Write 14-size testBuf into circleBuf.
    // (2) DiscardedRead 10 character.
    // (3) Write 16-size testBuf into circleBuf.
    // (1)->(2)->(3)->(1)->(2)->(3)->... This loop will execute 10 times.
    for (int i = 0; i < 10; i++) {
        if(i % 2 == 0) {
            ret = circleBuf->Write(testBuf, 14);
            EXPECT_EQ(ret, 14);
            ret = circleBuf->DiscardedRead(10);
            EXPECT_EQ(ret, 10);
        }
        else {
            ret = circleBuf->Write(testBuf, 16);
            EXPECT_EQ(ret, 16);
        }
    }

    // Read 20-size buf * 10 from circleBuf.
    buf[20] = '\0';
    for (int i = 0; i < 10; i++) {
        ret = circleBuf->Read(buf, 20);
        EXPECT_STREQ(buf, "12341234567890abcdeA");
    }
    EXPECT_EQ(circleBuf->isEmpty(), true);

    delete circleBuf;
}

// Test 5: CircleBuffer::Write(String), CircleBuffer::ReadString
TEST(CircleBuffer, testWRString)
{
    CircleBuffer<char> *circleBuf;

    circleBuf = new CircleBuffer<char>(MAX_SIZE);
    ASSERT_NE(nullptr, circleBuf);

    como::String testBuf = "123456789\0abcdeABCD";
    como::String buf;
    int ret;

    // Write 20-size testBuf * 10 into circleBuf.
    for (int i = 0; i < 10; i++) {
        ret = circleBuf->Write(testBuf);
        EXPECT_EQ(ret, 20);
    }

    // Read 10-size buf * 20 from circleBuf.
    for (int i = 0; i < 10; i++) {
        ret = circleBuf->ReadString(buf);
        EXPECT_EQ(ret, 10);
        if (i % 2 == 0)
            EXPECT_STREQ(buf, "123456789");
        else
            EXPECT_STREQ(buf, "abcdeABCD");
        if (i < 19)
            EXPECT_EQ(circleBuf->isEmpty(), false);
        else
            EXPECT_EQ(circleBuf->isEmpty(), true);
    }

    delete circleBuf;
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}