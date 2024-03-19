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

#ifndef __MEMPOOL_H__
#define __MEMPOOL_H__

#include "mutex.h"

namespace como {

enum TryToUseMemPool {
    DONT_USE_MEM_POOL,
    MUST_USE_MEM_POOL,
    TRY_TO_USE_MEM_POOL
};

class CMemPool
{
private:
    struct _Unit                            // The type of the node of linkedlist.
    {
        struct _Unit *pPrev, *pNext;
    };

    void         *m_pMemBlock;              // The address of memory pool.
    bool          shouldFreeMemBlock;

    //Manage all unit with two linkedlist.
    struct _Unit *m_pAllocatedMemBlock;     // Head pointer to Allocated linkedlist.
    struct _Unit *m_pFreeMemBlock;          // Head pointer to Free linkedlist.

    size_t m_ulUnitSize;             // Memory unit size. There are much unit in memory pool.
    size_t m_ulBlockSize;            // Memory pool size. Memory pool is make of memory unit.

    Mutex  m_Lock;

public:
    CMemPool();
    CMemPool(void *buffer, size_t ulUnitNum = 0, size_t ulUnitSize = 0);
    ~CMemPool();

    void *Alloc(size_t ulSize, TryToUseMemPool iTryToUseMemPool = MUST_USE_MEM_POOL);
    void Free(void *p);
    bool CheckExist(void *p);
    bool CheckFull();

protected:
    friend class CMemPoolSet;
    bool SetBuffer(void *buffer, size_t ulUnitNum, size_t ulUnitSize);
};

class CMemPoolSet
{
public:
    typedef struct tagMemPoolItem {
        size_t      lUnitNum;
        size_t      lUnitSize;
        CMemPool   *memPool;
    } MemPoolItem;

    CMemPoolSet(MemPoolItem *memPoolItems, size_t num);
    CMemPoolSet(MemPoolItem *memPoolItems, size_t num, size_t g_lUnitNum,
                                                            size_t g_lUnitSize);
    ~CMemPoolSet();

    void *Alloc(size_t ulSize, TryToUseMemPool iTryToUseMemPool = MUST_USE_MEM_POOL);
    bool Free(void *p);

    bool CreateGeneralMemPool(size_t lUnitNum, size_t lUnitSize);

#ifndef __IN_COMOTEST__
private:
#else
public:
#endif
    MemPoolItem    *m_MemPoolSet;
    size_t          m_ItemNum;

    // general purpose memory pool
    CMemPool       *g_MemPool;
    size_t          m_g_lUnitNum;
    size_t          m_g_lUnitSize;

    MemPoolItem    *m_g_MemPoolSet;
};

} // namespace como

#endif //__MEMPOOL_H__
