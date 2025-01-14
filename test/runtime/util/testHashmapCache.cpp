#include <comosp.h>
#include <comoobj.h>
#include <gtest/gtest.h>
#include <hashmapCache.h>

using namespace como;

TEST(HashMapCache, TestPutAndGet)
{
    como::HashMapCache<Integer, String> cache;

    String value1 = "Value1";
    String value2 = "Value2";
    Integer i1 = 1;
    Integer i2 = 2;

    EXPECT_EQ(cache.Put(i1, value1, 10), 0);
    EXPECT_EQ(cache.Put(i2, value2, 20), 0);

    EXPECT_EQ(cache.Get(i1), value1);
    EXPECT_EQ(cache.Get(i2), value2);
}

// TEST(HashMapCache, TestPutDuplicateKey)
// {
//     como::HashMapCache<int*, std::string*> cache;

//     int key = 1;
//     std::string value1 = "Value1", value2 = "Value2";
//     EXPECT_EQ(cache.Put(&key, &value1, 10), 0);
//     EXPECT_EQ(cache.Put(&key, &value2, 10), 0);
//     EXPECT_EQ(cache.Get(&key), &value2);
// }

// TEST(HashMapCache, TestRemove)
// {
//     como::HashMapCache<int*, std::string*> cache;

//     int key1 = 1, key2 = 2, key3 = 3;
//     int *pk1 = &key1, *pk2 = &key2, *pk3 = &key3;
//     std::string value1 = "Value1", value2 = "Value2";
//     std::string *pv1 = &value1, *pv2 = &value2;
//     cache.Put(pk1, pv1, 10);
//     cache.Put(pk2, pv2, 10);

//     EXPECT_EQ(cache.Remove(pk1), 1);
//     EXPECT_EQ(cache.Remove(pk2), 1);
//     EXPECT_EQ(cache.Remove(pk3), 0);

//     EXPECT_EQ(cache.Get(pk1), std::string(0));
//     EXPECT_EQ(cache.Get(pk2), std::string(0));
// }

// TEST(HashMapCache, TestContainsKey)
// {
//     como::HashMapCache<int, std::string> cache;

//     cache.Put(1, "Value1", 10);

//     assert(cache.ContainsKey(1) == true);
//     assert(cache.ContainsKey(2) == false);
// }

// TEST(HashMapCache, TestClear)
// {
//     como::HashMapCache<int, std::string> cache;

//     cache.Put(1, "Value1", 10);
//     cache.Put(2, "Value2", 20);

//     cache.Clear();

//     assert(cache.Get(1) == "");
//     assert(cache.Get(2) == "");
// }

// TEST(HashMapCache, TestCleanUpExpiredData)
// {
//     como::HashMapCache<int, std::string> cache(10, 1);

//     cache.Put(1, "Value1", 10);
//     sleep(2);

//     assert(cache.CleanUpExpiredData() == 1);
//     assert(cache.Get(1) == "");
// }

// void TestGetValues()
// {
//     como::HashMapCache<int, std::string> cache;

//     // 插入元素
//     cache.Put(1, "Value1", 10);
//     cache.Put(2, "Value2", 20);
//     cache.Put(3, "Value3", 30);

//     // 获取所有值
//     auto values = cache.GetValues();
//     assert(values.GetLength() == 3);  // 应该返回3个值
//     assert(values.Get(0) == "Value1");
//     assert(values.Get(1) == "Value2");
//     assert(values.Get(2) == "Value3");
// }

// void TestRemoveByValue()
// {
//     como::HashMapCache<int, std::string> cache;

//     // 插入元素
//     cache.Put(1, "Value1", 10);
//     cache.Put(2, "Value2", 20);

//     // 删除指定值的元素
//     assert(cache.RemoveByValue(10) == 1);  // 删除值为10的元素
//     assert(cache.RemoveByValue(30) == 0);  // 没有值为30的元素

//     // 确认删除后的结果
//     assert(cache.Get(1) == "");  // 键1的元素已删除
//     assert(cache.Get(2) == "Value2");  // 键2的元素未删除
// }

// void TestRehash()
// {
//     como::HashMapCache<int, std::string> cache(2);  // 初始容量为2

//     // 插入元素，触发重新哈希
//     cache.Put(1, "Value1", 10);
//     cache.Put(2, "Value2", 20);
//     cache.Put(3, "Value3", 30);  // 会触发重新哈希

//     // 确认所有元素仍然存在
//     assert(cache.Get(1) == "Value1");
//     assert(cache.Get(2) == "Value2");
//     assert(cache.Get(3) == "Value3");
// }

// void TestGetValuesWithWalker()
// {
//     como::HashMapCache<int, std::string> cache;

//     // 插入元素
//     cache.Put(1, "Value1", 10);
//     cache.Put(2, "Value2", 20);

//     // 遍历所有键值对
//     cache.GetValues([](std::string& str, int& key, std::string& value, struct timespec& time) {
//         // 简单打印键值对
//         std::cout << "Key: " << key << ", Value: " << value << std::endl;
//     });
// }


int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}