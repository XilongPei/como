#include <pthread.h>
#include "mimalloc.h"
#include "mimalloc-internal.h"

/*
todo list:
0. 去掉debug输出  done 2022.5.3
1. Huge Obj兼容
2. 排除mimalloc一些技术特性的影响
3. 多线程申请和释放确保安全
4. stat打印管理信息兼容 done 2022.5.3 unsafe
5. printf干扰mimalloc的heap管理，fprintf+sderr则无，没有主流程影响，查明原因
6. 接入como测试
7. 中文注释改英文
*/

FSCP_MEM_AREA_INFO* como_MimallocUtils_gFscpMemAreasInfo = NULL;  //提供外部的安全分区信息数组
int como_MimallocUtils_numFscpMemArea = 0;                        //当前安全分区内存数量

FSCP_INTERNAL_SEGMENT fscp_segments[FSCP_MEM_AREA_MAX];      //内部使用的安全分区segment结构
static mi_heap_t* heapsFscpMemAreaArray[FSCP_MEM_AREA_MAX];  //heap数组
static bool InitFscpMemAreas = false;
pthread_mutex_t miOptionAreaLock;  // 多线程并发申请内存锁

// @brief: 内部函数，前置的安全内存分区校验和处理工作
// @param: 外部安全分区信息数组MemAreasInfo, 安全分区数量numAreas
// @ret: 校验无误, 则true, 反之false
bool _checkMemAreasInfo(void* MemAreasInfo, int numAreas) {
    if (numAreas <= 0 || numAreas > FSCP_MEM_AREA_MAX) {
        _mi_error_message(ERANGE, "numArea out of range\n");
        return false;
    }
    if (MemAreasInfo == NULL) {
        _mi_error_message(EFAULT, "Illegal MemAreasInfo\n");
        return false;
    }

    for (int i = 0; i < numAreas; i++) {
        FSCP_MEM_AREA_INFO* nowArea = (FSCP_MEM_AREA_INFO*)(MemAreasInfo) + i;
        if (nowArea->allocated != 0) {
            _mi_error_message(EFAULT, "Illegal MemAreasInfo, allocated != 0\n");
            return false;
        }
        if ((uintptr_t)nowArea->base % MI_SEGMENT_SIZE != 0) {
            _mi_error_message(EFAULT, "Illegal MemAreasInfo, base address error\n");
            return false;
        }
        if (nowArea->mem_size % MI_SEGMENT_SIZE != 0) {
            _mi_error_message(EFAULT, "Illegal MemAreasInfo, memory size error\n");
            return false;
        }
        if (nowArea->mem_size / MI_SEGMENT_SIZE > FSCP_MEM_BLOCK_MAX) {
            _mi_error_message(EFAULT, "Illegal MemAreasInfo, memory size error\n");
            return false;
        }
    }

    return true;
}

// @brief: 内部函数，将外部内存分区信息转换为内部内存分区数据结构
// @param: 外部安全分区信息数组MemAreasInfo, 安全分区数量numAreas
// @ret: void
void _divideMemAreasInfo(void* MemAreasInfo, int numAreas) {
    //初始化
    for (int i = 0; i < FSCP_MEM_AREA_MAX; i++) {
        fscp_segments[i].areaNum = -1;
        fscp_segments[i].blockNum = 0;
    }

    //设置内部结构数据
    for (int i = 0; i < numAreas; i++) {
        FSCP_MEM_AREA_INFO* nowArea = (FSCP_MEM_AREA_INFO*)(MemAreasInfo) + i;
        fscp_segments[i].areaNum = i;
        fscp_segments[i].blockNum = nowArea->mem_size / MI_SEGMENT_SIZE;
        for (int j = 0; j < fscp_segments[i].blockNum; j++) {
            fscp_segments[i].memBaseState[j] = false;  //尚未被分配出去
            fscp_segments[i].memBase[j] = (void*)((uintptr_t)(nowArea->base) + j * MI_SEGMENT_SIZE);
        }
    }
}

