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

#include <inttypes.h>
#include <stdio.h>
#include "como/core/CStringBuilder.h"
#include "como/core/StackTraceElement.h"
#include <comosp.h>

using como::io::IID_ISerializable;

#define PAD_PTR "016" PRIxPTR

namespace como {
namespace core {

COMO_INTERFACE_IMPL_2(StackTraceElement, SyncObject, IStackTraceElement, ISerializable);

ECode StackTraceElement::Constructor(
    /* [in] */ Integer no,
    /* [in] */ HANDLE pc,
    /* [in] */ const String& soname,
    /* [in] */ const String& symbol,
    /* [in] */ HANDLE relOffset,
    /* [in] */ HANDLE thisPointer)
{
    mNo = no;
    mPC = pc;
    mSoname = soname;
    mSymbol = symbol;
    mRelOffset = relOffset;
    mThisPointer = thisPointer;
    return NOERROR;
}

ECode StackTraceElement::ToString(
    /* [out] */ String& desc)
{
    char buf[1024];
    if (mSymbol != nullptr) {
        /* The result of `buf` is similar to this:
            #0  pc 0x7f635454d795 ./lib/libdebug/libdebug.so (libdebugfunccrash+0x1b)
            #1  pc 0x7f635454d7b3 ./lib/libdebug/libdebug.so (libdebugfunc5+0x15)
        */
        snprintf(buf, sizeof(buf),
                            "#%02d  pc %" PAD_PTR "  %s (%s+%" PRIuPTR ")\n", mNo,
                            mPC, mSoname.string(), mSymbol.string(), mRelOffset);
    }
    else {
        snprintf(buf, sizeof(buf),
                            "#%02d  pc %" PAD_PTR "  %s\n", mNo, mPC, mSoname.string());
    }
    desc = buf;

    return NOERROR;
}

} // namespace core
} // namespace como
