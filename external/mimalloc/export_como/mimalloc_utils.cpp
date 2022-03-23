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

#include <mimalloc.h>
#include "mimalloc_utils.h"

namespace como {

// defined in ComoConfig.h
// sources in directory external/ shouldn't include COMO head file
typedef struct tagFSCP_MEM_AREA_INFO {
    size_t mem_size;        // Size of space to be managed
    void  *base;            // Base address of space to be managed
    size_t allocated;       // Allocated size
} FSCP_MEM_AREA_INFO;

extern "C" FSCP_MEM_AREA_INFO *como_MimallocUtils_gFscpMemAreasInfo;
extern "C" int como_MimallocUtils_numFscpMemArea = 0;
static mi_heap_t* heapsFscpMemArea = nullptr;

int MimallocUtils::setupFscpMemAreas(void *MemAreasInfo, int numAreas,
                                     COMO_MALLOC& mimalloc, FREE_MEM_FUNCTION& mifree)
{
    /*
        // FSCP: Function Safety Computing Platform
        typedef struct tagFSCP_MEM_AREA_INFO {
            size_t mem_size;        // Size of space to be managed
            void  *base;            // Base address of space to be managed
            size_t allocated;       // Allocated size
        } FSCP_MEM_AREA_INFO;
    */
    como_MimallocUtils_gFscpMemAreasInfo = (FSCP_MEM_AREA_INFO *)MemAreasInfo;
    como_MimallocUtils_numFscpMemArea = numAreas;
    if (numAreas > 0) {
        heapsFscpMemArea = calloc(sizeof(mi_heap_t*), numAreas);
        if (nullptr == heapsFscpMemArea)
            return 1;
        for (int i = 0;  i < numAreas;  i++) {
            heapsFscpMemArea[i] = mi_heap_new();
            heapsFscpMemArea[i]->iFscpMemArea = i;
        }
    }

    mimalloc = mi_malloc;
    mifree = mi_free;

    return 0;
}


void *MimallocUtils::area_malloc(short iMemArea, size_t size)
{
    void* p = mi_heap_malloc(heapsFscpMemArea[iMemArea], size);
}

void MimallocUtils::area_free(short iMemArea, const void *ptr)
{
    (void)(iMemArea);
    mi_free(ptr);
}

} // namespace como
