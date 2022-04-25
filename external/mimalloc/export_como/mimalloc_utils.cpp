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
目前改造：
仅支持安全分区内存大小为4M大小；Mimalloc PAGE Kind = SMALL Pages(64KiB)；
单个heap仅对应单个安全分区segment
*/

#include "mimalloc.h"
#include "mimalloc-internal.h"
#include "mimalloc_utils.h"
#include <stdlib.h> //function calloc
#include <stdio.h>

// defined in ComoConfig.h
// sources in directory external/ shouldn't include COMO head file
typedef struct tagFSCP_MEM_AREA_INFO {
    size_t mem_size;        // Size of space to be managed
    void  *base;            // Base address of space to be managed
    size_t allocated;       // Allocated size
} FSCP_MEM_AREA_INFO;

extern FSCP_MEM_AREA_INFO *como_MimallocUtils_gFscpMemAreasInfo;
extern int como_MimallocUtils_numFscpMemArea;

//#define MI_SEGMENT_SIZE 4194304 //4M

namespace como
{
    


static mi_heap_t** heapsFscpMemArea = nullptr;

int setupFscpMemAreas(void *MemAreasInfo, int numAreas,
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

    //todo : 前置的功能安全内存信息检查

    como_MimallocUtils_gFscpMemAreasInfo = (FSCP_MEM_AREA_INFO *)MemAreasInfo;
    como_MimallocUtils_numFscpMemArea = numAreas;
    printf("setupFscpMemAreas:MemAreasInfo(%p),numAreas(%d)\n",como_MimallocUtils_gFscpMemAreasInfo,como_MimallocUtils_numFscpMemArea);
    if (numAreas > 0) {
        heapsFscpMemArea = (mi_heap_t**)mi_calloc(sizeof(mi_heap_t*), numAreas); //在默认堆上分配堆管理结构
        if (nullptr == heapsFscpMemArea) {
            fprintf(stderr,"setupFscpMemAreas: calloc failed\n");
            return 1;
        }else{
            printf("setupFscpMemAreas: heapsFscpMemArea(%p)\n",heapsFscpMemArea);
        }


        for (int i = 0;  i < numAreas;  i++) {
            mi_heap_t* area = mi_heap_new();;

            // 该heap结构数据存储在mimalloc默认线程heap管理的内存中
            heapsFscpMemArea[i]=area;

            if (nullptr == area) {
                fprintf(stderr,"setupFscpMemAreas: mi_heap_new failed\n");
            }else{
                printf("setupFscpMemAreas: mi_heap_new %p\n",area);
            }


            area -> iFscpMemArea = i; //todo :这种方式不太好
        }

        mimalloc = area_malloc;
        mifree = area_free;
    } else {
        mimalloc =  nullptr;
        mifree = nullptr;
        fprintf(stderr,"setupFscpMemAreas: numAreas <= 0\n");
        return 1;
    }

    return 0;
}

// 该函数中才实际进行heap和FSCPMemArea进行对应
void *area_malloc(short iMemArea, size_t size)
{
    if (como_MimallocUtils_gFscpMemAreasInfo == nullptr) {
        // 尚未进行安全分区初始化 setupFscpMemAreas
        fprintf(stderr,"area_malloc: FscpMemAreas don't exist\n");
        return nullptr;
    }
    if (iMemArea >= como_MimallocUtils_numFscpMemArea || iMemArea < 0) {
        // 所选分区超过当前实际安全分区数量
        fprintf(stderr,"area_malloc: iMemArea error\n");
        return nullptr;
    }
    if (size > MI_SMALL_SIZE_MAX || size <= 0){
        // 目前只能分配小对象:128 * 8
        fprintf(stderr,"area_malloc: size error\n");
        return nullptr;
    }

    FSCP_MEM_AREA_INFO* nowFSCPMemArea = &como_MimallocUtils_gFscpMemAreasInfo[iMemArea];
    if (nowFSCPMemArea == nullptr || nowFSCPMemArea->base == nullptr) {
        // 所选安全分区实际不存在
        fprintf(stderr,"area_malloc: FSCPMemArea No.%d dont't exist\n",iMemArea);
        return nullptr;
    }

    if((unsigned long long)(nowFSCPMemArea->base) % MI_SEGMENT_SIZE != 0){
        // 安全分区若要作为mimalloc的segment使用,则需要满足地址对齐
        fprintf(stderr,"area_malloc: FSCPMemArea No.%d 's baseAddr error\n",iMemArea);
        return nullptr;
    }

    if((nowFSCPMemArea->mem_size) != MI_SEGMENT_SIZE){
        // 安全分区若要作为mimalloc的segment使用,大小要满足4M
        fprintf(stderr,"area_malloc: FSCPMemArea No.%d 's size error\n",iMemArea);
        return nullptr;
    }
    
    //todo memset


    mi_heap_t* nowHeapArea = heapsFscpMemArea[iMemArea];
    void* p = mi_heap_malloc(nowHeapArea, size);
    return p;
}

void area_free(short iMemArea, const void *ptr)
{
    // qjy : segment的地址为4M对齐，根据ptr可以直接找到其对应的segment并进行释放操作
    int i = iMemArea;
    i++;
    mi_free((void*)ptr);
}

} // namespace name
