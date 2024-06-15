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

#ifndef __COMO_CONTEXT_H__
#define __COMO_CONTEXT_H__

#include <pthread.h>
#include "comoref.h"
#include "mutex.h"
#include "lamport_clock.h"

namespace como {

using COMO_MALLOC = void*(*)(Short,size_t);

/**
 * the value bigger than:
 *      ComoConfig::ThreadPoolChannelInvoke_MAX_THREAD_NUM +
 *      ComoContext::gThreadsWorkingNum + 1
 */
#define MAX_NUM_THREADS_WORKING     10

class COM_PUBLIC ComoContext
{
public:
    ComoContext();

    static ComoContext     *gComoContext;
    static pthread_mutex_t  gContextLock;

    static LamportClock    *gLamportClock;

#ifdef COMO_FUNCTION_SAFETY_RTOS
    static pthread_t        gThreadsWorking[MAX_NUM_THREADS_WORKING];
#else
    static pthread_t       *gThreadsWorking;
#endif
    static int              gThreadsWorkingNum;

    // --- Memory Area
    Short               iCurrentMemArea;
    COMO_MALLOC         funComoMalloc;
    FREE_MEM_FUNCTION   freeMemInArea;

    // ---
    Mutex contextLock;

    // --- for phxpaxos
    static void *gEchoServer;       // como::PhxEchoServer
    static int   socketTCP;         // a TCP socket lisetning to 'port' ready to accept connections.

    static ECode SetGctxMemFun(COMO_MALLOC mimalloc, FREE_MEM_FUNCTION mifree);
};

} // namespace como

#endif // __COMO_CONTEXT_H__
