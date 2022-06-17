/**
 * Nevertheless, there are many scenarios where customized allocators are
 * desirable. Some of the most common reasons for writing custom allocators
 * include improving performance of allocations by using memory pools, and
 * encapsulating access to different types of memory, like shared memory or
 * garbage-collected memory. In particular, programs with many frequent
 * allocations of small amounts of memory may benefit greatly from specialized
 * allocators, both in terms of running time and memory footprint.
 */

#ifndef __Allocator_H__
#define __Allocator_H__

#include <memory>
#include <errno.h>
//#include "../mimalloc/export_como/mimalloc_utils.h"

namespace como {

template <typename T, std::size_t growSize = 1024>
class MemoryPool
{
    struct Block
    {
        Block *next;
    };

    class Buffer
    {
        static const std::size_t blockSize = sizeof(T) > sizeof(Block) ? sizeof(T) : sizeof(Block);

        uint8_t data[blockSize * growSize];

        public:

            Buffer *const next;

            Buffer(Buffer *next)
                : next(next)
            {}

            T *getBlock(std::size_t index)
            {
                return reinterpret_cast<T *>(&data[blockSize * index]);
            }
    };

    Block  *firstFreeBlock = nullptr;
    Buffer *firstBuffer    = nullptr;
    std::size_t bufferedBlocks = growSize;

public:
    MemoryPool() = default;
    MemoryPool(MemoryPool &&memoryPool) = delete;
    MemoryPool(const MemoryPool &memoryPool) = delete;
    MemoryPool operator =(MemoryPool &&memoryPool) = delete;
    MemoryPool operator =(const MemoryPool &memoryPool) = delete;

    ~MemoryPool()
    {
        while (firstBuffer) {
            Buffer *buffer = firstBuffer;
            firstBuffer = buffer->next;
            delete buffer;
        }
    }

    T *allocate()
    {
        if (firstFreeBlock) {
            Block *block = firstFreeBlock;
            firstFreeBlock = block->next;
            return reinterpret_cast<T *>(block);
        }

        if (bufferedBlocks >= growSize) {
            firstBuffer = new Buffer(firstBuffer);
            if (nullptr == firstBuffer) {
                errno = ENOMEM;
                return nullptr;
            }

            bufferedBlocks = 0;
        }

        return firstBuffer->getBlock(bufferedBlocks++);
    }

    void deallocate(T *pointer)
    {
        Block *block = reinterpret_cast<Block *>(pointer);
        block->next = firstFreeBlock;
        firstFreeBlock = block;
    }
};


/**
 * MemoryCacheAllocator
 ******************************************************************************/

template <typename T, std::size_t growSize = 1024>
class MemoryCacheAllocator : private MemoryPool<T, growSize>
{
#if defined(_WIN32) && defined(ENABLE_OLD_WIN32_SUPPORT)
    Allocator         *copyAllocator = nullptr;
    std::allocator<T> *rebindAllocator = nullptr;
#endif

public:
    typedef std::size_t     size_type;
    typedef std::ptrdiff_t  difference_type;
    typedef T        *pointer;
    typedef const T  *const_pointer;
    typedef T        &reference;
    typedef const T  &const_reference;
    typedef T         value_type;

    template <typename U>
    struct rebind
    {
        typedef MemoryCacheAllocator<U, growSize> other;
    };

#if defined(_WIN32) && defined(ENABLE_OLD_WIN32_SUPPORT)
    MemoryCacheAllocator() = default;

    MemoryCacheAllocator(Allocator &allocator)
        : copyAllocator(&allocator)
    {}

    template <typename U>
    MemoryCacheAllocator(const Allocator<U, growSize> &other)
    {
        if (! std::is_same<T, U>::value) {
            rebindAllocator = new std::allocator<T>();
            if (nullptr == rebindAllocator)
                errno = ENOMEM;
        }
    }

    ~MemoryCacheAllocator()
    {
        delete rebindAllocator;
    }
#endif

    //@ `ALLOCATE#HINT`
    /**
     * C++11 states, in 20.6.9.1 allocator members:
     * 4 - [ Note: In a container member function, the address of an adjacent
     *       element is often a good choice to pass for the hint argument. — end note ]
     * [...]
     * 6 - [...] The use of hint is unspecified, but intended as an aid to
     *           locality if an implementation so desires.
     *
     * Allocating new elements adjacent or close to existing elements in memory
     * can aid performance by improving locality; because they are usually cached
     * together, nearby elements will tend to travel together up the memory
     * hierarchy and will not evict each other.
     */
    pointer allocate(size_type n, const void *hint = 0)
    {
#if defined(_WIN32) && defined(ENABLE_OLD_WIN32_SUPPORT)
        if (nullptr != copyAllocator)
            return copyAllocator->allocate(n, hint);

        if (nullptr != rebindAllocator)
            return rebindAllocator->allocate(n, hint);
#endif

        /**
         * The MemoryPool mechanism can allocate only one object at a time.
         */
        if (n != 1 || hint) {
            // COMO turns off the c++ exception mechanism
            // throw std::bad_alloc();
            return nullptr;
        }

        return MemoryPool<T, growSize>::allocate();
    }

    void deallocate(pointer p, size_type n)
    {
#if defined(_WIN32) && defined(ENABLE_OLD_WIN32_SUPPORT)
        if (nullptr != copyAllocator) {
            copyAllocator->deallocate(p, n);
            return;
        }

        if (nullptr != rebindAllocator) {
            rebindAllocator->deallocate(p, n);
            return;
        }
#endif

        MemoryPool<T, growSize>::deallocate(p);
    }

    void construct(pointer p, const_reference val)
    {
        if (nullptr != p)
            new (p) T(val);
        else
            errno = EADDRNOTAVAIL;
    }

    void destroy(pointer p)
    {
        p->~T();
    }
};


/**
 * MemoryAearAllocator
 ******************************************************************************/

#ifdef __MIMALLOC_UTILS_H__

template <typename T, short iMemArea = 0>
class MemoryAearAllocator
{
public:
    typedef std::size_t     size_type;
    typedef std::ptrdiff_t  difference_type;
    typedef T        *pointer;
    typedef const T  *const_pointer;
    typedef T        &reference;
    typedef const T  &const_reference;
    typedef T         value_type;

    template <typename U>
    struct rebind
    {
        typedef MemoryAearAllocator<U, iMemArea> other;
    };

    // refer to `ALLOCATE#HINT`
    pointer allocate(size_type n, const void *hint = 0)
    {
        return MimallocUtils::area_malloc(iMemArea, sizeof(T) * n);
    }

    void deallocate(pointer p, size_type n)
    {
        MimallocUtils::area_free(iMemArea, p);
    }

    //@ `construct`
    /**
     * Defined in header <memory>
     * Constructs an object of type T in allocated uninitialized storage pointed
     * to by p, using placement-new
     *
     * void construct(pointer p, const_reference val);  (1) (until C++11)
     *
     * template<class U, class... Args>
     * void construct(U* p, Args&&... args);            (2) (since C++11)
     *                                                      (deprecated in C++17)
     *                                                      (removed in C++20)
     * construct_at (C++20)         creates an object at a given address
     *                              (function template)
     *
     * p       -  pointer to allocated uninitialized storage
     * val     -  the value to use as the copy constructor argument
     * args... -  the constructor arguments to use
     */
    void construct(pointer p, const_reference val)
    {
        if (nullptr != p)
            new (p) T(val);
        else
            errno = EADDRNOTAVAIL;
    }

    void destroy(pointer p)
    {
        p->~T();
    }
};

#endif

} // namespace como

#endif // __Allocator_H__
