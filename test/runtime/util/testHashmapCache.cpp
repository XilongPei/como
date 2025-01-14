#include <comosp.h>
#include <comoobj.h>
#include <gtest/gtest.h>
#include <hashmapCache.h>

using namespace como;

TEST(HashMapCache, TestPutAndGet)
{
    como::HashMapCache<Integer, String> cache;

    Integer key1 = 1, key2 = 2;
    String value1 = "Value1", value2 = "Value2";

    EXPECT_EQ(cache.Put(key1, value1, 10), 0);
    EXPECT_EQ(cache.Put(key2, value2, 20), 0);

    EXPECT_EQ(cache.Get(key1), value1);
    EXPECT_EQ(cache.Get(key2), value2);
}

TEST(HashMapCache, TestPutDuplicateKey)
{
    como::HashMapCache<Integer, String> cache;

    Integer key = 1;
    String value1 = "Value1", value2 = "Value2";
    EXPECT_EQ(cache.Put(key, value1, 10), 0);
    EXPECT_EQ(cache.Put(key, value2, 10), 0);
    EXPECT_EQ(cache.Get(key), value2);
}

TEST(HashMapCache, TestRemove)
{
    como::HashMapCache<Integer, String> cache;

    Integer key1 = 1, key2 = 2, key3 = 3;
    String value1 = "Value1", value2 = "Value2", value3 = "Value3";
    cache.Put(key1, value1, 10);
    cache.Put(key2, value2, 10);

    EXPECT_EQ(cache.Remove(key1), 1);
    EXPECT_EQ(cache.Remove(key2), 1);
    EXPECT_EQ(cache.Remove(key3), 0);

    EXPECT_EQ(cache.Get(key1), String(0));
    EXPECT_EQ(cache.Get(key2), String(0));
}

TEST(HashMapCache, TestContainsKey)
{
    como::HashMapCache<Integer, String> cache;
    Integer key1 = 1, key2 = 2;
    String value = "Value";

    cache.Put(key1, value, 10);

    EXPECT_EQ(cache.ContainsKey(key1), true);
    EXPECT_EQ(cache.ContainsKey(key2), false);
}

TEST(HashMapCache, TestClear)
{
    como::HashMapCache<Integer, String> cache;

    Integer key1 = 1, key2 = 2;
    String value1 = "Value1", value2 = "Value2";
    cache.Put(key1, value1, 10);
    cache.Put(key2, value2, 20);

    cache.Clear();

    EXPECT_EQ(cache.ContainsKey(key1), false);
    EXPECT_EQ(cache.ContainsKey(key2), false);
}

TEST(HashMapCache, TestCleanUpExpiredData)
{
    como::HashMapCache<Integer, String> cache(5, 2);

    Integer key = 1;
    String value = "Value1";
    cache.Put(key, value, 10);
    sleep(5);

    cache.CleanUpExpiredData();
    // This test failure is expected because CHECK_EXPIRES_PERIOD is defined as 300 in HashMapCache. 
    // If you modify CHECK_EXPIRES_PERIOD to a value smaller than the cleanup time, 
    // it will allow the current test to pass.
    EXPECT_EQ(cache.ContainsKey(key), false);
}

TEST(HashMapCache, TestGetValues)
{
    como::HashMapCache<Integer, String> cache;
    Integer key1 = 1, key2 = 2, key3 = 3;
    String value1 = "Value1", value2 = "Value2", value3 = "Value3";
    cache.Put(key1, value1, 10);
    cache.Put(key2, value2, 20);
    cache.Put(key3, value3, 20);

    Array<String> ret = cache.GetValues();
    EXPECT_EQ(ret.GetLength(), 3);
}

TEST(HashMapCache, TestRemoveByValue)
{
    como::HashMapCache<Integer, String> cache;

    Integer key1 = 1, key2 = 2;
    String value1 = "Value1", value2 = "Value2";
    cache.Put(key1, value1, 10);
    cache.Put(key2, value2, 20);

    EXPECT_EQ(cache.RemoveByValue(10), 1);
    EXPECT_EQ(cache.RemoveByValue(30), 0);

    // Confirm the element has been deleted.
    EXPECT_EQ(cache.Get(1), String(0));
    EXPECT_EQ(cache.Get(2), value2);
}

TEST(HashMapCache, TestRehash)
{
    como::HashMapCache<Integer, String> cache(5);

    Integer key1 = 1, key2 = 2, key3 = 3, key4 = 4, key5 = 5;
    String value1 = "Value1", value2 = "Value2", value3 = "Value3", value4 = "Value4", value5 = "Value5";
    // trigger rehash function
    cache.Put(key1, value1, 10);
    cache.Put(key2, value2, 20);
    cache.Put(key3, value3, 30);
    cache.Put(key4, value4, 40);
    cache.Put(key5, value5, 50);

    EXPECT_EQ(cache.Get(1), value1);
    EXPECT_EQ(cache.Get(2), value2);
    EXPECT_EQ(cache.Get(3), value3);
    EXPECT_EQ(cache.Get(4), value4);
    EXPECT_EQ(cache.Get(5), value5);
}


int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}