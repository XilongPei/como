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

/*
class CMemPool
+--------------+    m_pMemBlock___      /----------------\
| struct _Unit +--> _Unit 0 <==>  _Unit 1   _Unit 2 ...  _Unit N
+--------------+          m_pFreeMemBlock---^

class CMemPoolSet
+------------------------+
| struct _Unit UnitSize0 | --> CMemPool0
| struct _Unit UnitSize1 | --> CMemPool1
| ...                    | --> ...
| ...                    | --> ...
| struct _Unit UnitSizeN | --> CMemPoolN
+------------------------+
| struct _Unit UnitSize0 | -->Gereral Purpose fix sized <= from g_MemPool
| struct _Unit UnitSize1 | -->Gereral Purpose fix sized <= from g_MemPool
| ...................... |
|                        |
+------------------------+
*/

#include <malloc.h>
#include <stdint.h>
#include "MemPool.h"

namespace como {

/**
 * Constructor of this class. It allocate memory block from system and create
 * a static double linked list to manage all memory unit.
 *
 * Parameters:
 *     [in] ulUnitNum     The number of unit which is a part of memory block.
 *     [in] ulUnitSize The size of unit.
 */
CMemPool::CMemPool()
    : m_pMemBlock(nullptr)
    , m_pAllocatedMemBlock(nullptr)
    , m_pFreeMemBlock(nullptr)
    , m_ulBlockSize(0)
    , m_ulUnitSize(0)
{}

CMemPool::CMemPool(void *buffer, size_t ulUnitNum, size_t ulUnitSize)
    : m_pAllocatedMemBlock(nullptr)
    , m_pFreeMemBlock(nullptr)
{
    (void)SetBuffer(buffer, ulUnitNum, ulUnitSize);
}

/**
 * Destructor of this class. Its task is to free memory block.
 */
CMemPool::~CMemPool()
{
    if (shouldFreeMemBlock && (nullptr != m_pMemBlock))  {
        ::free(m_pMemBlock);
    }
}

/**
 * SetBuffer
 */
bool CMemPool::SetBuffer(void *buffer, size_t ulUnitNum, size_t ulUnitSize)
{
    m_ulBlockSize = ulUnitNum * (ulUnitSize + sizeof(struct _Unit));
    m_ulUnitSize = ulUnitSize;

    if (nullptr == buffer) {
        m_pMemBlock = ::malloc(m_ulBlockSize);       // Allocate a memory block.
        if (nullptr == m_pMemBlock) {
            return false;
        }

        shouldFreeMemBlock = true;
    }
    else {
        m_pMemBlock = buffer;
        shouldFreeMemBlock = false;
    }

    if (nullptr != m_pMemBlock) {
        // Link all mem unit.

        size_t ul = m_ulUnitSize + sizeof(struct _Unit);
        ulUnitNum = m_ulBlockSize / ul;
        for (size_t i = 0u;  i < ulUnitNum;  i++) {
            struct _Unit *pCurUnit = (struct _Unit *)((char *)m_pMemBlock + (i * ul));

            pCurUnit->pPrev = nullptr;
            pCurUnit->pNext = m_pFreeMemBlock;   // Insert the new unit at head.

            if (nullptr != m_pFreeMemBlock) {
                m_pFreeMemBlock->pPrev = pCurUnit;
            }
            m_pFreeMemBlock = pCurUnit;
        }
    }

    return true;
}

/**
 * Alloc
 * To allocate a memory unit. If memory pool can`t provide proper memory unit,
 * will call system function.
 *
 * Parameters:
 *     [in] ulSize            Memory unit size.
 *     [in] iTryToUseMemPool  Whether use memory pool.
 *
 * Return Values:
 *     Return a pointer to a memory unit.
 */
void *CMemPool::Alloc(size_t ulSize, TryToUseMemPool iTryToUseMemPool)
{
    Mutex::AutoLock lock(m_Lock);

    if (ulSize > m_ulUnitSize ||
                         nullptr == m_pMemBlock || nullptr == m_pFreeMemBlock) {
        if (MUST_USE_MEM_POOL == iTryToUseMemPool) {
            return nullptr;
        }

        return ::malloc(ulSize);
    }

    // Now FreeList isn`t empty
    struct _Unit *pCurUnit = m_pFreeMemBlock;

    m_pFreeMemBlock = pCurUnit->pNext;       // Get a unit from free linkedlist.
    if (nullptr != m_pFreeMemBlock) {
        m_pFreeMemBlock->pPrev = nullptr;
    }

    pCurUnit->pNext = m_pAllocatedMemBlock;

    if (nullptr != m_pAllocatedMemBlock) {
        m_pAllocatedMemBlock->pPrev = pCurUnit;
    }
    m_pAllocatedMemBlock = pCurUnit;

    return (void *)((char *)pCurUnit + sizeof(struct _Unit));
}

/**
 * To free a memory unit. If the pointer of parameter point to a memory unit,
 * then insert it to "Free linked list". Otherwise, call system function "free".
 *
 * Parameters:
 *     [in] p    point to a memory unit and prepare to free it.
 *
 * Return Values:
 *     none
 */
void CMemPool::Free(void *p)
{
    Mutex::AutoLock lock(m_Lock);

    if ((m_pMemBlock < p) && (p < (void *)((char *)m_pMemBlock + m_ulBlockSize))) {
        struct _Unit *pCurUnit = (struct _Unit *)((char *)p - sizeof(struct _Unit));

        m_pAllocatedMemBlock = pCurUnit->pNext;
        if (nullptr != m_pAllocatedMemBlock) {
            m_pAllocatedMemBlock->pPrev = nullptr;
        }

        pCurUnit->pNext = m_pFreeMemBlock;
        if (nullptr != m_pFreeMemBlock) {
            m_pFreeMemBlock->pPrev = pCurUnit;
        }

        m_pFreeMemBlock = pCurUnit;
    }
    else {
        ::free(p);
    }
}

/**
 * Parameters:
 *     [in] p    point to a memory unit.
 *
 * Return Values:
 *     whether p in this CMemPool
 */
bool CMemPool::CheckExist(void *p)
{
    if ((m_pMemBlock < p) && (p < (void *)((char *)m_pMemBlock + m_ulBlockSize))) {
        return true;
    }

    return false;
}

/**
 * CheckFull
 * Determine if the pool is full.
 *
 * Return Values:
 *     Return true if the pool is full.
 */
bool CMemPool::CheckFull()
{
    return (nullptr == m_pMemBlock) || (nullptr == m_pFreeMemBlock);
}

/**
 * (m_MemPoolSet == nullptr) indicates the construction fail.
 * memPoolItems must be ordered by keyword lUnitSize.
 */
CMemPoolSet::CMemPoolSet(MemPoolItem *memPoolItems, size_t num)
    : g_MemPool(nullptr)
    , m_g_lUnitNum(0)
    , m_g_lUnitSize(0)
{
    m_MemPoolSet = (MemPoolItem *)calloc(num, sizeof(MemPoolItem));
    if (nullptr != m_MemPoolSet) {
        for (size_t i = 0u;  i < num;  i++) {
            m_MemPoolSet[i].memPool = new CMemPool(nullptr,
                                                   memPoolItems[i].lUnitNum,
                                                   memPoolItems[i].lUnitSize);
            if (nullptr == m_MemPoolSet[i].memPool) {
                for (size_t j = 0u;  j < i;  j++) {
                    delete m_MemPoolSet[i].memPool;
                }
                ::free(m_MemPoolSet);

                // Set m_MemPoolSet to nullptr indicates that the object was not
                // constructed successfully.
                m_MemPoolSet = nullptr;
                return;
            }

            m_MemPoolSet[i].lUnitNum = memPoolItems[i].lUnitNum;
            m_MemPoolSet[i].lUnitSize = memPoolItems[i].lUnitSize;
        }
    }
    m_ItemNum = num;
}

CMemPoolSet::CMemPoolSet(MemPoolItem *memPoolItems, size_t num,
                                          size_t g_lUnitNum, size_t g_lUnitSize)
{
    CMemPoolSet(memPoolItems, num);
    (void)CreateGeneralMemPool(g_lUnitNum, g_lUnitSize);
}

bool CMemPoolSet::CreateGeneralMemPool(size_t lUnitNum, size_t lUnitSize)
{
    if ((0u == lUnitNum ) || (0u == lUnitSize)) {
        return false;
    }

    g_MemPool = new CMemPool(nullptr, lUnitNum, lUnitSize);
    if (nullptr == g_MemPool) {
        return false;
    }

    m_g_lUnitNum = lUnitNum;
    m_g_lUnitSize = lUnitSize;

    m_g_MemPoolSet = (MemPoolItem *)calloc(lUnitNum, sizeof(MemPoolItem));
    if (nullptr == m_g_MemPoolSet) {
        ::free(g_MemPool);
        return false;
    }

    for (size_t i = 0u;  i < lUnitNum;  i++) {
        m_g_MemPoolSet[i].memPool = new CMemPool();
        if (nullptr == m_g_MemPoolSet[i].memPool) {
            ::free(g_MemPool);
            return false;
        }
    }

    return true;
}

/**
 * ~CMemPoolSet()
 */
CMemPoolSet::~CMemPoolSet()
{
    for (size_t i = 0u;  i < m_ItemNum;  i++) {
        delete m_MemPoolSet[i].memPool;
    }
    ::free(m_MemPoolSet);
}

/**
 * Alloc
 * To allocate a memory unit.
 *
 * Parameters:
 *     [in] ulSize            Memory unit size.
 *     [in] iTryToUseMemPool  Whether use memory pool.
 *
 * Return Values:
 *     Return a pointer to a memory unit.
 */
void *CMemPoolSet::Alloc(size_t ulSize, TryToUseMemPool iTryToUseMemPool)
{
    void *p;
    size_t i;
    size_t ulElemSize;

    for (i = 0u;  i < m_ItemNum;  i++) {
        if (ulSize <= m_MemPoolSet[i].lUnitSize) {
            p = m_MemPoolSet[i].memPool->Alloc(ulSize, iTryToUseMemPool);
            if (nullptr == p) {
                break;
            }
            else {
                return p;
            }
        }
    }

    if (i < m_ItemNum) {
        ulElemSize = m_MemPoolSet[i].lUnitSize;
    }
    else {
        ulElemSize = ulSize;
    }

    // acquire memory from g_MemPool
    size_t lastUnitSize = UINTMAX_MAX;
    size_t pos = m_g_lUnitNum;
    if (nullptr != g_MemPool) {

/*This algorithm needs to be optimized！
Gereral Purpose, fix sized, from g_MemPool

             +------------------------+
           0 |                        |
             | === === === === ===    |-->SetBuffer, m_ulBlockSize, m_ulUnitSize
             | ......                 |
m_g_lUnitNum |                        |
             +------------------------+
*/
        for (i = 0u;  i < m_g_lUnitNum;  i++) {
            if ((ulSize <= m_g_MemPoolSet[i].lUnitSize) &&
                                (m_g_MemPoolSet[i].lUnitSize < lastUnitSize) &&
                                (! m_g_MemPoolSet[i].memPool->CheckFull())) {
                lastUnitSize = m_g_MemPoolSet[i].lUnitSize;
                pos = i;
            }
        }

        if (pos < m_g_lUnitNum) {
            p = m_g_MemPoolSet[pos].memPool->Alloc(ulSize, iTryToUseMemPool);
            if (nullptr != p) {
                return p;
            }
        }

        for (i = 0u;  i < m_g_lUnitNum;  i++) {
            CMemPool *tmpPool = m_g_MemPoolSet[i].memPool;
            if (0u == tmpPool->m_ulBlockSize) {
                void *mem = g_MemPool->Alloc(m_g_lUnitSize);
                if (nullptr == mem) {
                    return nullptr;
                }

                tmpPool->SetBuffer(mem, ulElemSize, m_g_lUnitSize/ulElemSize);

                return tmpPool->Alloc(ulSize, iTryToUseMemPool);
            }
        }
    }

    return nullptr;
}

/**
 * To free a memory unit. If the pointer of parameter point to a memory unit,
 * then insert it to "Free linked list". Otherwise, do nothing.
 *
 * Parameters:
 *     [in] p    point to a memory unit and prepare to free it.
 *
 * Return Values:
 *     whether memory P is freed.
 */
bool CMemPoolSet::Free(void *p)
{
    for (size_t i = 0u;  i < m_ItemNum;  i++) {
        if (m_MemPoolSet[i].memPool->CheckExist(p)) {
            m_MemPoolSet[i].memPool->Free(p);
            return true;
        }
    }

    return false;
}

} // namespace como
