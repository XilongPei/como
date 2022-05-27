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

#include <stdarg.h>
#include <stdlib.h>
#include "como/util/MemUtils.h"

namespace como {
namespace util {

int MemUtils::CheckSomeVoidP(int cnt, ...)
{
    void *vp;

    va_list argptr;
    va_start(argptr, cnt);

    int num = 0;
    for (int i = 0;  i < cnt;  i++) {
        vp = va_arg(argptr, void*);
        if (nullptr != vp) {
            num++;
        }
    }
    va_end(argptr);

    return num;
}

void MemUtils::FreeSomeVoidP(int cnt, ...)
{
    void *vp;

    va_list argptr;
    va_start(argptr, cnt);

    for (int i = 0;  i < cnt;  i++) {
        vp = va_arg(argptr, void*);
        if (nullptr != vp) {
            free(vp);
        }
    }
    va_end(argptr);
}

} // namespace util
} // namespace como
