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

    //Manage all unit with two linkedlist.
    struct _Unit *m_pAllocatedMemBlock;     // Head pointer to Allocated linkedlist.
    struct _Unit *m_pFreeMemBlock;          // Head pointer to Free linkedlist.

    size_t m_ulUnitSize;             // Memory unit size. There are much unit in memory pool.
    size_t m_ulBlockSize;            // Memory pool size. Memory pool is make of memory unit.

public:
    CMemPool(size_t lUnitNum = 50, size_t lUnitSize = 1024);
    ~CMemPool();

    void *Alloc(size_t ulSize, TryToUseMemPool iTryToUseMemPool = MUST_USE_MEM_POOL);
    void  Free(void* p);
};

} // namespace como

#endif //__MEMPOOL_H__
