//=========================================================================
// Copyright (C) 2018 The C++ Component Model(COMO) Open Source Project
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

#ifndef __CDLC_METADATA_H__
#define __CDLC_METADATA_H__

#include "runtime/metadata/Component.h"
#include "runtime/metadata/MetadataSerializer.h"

#define ALIGN4(v) (((v) + 3) & ~3)
#define ALIGN8(v) (((v) + 7) & ~7)

#if defined(__i386__) || defined(__arm__)
    #define ALIGN(v) ALIGN4(v)
#elif defined(__x86_64__) || defined(__aarch64__)
    #define ALIGN(v) ALIGN8(v)
#else
    #if defined(__riscv)
        #if (__riscv_xlen == 32)
        #define ALIGN(v)        ALIGN4(v)
        #elif (__riscv_xlen == 64)
        #define ALIGN(v)        ALIGN8(v)
        #endif
    #endif
#endif

#endif // __CDLC_METADATA_H__
