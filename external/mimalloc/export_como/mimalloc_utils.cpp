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

static FSCP_MEM_AREA_INFO *gFscpMemAreasInfo;
static int curFscpMemArea = 0;

int JemallocUtils::setupFscpMemAreas(void *MemAreasInfo, int numAreas)
{
}

void *MimallocUtils::malloc(size_t size)
{
    void* p = mi_heap_malloc(heaps[curFscpMemArea], size);
}

void MimallocUtils::free(void* ptr)
{
     mi_free(ptr);
}

} // namespace como
