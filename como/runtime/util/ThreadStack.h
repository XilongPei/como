//=========================================================================
// Copyright (C) 2023 The C++ Component Model(COMO) Open Source Project
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

#ifndef __THREADSTACK_H__
#define __THREADSTACK_H__

#include <stdint.h>

class ThreadStack
{
public:
    static __attribute__((noinline)) void rb_gc_set_stack_end(uintptr_t **stack_end_p);

    #if defined(__x86_64__) && !defined(_ILP32) && defined(__GNUC__) && !defined(__native_client__)
        #define SET_MACHINE_STACK_END(p) __asm__ __volatile__ ("movq\t%%rsp, %0" : "=r" (*(p)))
    #elif defined(__i386) && defined(__GNUC__) && !defined(__native_client__)
        #define SET_MACHINE_STACK_END(p) __asm__ __volatile__ ("movl\t%%esp, %0" : "=r" (*(p)))
    #else
        #define SET_MACHINE_STACK_END(p) rb_gc_set_stack_end(p)
    #endif

    static __attribute__((noinline)) volatile char *ReserveStack(volatile char *limit, size_t size);

    static int GetStack(void **addr, size_t *size);

    static int CheckStackOverflow(const pthread_t thread_id, const void *addr,
                                            size_t stack_maxsize, char *stack_start);
};

#endif  // __THREADSTACK_H__
