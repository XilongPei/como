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

#include "mutex.h"

namespace como {

using COMO_CALLOC = void*(*)(size_t, size_t);

class COM_PUBLIC ComoContext
{
public:
    ComoContext();

    static ComoContext *gComoContext;

    //static void *gMemArea;          // in ComoConfig
    Integer iCurrentMemArea;
    COMO_CALLOC funComoCalloc;

    Mutex contextLock;
};

} // namespace como

#endif // __COMO_CONTEXT_H__
