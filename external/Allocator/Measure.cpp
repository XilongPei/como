#include <iostream>
#include <chrono>
#include <random>

#include <set>
#include <map>
#include <list>
#include <forward_list>

#include "Allocator.h"
#include "RapidjsonAllocators.h"

const size_t growSize = 1024;
const size_t numberOfIterations = 1024;
const size_t randomRange = 1024;

template <typename Container>
class PerformanceTest
{
    virtual void testIteration(size_t newSize) = 0;

    protected:

        Container container;

        std::default_random_engine randomNumberGenerator;
        std::uniform_int_distribution<size_t> randomNumberDistribution;

    public:

        PerformanceTest()
            : randomNumberGenerator(0)
            , randomNumberDistribution(0, randomRange)
        {}

        virtual ~PerformanceTest()
        {}

        double run()
        {
            auto from = std::chrono::high_resolution_clock::now();

            for (size_t i = 0; i < numberOfIterations; i++)
                testIteration(randomNumberDistribution(randomNumberGenerator));

            auto to = std::chrono::high_resolution_clock::now();
            return std::chrono::duration_cast<std::chrono::duration<double>>(to - from).count();
        }
};

template <typename Container>
class PushFrontTest : public PerformanceTest<Container>
{
    virtual void testIteration(size_t newSize)
    {
        size_t size = 0;

        while (size < newSize)
            this->container.push_front(size++);

        for (; size > newSize; size--)
            this->container.pop_front();
    }
};

template <typename Container>
class PushBackTest : public PerformanceTest<Container>
{
    virtual void testIteration(size_t newSize)
    {
        size_t size = 0;

        while (size < newSize)
            this->container.push_back(size++);

        for (; size > newSize; size--)
            this->container.pop_back();
    }
};

template <typename Container>
class MapTest : public PerformanceTest<Container>
{
    virtual void testIteration(size_t newSize)
    {
        size_t size = 0;

        for (; size < newSize; size++)
            this->container.insert(typename Container::value_type(size, size));

        while (size > newSize)
            this->container.erase(--size);
    }
};

template <typename Container>
class SetTest : public PerformanceTest<Container>
{
    virtual void testIteration(size_t newSize)
    {
        size_t size = 0;

        while (size < newSize)
            this->container.insert(size++);

        while (size > newSize)
            this->container.erase(--size);
    }
};

template <typename StlAllocatorContainer, typename MoyaAllocatorContainer>
void printTestStatus(const char *name, StlAllocatorContainer &stlContainer, MoyaAllocatorContainer &fastContainer)
{
    double stlRunTime = stlContainer.run();
    double moyaRunTime = fastContainer.run();

    std::cout << std::fixed;
    std::cout << name << " - Default STL Allocator : " << stlRunTime << " seconds." << std::endl;
    std::cout << name << " - Memory Pool Allocator : " << moyaRunTime << " seconds." << std::endl;
    std::cout << name << " - Gain : x" << stlRunTime / moyaRunTime << "." << std::endl;
    std::cout << std::endl;
}

int main()
{
    typedef size_t DataType;
    typedef como::MemoryCacheAllocator<DataType, growSize> MemoryCacheAllocator;
    typedef como::MemoryCacheAllocator<std::map<DataType, DataType>::value_type, growSize> MapMemoryCacheAllocator;

    std::cout << "MemoryCacheAllocator performance measurement example" << std::endl;

    PushFrontTest<std::forward_list<DataType>> pushFrontForwardListTestStl;
    PushFrontTest<std::forward_list<DataType, MemoryCacheAllocator>> pushFrontForwardListTestFast;
    printTestStatus("ForwardList PushFront", pushFrontForwardListTestStl, pushFrontForwardListTestFast);

    PushFrontTest<std::list<DataType>> pushFrontListTestStl;
    PushFrontTest<std::list<DataType, MemoryCacheAllocator>> pushFrontListTestFast;
    printTestStatus("List PushFront", pushFrontListTestStl, pushFrontListTestFast);

    PushBackTest<std::list<DataType>> pushBackListTestStl;
    PushBackTest<std::list<DataType, MemoryCacheAllocator>> pushBackListTestFast;
    printTestStatus("List PushBack", pushBackListTestStl, pushBackListTestFast);

    MapTest<std::map<DataType, DataType, std::less<DataType>>> mapTestStl;
    MapTest<std::map<DataType, DataType, std::less<DataType>, MapMemoryCacheAllocator>> mapTestFast;
    printTestStatus("Map", mapTestStl, mapTestFast);

    SetTest<std::set<DataType, std::less<DataType>>> setTestStl;
    SetTest<std::set<DataType, std::less<DataType>, MemoryCacheAllocator>> setTestFast;
    printTestStatus("Set", setTestStl, setTestFast);


    //////////RapidjsonAllocators///////////////////////////////////////////////


    typedef como::StdAllocator<DataType, como::StdAllocator<DataType, como::MemoryPoolAllocator<>>> memPoolAllocator;
    typedef como::StdAllocator<std::map<DataType, DataType>::value_type,
                                         como::StdAllocator<DataType, como::MemoryPoolAllocator<>>> mapMemPoolAllocator;

    std::cout << "StdAllocator performance measurement example" << std::endl;

    PushFrontTest<std::forward_list<DataType>> pushFrontForwardListTestStl_2;
    PushFrontTest<std::forward_list<DataType, memPoolAllocator>> pushFrontForwardListTestFast_2;
    printTestStatus("ForwardList PushFront", pushFrontForwardListTestStl_2, pushFrontForwardListTestFast_2);

    PushFrontTest<std::list<DataType>> pushFrontListTestStl_2;
    PushFrontTest<std::list<DataType, memPoolAllocator>> pushFrontListTestFast_2;
    printTestStatus("List PushFront", pushFrontListTestStl_2, pushFrontListTestFast_2);

    PushBackTest<std::list<DataType>> pushBackListTestStl_2;
    PushBackTest<std::list<DataType, memPoolAllocator>> pushBackListTestFast_2;
    printTestStatus("List PushBack", pushBackListTestStl_2, pushBackListTestFast_2);

    MapTest<std::map<DataType, DataType, std::less<DataType>>> mapTestStl_2;
    MapTest<std::map<DataType, DataType, std::less<DataType>, mapMemPoolAllocator>> mapTestFast_2;
    printTestStatus("Map", mapTestStl_2, mapTestFast_2);

    SetTest<std::set<DataType, std::less<DataType>>> setTestStl_2;
    SetTest<std::set<DataType, std::less<DataType>, memPoolAllocator>> setTestFast_2;
    printTestStatus("Set", setTestStl_2, setTestFast_2);

    return 0;
}
