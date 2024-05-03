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

#include "util/comolog.h"
#include "ComoContext.h"
#include "ComoConfig.h"
#include "mimalloc_utils.h"

namespace como {

ComoContext *ComoContext::gComoContext = nullptr;
pthread_mutex_t ComoContext::gContextLock = PTHREAD_MUTEX_INITIALIZER;

LamportClock *ComoContext::gLamportClock = new LamportClock();
pthread_t *ComoContext::gThreadsWorking = nullptr;
int ComoContext::gThreadsWorkingNum = 0;

void *ComoContext::gEchoServer = nullptr;
int   ComoContext::socketTCP = 0;

ComoContext::ComoContext()
    : funComoMalloc(nullptr)
    , freeMemInArea(nullptr)
{
    if (ComoConfig::numFscpMemAreaInfo > 0) {
/*
        int ret =  MimallocUtils::setupFscpMemAreas(ComoConfig::FscpMemAreaInfo,
                                                    ComoConfig::numFscpMemAreaInfo,
                                                    funComoMalloc, freeMemInArea);
        if (0 != ret) {
            Logger::E("ComoContext", "new ComoContext error, errno: %d", ret);
        }
*/    }
}

/**
 * Set_gComoContext_MemFun
 */
ECode ComoContext::SetGctxMemFun(COMO_MALLOC mimalloc, FREE_MEM_FUNCTION mifree)
{
    if (nullptr == gComoContext) {
        gComoContext = new ComoContext();
    }

    if (nullptr == gComoContext) {
        return E_OUT_OF_MEMORY_ERROR;
    }

    gComoContext->funComoMalloc = mimalloc;
    gComoContext->freeMemInArea = mifree;

    return NOERROR;
}

} // namespace como
