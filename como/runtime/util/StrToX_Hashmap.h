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

    char **vHashmap(char *key, bool wantToFind=false);

    char **vHashmap_stdstring(std::string *key_stdstring, char **pvalue,
                                                        bool wantToFind=false);

    static char **hashmap(char *key, void *heap, ptrdiff_t *heaplen);

    static char **hashmap_stdstring(std::string *key_stdstring, void *heap,
                                            ptrdiff_t *heaplen, char **pvalue);

    static inline std::string MakeKey_X(char *key, Byte x);
    static inline std::string MakeKey_X(char *key, Short x);
    static inline std::string MakeKey_X(char *key, Integer x);
    static inline std::string MakeKey_X(char *key, Long x);
    static inline std::string MakeKey_X(char *key, Float x);
    static inline std::string MakeKey_X(char *key, Double x);
    static inline std::string MakeKey_X(char *key, Char x);
    static inline std::string MakeKey_X(char *key, Boolean x);
    static inline std::string MakeKey_X(char *key, HANDLE x);
    static inline std::string MakeKey_X(char *key, char *x);

    size_t  mHeaplen;
    void   *mHeap;
};

inline std::string StrToX_Hashmap::MakeKey_X(char *key, Byte x)
{
    return std::string(key, strlen(key)+1) + std::string((char*)&x, sizeof(x));
}

inline std::string StrToX_Hashmap::MakeKey_X(char *key, Short x)
{
    return std::string(key, strlen(key)+1) + std::string((char*)&x, sizeof(x));
}

inline std::string StrToX_Hashmap::MakeKey_X(char *key, Integer x)
{
    return std::string(key, strlen(key)+1) + std::string((char*)&x, sizeof(x));
}

inline std::string StrToX_Hashmap::MakeKey_X(char *key, Long x)
{
    return std::string(key, strlen(key)+1) + std::string((char*)&x, sizeof(x));
}

inline std::string StrToX_Hashmap::MakeKey_X(char *key, Float x)
{
    return std::string(key, strlen(key)+1) + std::string((char*)&x, sizeof(x));
}

inline std::string StrToX_Hashmap::MakeKey_X(char *key, Double x)
{
    return std::string(key, strlen(key)+1) + std::string((char*)&x, sizeof(x));
}

inline std::string StrToX_Hashmap::MakeKey_X(char *key, Char x)
{
    return std::string(key, strlen(key)+1) + std::string((char*)&x, sizeof(x));
}

inline std::string StrToX_Hashmap::MakeKey_X(char *key, Boolean x)
{
    return std::string(key, strlen(key)+1) + std::string((char*)&x, sizeof(x));
}

inline std::string StrToX_Hashmap::MakeKey_X(char *key, HANDLE x)
{
    return std::string(key, strlen(key)+1) + std::string((char*)&x, sizeof(x));
}

inline std::string StrToX_Hashmap::MakeKey_X(char *key, char *x)
{
    return std::string(key, strlen(key)+1) + std::string(x, strlen(x)+1);
}

} // namespace como

#endif  // __STRTOX_HASHMAP__