// @brief: 初始化功能安全内存分区
// @param: 外部安全分区信息数组MemAreasInfo, 安全分区数量numAreas, 分配内存函数指针mimalloc, 释放内存函数指针mifree
// @ret: 初始化成功, 则返回0, 否则-1
// @note: 请前置初始化，请在主线程进行初始化，不要进行多线程初始化
int mi_setupFscpMemAreas(void* MemAreasInfo, int numAreas, COMO_MALLOC mimalloc, FREE_MEM_FUNCTION mifree) {
    pthread_mutex_init(&miOptionAreaLock, NULL);
    mi_option_enable(mi_option_fscp);  //开启mi_option_fscp :内部实现进行区分的标志
    if (!_checkMemAreasInfo(MemAreasInfo, numAreas)) {
        mimalloc = NULL;
        mifree = NULL;
        return -1;
    }

    como_MimallocUtils_gFscpMemAreasInfo = (FSCP_MEM_AREA_INFO*)MemAreasInfo;
    como_MimallocUtils_numFscpMemArea = numAreas;

    _divideMemAreasInfo(MemAreasInfo, numAreas);

    for (int i = 0; i < numAreas; i++) {
        mi_option_set(mi_option_fscp_numArea, i);
        // 申请到的heap结构都是系统默认分配的内存
        mi_heap_t* newFscpHeap = mi_heap_fscp_new(i);
        if (NULL == newFscpHeap) {
            return -1;
        }
        heapsFscpMemAreaArray[i] = newFscpHeap;
        fscp_segments[i].heapBelongTo = newFscpHeap;
    }

    mimalloc = mi_area_malloc;
    mifree = mi_area_free;
    if (NULL == mimalloc || NULL == mifree) {
        return -1;
    }

    InitFscpMemAreas = true;
    return 0;
}

// @brief: 在特定内存分区进行内存申请
// @param: 分区序号iMemArea(从0开始), 申请内存大小size(目前不支持大对象内存申请 2MiB)
// @ret: 申请成功, 则返回地址, 否则返回NULL
// @note: 支持多线程下进行申请操作(目前还不支持，可能会被mimalloc拦截，需要去除一些断言)
void* mi_area_malloc(short iMemArea, size_t size) {
    if (InitFscpMemAreas == false) {
        _mi_error_message(ERANGE, "FSCP not initialized\n");
        return NULL;
    }
    if (iMemArea >= como_MimallocUtils_numFscpMemArea || iMemArea < 0) {
        _mi_error_message(ERANGE, "iMemArea out of range\n");
        return NULL;
    }
    if (size > MI_LARGE_OBJ_SIZE_MAX || 0 == size) {
        _mi_error_message(ERANGE, "size out of range\n");
        return NULL;
    }

    pthread_mutex_lock(&miOptionAreaLock);

    mi_option_set(mi_option_fscp_numArea, iMemArea);
    mi_heap_t* nowHeapArea = heapsFscpMemAreaArray[iMemArea];
    void* ret = mi_heap_malloc(nowHeapArea, size);

    pthread_mutex_unlock(&miOptionAreaLock);
    return ret;
}

void mi_area_free(short iMemArea, const void* ptr) {
    if (iMemArea >= como_MimallocUtils_numFscpMemArea || iMemArea < 0) {
        _mi_error_message(ERANGE, "iMemArea out of range\n");
        return;
    }
    mi_free((void*)ptr);
}

void mi_area_info(short iMemArea) {
    //unsafe now
    if (iMemArea >= como_MimallocUtils_numFscpMemArea || iMemArea < 0) {
        _mi_error_message(ERANGE, "iMemArea out of range\n");
        return;
    }
    mi_areaHeap_stats_print(NULL, heapsFscpMemAreaArray[iMemArea]);
}
