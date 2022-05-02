#include "mimalloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>
#include <pthread.h>

#define MI_SEGMENT_SIZE 4194304

// Align upwards
uintptr_t align_up(uintptr_t sz, size_t alignment) {
  uintptr_t mask = alignment - 1;
  if ((alignment & mask) == 0) {  // power of two?
    return ((sz + mask) & ~mask);
  }
  else {
    return (((sz + mask)/alignment)*alignment);
  }
}

void* act_partition1(void* args) {
  void* p= mi_malloc(16);
  printf("thread p pointer %p\n",p);
  return NULL;
}


int main(){
    remove("OsManagerTest.dat");
    remove("OsManagerTest2.dat");
    int fd = open("OsManagerTest.dat", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);  
    int fd2 = open("OsManagerTest2.dat",O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd < 0 || fd2 < 0) {
        perror("open fail : ");
        return 0;
    }

    int res = ftruncate(fd, 2*MI_SEGMENT_SIZE);
    int res2 = ftruncate(fd2, 2*MI_SEGMENT_SIZE);
    if(res < 0 || res2 < 0){
        perror("ftruncate fail : ");
        return 0;
    }

    void *start = mmap(NULL,2*MI_SEGMENT_SIZE,PROT_READ | PROT_WRITE, MAP_SHARED,fd,0);
    void *start2 = mmap(NULL,2*MI_SEGMENT_SIZE,PROT_READ | PROT_WRITE, MAP_SHARED,fd2,0);
    if(start == MAP_FAILED || start2 == MAP_FAILED){
        perror("mmap fail : ");
        return 0;
    } 
    else {
        start= (void*)align_up((unsigned long)start,MI_SEGMENT_SIZE);
        start2= (void*)align_up((unsigned long)start2,MI_SEGMENT_SIZE);
    }

    FSCP_MEM_AREA_INFO area[2];
    area[0].base = start;
    area[0].mem_size = MI_SEGMENT_SIZE;
    area[0].allocated=0;
    area[1].base = start2;
    area[1].mem_size = MI_SEGMENT_SIZE;
    area[1].allocated=0;

    COMO_MALLOC fun1;
    FREE_MEM_FUNCTION fun2;

    //mi_heap_new();
    int ret = setupFscpMemAreas(area,2,fun1,fun2);
    if(ret < 0 ){
      return 0;
    }
    void* p = area_malloc(0,16);
    void* p2 = area_malloc(1,16);
    
    fprintf(stderr, "fscp area(%p) : in heap1 malloc %p\n",start, p);
    fprintf(stderr,"fscp area(%p) : in heap2 malloc %p\n",start2, p2);
    //mi_stats_print(NULL);

    mi_free(p);
    mi_free(p2);

    p2 = area_malloc(1,1025);
    fprintf(stderr,"fscp area(%p) : in heap2 malloc %p\n",start2, p2);
    mi_free(p2);

  return 0;
}
