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

#include "jemalloc_utils.h"

namespace como {

// defined in ComoConfig.h
// sources in directory external/ shouldn't include COMO head file
typedef struct tagFSCP_MEM_AREA_INFO {
    size_t mem_size;        // Size of space to be managed
    void  *base;            // Base address of space to be managed
    size_t allocated;       // Allocated size
} FSCP_MEM_AREA_INFO;
#endif

// 待管理的空间大小
size_t mem_size = 1 << 30;
// 待管理的空间的首地址
void* base;
// 已经分配出去的大小
size_t allocated = 0;

static FSCP_MEM_AREA_INFO *gFscpMemAreasInfo;
static int curFscpMemArea = 0;

int jemallocControl(void *MemAreasInfo, int numAreas, COMO_CALLOC& funComoCalloc)
{
    gFscpMemAreasInfo = (MEM_AREA_INFO *)MemAreasInfo;

    funComoCalloc = je_malloc;

    // 用je_mallctl('arenas.extend')命令创建一个arean，
    // 参考jemalloc/doc/jemalloc.html中arenas.extend一段
    unsigned arena_index;
    size_t unsigned_size = sizeof(unsigned int);
    if (je_mallctl("arenas.extend", (void*)&arena_index, &unsigned_size, nullptr, 0)) {
        printf("je_mallctl('arenas.extend') failed!\n");
        return 1;
    }
    printf("arena_index = %u\n", arena_index);

    //为这个arena绑定我们自定义的chunk hook，于是该arena就会按我们的方式去申请/释放chunk
    // 参考jemalloc/doc/jemalloc.html中arena.<i>.chunk_hooks一段
    char cmd[64];
    sprintf(cmd, "arena.%u.chunk_hooks", arena_index);
    if (je_mallctl(cmd, nullptr, nullptr, (void*)my_chunk_hooks, sizeof(chunk_hooks_t))) {
        printf("je_mallctl('%s') failed!\n", cmd);
        return 1;
    }
}

//+++++++++++++++++++++++++++++HOOK BEGIN+++++++++++++++++++++++++++++

// 从这里开始定义jemalloc管理chunk的hook函数，自定义chunk管理行为
// 可以参考jemalloc/doc/jemalloc.html中arena.<i>.chunk_hooks一段

// 当jemalloc发现chunk不够用了，会callback此函数索要空间
// chunk大小在编译时配置（原版jemalloc-4.0.3默认人2M，libmemkind配置为4M）

#define CHUNK_ALLOC_DEF(fscp_mem_area)                                                      \
void* my_chunk_alloc_##fscp_mem_area(void *chunk, size_t size, size_t alignment,            \
                                        bool *zero, bool *commit, unsigned arena_index)     \
{                                                                                           \
    if (size % alignment)                                                                   \
        return nullptr;                                                                     \
    if (allocated + size > memAreasInfo[##fscp_mem_area]->mem_size)                         \
        return nullptr;                                                                     \
    if (chunk && chunk != memAreasInfo[##fscp_mem_area]->base +                             \
                                            memAreasInfo[##fscp_mem_area]->allocated)       \
        return nullptr;                                                                     \
    void* addr = memAreasInfo[##fscp_mem_area]->base + memAreasInfo[##fscp_mem_area]->allocated;    \
    memAreasInfo[##fscp_mem_area]->allocated += size;                                       \
    return addr;                                                                            \
}

CHUNK_ALLOC_DEF(0)
CHUNK_ALLOC_DEF(1)
CHUNK_ALLOC_DEF(2)
CHUNK_ALLOC_DEF(3)
CHUNK_ALLOC_DEF(4)
CHUNK_ALLOC_DEF(5)

// 返回true表示该内存可以继续使用
bool my_chunk_dalloc(void *chunk, size_t size, bool commited, unsigned arena_index)
{
    return true;
}

// 返回false表示内存充足
bool my_chunk_commit(void *chunk, size_t size, size_t offset, size_t length, unsigned arena_index)
{
    return false;
}

// 返回true表示该内存即使释放了，也是与物理内存对应的，可以重用
bool my_chunk_decommit(void *chunk, size_t size, size_t offset, size_t length, unsigned arena_index)
{
    return true;
}

// 返回true表示该段地址空间被重用后不会清空
bool my_chunk_purge(void *chunk, size_t size, size_t offset, size_t length, unsigned arena_index)
{
    return true;
}

bool my_chunk_split(void *chunk, size_t size, size_t size_a, size_t size_b, bool commited, unsigned arena_index)
{
    return false;
}

bool my_chunk_merge(void *chunk_a, size_t size_a, void *chunk_b, size_t size_b, bool commited, unsigned arena_index)
{
    return false;
}

chunk_hooks_t my_chunk_hooks[] =
{

#define ONE_CHUNK_HOOK(n)               \
    {                                   \
        .alloc = my_chunk_alloc##n,     \
        .dalloc = my_chunk_dalloc,      \
        .commit = my_chunk_commit,      \
        .decommit = my_chunk_decommit,  \
        .purge = my_chunk_purge,        \
        .split = my_chunk_split,        \
        .merge = my_chunk_merge,        \
    }

ONE_CHUNK_HOOK(0),
ONE_CHUNK_HOOK(1),
ONE_CHUNK_HOOK(2),
ONE_CHUNK_HOOK(3),
ONE_CHUNK_HOOK(4),
ONE_CHUNK_HOOK(5)
};

//-----------------------------HOOK END-----------------------------

} // namespace como
