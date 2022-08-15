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

#include <malloc.h>
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
CMemPool::CMemPool(size_t ulUnitNum, size_t ulUnitSize)
    : m_pMemBlock(nullptr)
    , m_pAllocatedMemBlock(nullptr)
    , m_pFreeMemBlock(nullptr)
    , m_ulBlockSize(ulUnitNum * (ulUnitSize + sizeof(struct _Unit)))
    , m_ulUnitSize(ulUnitSize)
{

    m_pMemBlock = ::malloc(m_ulBlockSize);            // Allocate a memory block.

    if (nullptr != m_pMemBlock) {
        // Link all mem unit.
        for (size_t i = 0;  i < ulUnitNum;  i++) {
            struct _Unit *pCurUnit = (struct _Unit *)((char *)m_pMemBlock + i *
                                            (ulUnitSize + sizeof(struct _Unit)));

            pCurUnit->pPrev = nullptr;
            pCurUnit->pNext = m_pFreeMemBlock;   // Insert the new unit at head.

            if (nullptr != m_pFreeMemBlock) {
                m_pFreeMemBlock->pPrev = pCurUnit;
            }
            m_pFreeMemBlock = pCurUnit;
        }
    }
}

/**
 * Destructor of this class. Its task is to free memory block.
 */
CMemPool::~CMemPool()
{
    ::free(m_pMemBlock);
}

/**
 * Alloc
 * To allocate a memory unit. If memory pool can`t provide proper memory unit,
 * will call system function.
 *
 * Parameters:
 *     [in] ulSize       	  Memory unit size.
 *     [in] iTryToUseMemPool  Whether use memory pool.
 *
 * Return Values:
 *     Return a pointer to a memory unit.
//=============================================================================
*/
void *CMemPool::Alloc(size_t ulSize, TryToUseMemPool iTryToUseMemPool)
{
    if (ulSize > m_ulUnitSize ||
            			 nullptr == m_pMemBlock || nullptr == m_pFreeMemBlock) {
        if (MUST_USE_MEM_POOL == iTryToUseMemPool)
        	return nullptr;

    	return malloc(ulSize);
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
void CMemPool::Free(void* p)
{
    if (m_pMemBlock < p && p < (void *)((char *)m_pMemBlock + m_ulBlockSize) ) {
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
        free(p);
    }
}

} // namespace como
