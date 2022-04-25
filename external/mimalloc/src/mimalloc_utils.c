#include "mimalloc.h"
#include "mimalloc-internal.h"
#include <pthread.h>

FSCP_MEM_AREA_INFO *como_MimallocUtils_gFscpMemAreasInfo = NULL;  //提供外部的安全分区信息数组
int como_MimallocUtils_numFscpMemArea = 0; //当前安全分区内存数量

FSCP_INTERNAL_SEGMENT fscp_segments[FSCP_MEM_AREA_MAX];  //内部使用的安全分区segment结构
static mi_heap_t* heapsFscpMemAreaArray[FSCP_MEM_AREA_MAX]; //heap数组
static bool InitFscpMemAreas = false;

// 前置的安全内存分区校验和处理工作
bool _checkMemAreasInfo(void* MemAreasInfo, int numAreas){
    if(numAreas <= 0 || numAreas > FSCP_MEM_AREA_MAX){
        return false;
    }
    if(MemAreasInfo == NULL){
        return false;
    }

    for(int i=0;i<numAreas;i++){
        FSCP_MEM_AREA_INFO* nowArea = (FSCP_MEM_AREA_INFO*)(MemAreasInfo) + i;
        if(nowArea->allocated!=0){
            return false;
        }
        if((uintptr_t)nowArea->base%MI_SEGMENT_SIZE!= 0){
            return false;
        }
        if(nowArea->mem_size%MI_SEGMENT_SIZE!=0){
            return false;
        }
        if(nowArea->mem_size/MI_SEGMENT_SIZE>FSCP_MEM_BLOCK_MAX){
            return false;
        }
    }

    return true;
}

void _divideMemAreasInfo(void* MemAreasInfo, int numAreas){
    //初始化
    for(int i=0;i<FSCP_MEM_AREA_MAX;i++){
        fscp_segments[i].areaNum = -1;
        fscp_segments[i].blockNum = 0;
    }
    
    //设置内部结构数据
    for(int i=0;i<numAreas;i++){
        FSCP_MEM_AREA_INFO* nowArea = (FSCP_MEM_AREA_INFO*)(MemAreasInfo) + i;
        fscp_segments[i].areaNum=i;
        fscp_segments[i].blockNum=nowArea->mem_size/MI_SEGMENT_SIZE;
        for(int j=0;j<fscp_segments[i].blockNum;j++){
            fscp_segments[i].memBaseState[j]=true;
            fscp_segments[i].memBase[j]=(void*)((uintptr_t)(nowArea->base) + j * MI_SEGMENT_SIZE);
        }
    }
}

//todo : 模块数据完备性校验

//todo : 禁止跨线程的分配和释放

//todo : 分配的大小限制 & Huge Obj兼容

//todo : region::_mi_mem_alloc_aligned做截断

int setupFscpMemAreas(void *MemAreasInfo, int numAreas,
                                     COMO_MALLOC mimalloc, FREE_MEM_FUNCTION mifree)
{
    mi_option_enable(mi_option_fscp); //开启mi_option_fscp :内部实现进行区分的标志
    if(!_checkMemAreasInfo(MemAreasInfo, numAreas)){
        mimalloc =  NULL;
        mifree = NULL;
        return -1;
    }

    como_MimallocUtils_gFscpMemAreasInfo = (FSCP_MEM_AREA_INFO *)MemAreasInfo;
    como_MimallocUtils_numFscpMemArea = numAreas;
    
    _divideMemAreasInfo(MemAreasInfo,numAreas);
    
    for(int i=0;i<numAreas;i++){
        mi_option_set(mi_option_fscp_numArea,i);
        heapsFscpMemAreaArray[i] = mi_heap_fscp_new(i);
    }

    mimalloc = area_malloc;
    mifree = area_free;

    mi_option_enable(mi_option_fscp); //开启mi_option_fscp :内部实现进行区分的标志
    InitFscpMemAreas = true;
    return 0;
}

// 该函数中才实际进行heap和FSCPMemArea进行对应
void *area_malloc(short iMemArea, size_t size)
{
    if (InitFscpMemAreas == false){
        fprintf(stderr,"area_malloc: FSCP haven't init\n");
        return NULL;
    }
    if (iMemArea >= como_MimallocUtils_numFscpMemArea || iMemArea < 0) {
        // 所选分区超过当前实际安全分区数量
        fprintf(stderr,"area_malloc: iMemArea error\n");
        return NULL;
    }
    if (size > MI_SMALL_SIZE_MAX || size <= 0){
        // 目前只能分配小对象:128 * 8
        fprintf(stderr,"area_malloc: size error\n");
        return NULL;
    }

    mi_option_set(mi_option_fscp_numArea,iMemArea);
    mi_heap_t* nowHeapArea = heapsFscpMemAreaArray[iMemArea];
    return mi_heap_malloc(nowHeapArea, size);
}

void area_free(short iMemArea, const void *ptr)
{
    mi_free((void*)ptr);
}
