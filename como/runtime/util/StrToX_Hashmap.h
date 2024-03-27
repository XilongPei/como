//=========================================================================
// Copyright (C) 2024 The C++ Component Model(COMO) Open Source Project
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

#ifndef __STRTOX_HASHMAP__
#define __STRTOX_HASHMAP__

#include <string>
#include "comotypes.h"

namespace como {

class COM_PUBLIC StrToX_Hashmap
{
public:
    StrToX_Hashmap(void *heap, size_t heaplen);

    char **vHashmap(char *key);

    char **vHashmap_stdstring(std::string *key_stdstring, char **pvalue);

    static char **hashmap(char *key, void *heap, ptrdiff_t *heaplen);

    static char **hashmap_stdstring(std::string *key_stdstring, void *heap,
                                            ptrdiff_t *heaplen, char **pvalue);

    size_t  mHeaplen;
    void   *mHeap;
};

} // namespace como

#endif  // __STRTOX_HASHMAP__
