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


// reference:
// https://github.com/elizagamedev/ruby-oneshot/blob/master/thread_pthread.c

#include <alloca.h>
#include <pthread.h>
#include <sys/resource.h>
#include "ThreadStack.h"

namespace como {

__attribute__((noinline)) void ThreadStack::rb_gc_set_stack_end(uintptr_t **stack_end_p)
{
    uintptr_t stack_end;
    *stack_end_p = &stack_end;
}


/**
 * fix intermittent SIGBUS on Linux, by reserving the stack virtual address
 * space at process start up, so that it will not clash with the heap space.
 */
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-local-addr"
#endif
__attribute__((noinline)) volatile char *ThreadStack::ReserveStack(volatile char *stack_start, size_t size)
{
    struct rlimit rl;
    volatile char buf[0x100];
    enum {stack_check_margin = 0x1000}; /* for -fstack-check */

    if ((! getrlimit(RLIMIT_STACK, &rl)) && (rl.rlim_cur == RLIM_INFINITY))
        return 0;

    if (size < stack_check_margin) {
        return 0;
    }

    size -= stack_check_margin;

    size -= sizeof(buf);        // margin
#ifdef STACK_DIR_UPPER
    const volatile char *end = buf + sizeof(buf);
    stack_start += size;
    if (stack_start > end) {
        size = stack_start - end;
        stack_start = (volatile char*)alloca(size);
        stack_start[stack_check_margin + size - 1] = 0;
    }
#else
    stack_start -= size;
    if (buf > stack_start) {
        stack_start = (volatile char*)alloca(buf - stack_start);
        stack_start[0] = 0;     // ensure alloca is called
        stack_start -= stack_check_margin;
    }
#endif

    return stack_start;
}
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#define HAVE_PTHREAD_GETATTR_NP
#define CHECK_ERR(expr)             \
    {int err = (expr); if (err) return err;}

/*
 * Get the initial address and size of current thread's stack
 */
int ThreadStack::GetStack(void **addr, size_t *size)
{
#ifdef HAVE_PTHREAD_GETATTR_NP /* Linux */

    pthread_attr_t attr;
    size_t guard = 0;

    CHECK_ERR(pthread_getattr_np(pthread_self(), &attr));
    CHECK_ERR(pthread_attr_getstack(&attr, addr, size));
    *addr = (char *)*addr + *size;
    CHECK_ERR(pthread_attr_getguardsize(&attr, &guard));
    *size -= guard;
    pthread_attr_destroy(&attr);
#elif defined HAVE_PTHREAD_ATTR_GET_NP /* FreeBSD, DragonFly BSD, NetBSD */
    pthread_attr_t attr;
    CHECK_ERR(pthread_attr_init(&attr));
    CHECK_ERR(pthread_attr_get_np(pthread_self(), &attr));
    CHECK_ERR(pthread_attr_getstack(&attr, addr, size));
    *addr = (char *)*addr + *size;
    pthread_attr_destroy(&attr);
#elif (defined HAVE_PTHREAD_GET_STACKADDR_NP && defined HAVE_PTHREAD_GET_STACKSIZE_NP) /* MacOS X */
    pthread_t th = pthread_self();
    *addr = pthread_get_stackaddr_np(th);
    *size = pthread_get_stacksize_np(th);
#elif defined HAVE_THR_STKSEGMENT || defined HAVE_PTHREAD_STACKSEG_NP
    stack_t stk;
    #if defined HAVE_THR_STKSEGMENT /* Solaris */
        CHECK_ERR(thr_stksegment(&stk));
    #else /* OpenBSD */
        CHECK_ERR(pthread_stackseg_np(pthread_self(), &stk));
    #endif
    *addr = stk.ss_sp;
    *size = stk.ss_size;
#elif defined HAVE_PTHREAD_GETTHRDS_NP /* AIX */
    pthread_t th = pthread_self();
    struct __pthrdsinfo thinfo;
    char reg[256];
    int regsiz=sizeof(reg);
    CHECK_ERR(pthread_getthrds_np(&th, PTHRDSINFO_QUERY_ALL,
                   &thinfo, sizeof(thinfo),
                   &reg, &regsiz));
    *addr = thinfo.__pi_stackaddr;
    *size = thinfo.__pi_stacksize;
    STACK_DIR_UPPER((void)0, (void)(*addr = (char *)*addr + *size));
#else
    #error STACKADDR_AVAILABLE is defined but not implemented.
#endif
    return 0;
#undef CHECK_ERR
}

enum {
    RUBY_STACK_SPACE_LIMIT = 1024 * 1024, /* 1024KB */
    RUBY_STACK_SPACE_RATIO = 5
};
int ThreadStack::CheckStackOverflow(const pthread_t thread_id, const void *addr,
                                        size_t stack_maxsize, char *stack_start)
{
    void *base;
    size_t size;
    const size_t water_mark = 1024 * 1024;

    if (GetStack(&base, &size) == 0) {
        #ifdef STACK_DIR_UPPER
            base = (char *)base + size;
        #else
            base = (char *)base - size;
        #endif
    }
    else {
        size = stack_maxsize;
        #ifdef STACK_DIR_UPPER
            base = (char *)stack_start;
        #else
            base = (char *)stack_start - size;
        #endif

        size /= RUBY_STACK_SPACE_RATIO;
        if (size > water_mark) {
            size = water_mark;
            #ifdef STACK_DIR_UPPER
                if (size > ~(size_t)base+1)
                    size = ~(size_t)base+1;
                if (addr > base && addr <= (void *)((char *)base + size))
                    return 1;
            #else
                if (size > (size_t)base)
                    size = (size_t)base;
                if (addr > (void *)((char *)base - size) && addr <= base)
                    return 1;
            #endif
        }
    }
    return 0;
}

} // namespace como
