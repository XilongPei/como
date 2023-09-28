#include <comosp.h>
#include <comoobj.h>
#include <ThreadStack.h>
#include <gtest/gtest.h>
#include <iostream>

using namespace como;
using namespace std;

TEST(ThreadStack, testThreadStack)
{
    void *addr;
    size_t size;

    int ret = ThreadStack::GetStack(&addr, &size);
    EXPECT_EQ(ret, 0);
    printf("GetStack addr: %p  size: %lx\n", addr, size);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
