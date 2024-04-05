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

#define __IN_COMOTEST__

#include <stdio.h>
#include <stdlib.h>
#include <gtest/gtest.h>
#include "StrToX_Hashmap.h"

using namespace como;

TEST(StrToX_Hashmap, Test_hashmap)
{
    ptrdiff_t len = 1 << 16;
    void *heap = malloc(len);
    StrToX_Hashmap::hashmap(0, heap, &len);

    *StrToX_Hashmap::hashmap((char*)"hello", heap, &len) = (char *)"world";
    *StrToX_Hashmap::hashmap((char*)"foo", heap, &len) = (char *)"bar";
    puts(*StrToX_Hashmap::hashmap((char*)"foo", heap, 0));
    puts(*StrToX_Hashmap::hashmap((char*)"hello", heap, 0));

    for (char **e = 0; (e = StrToX_Hashmap::hashmap(e ? *e : 0, heap, (ptrdiff_t*)heap));) {
        printf("k:%s\tv:%s\n", e[0], e[1]);
    }

    /////////////////////////////////////////////

    char *pvalue;
    StrToX_Hashmap::hashmap_stdstring(0, heap, &len, &pvalue);

    std::string str = std::string("hello", 6) + std::string("world", 6);
    StrToX_Hashmap::hashmap_stdstring(&str, heap, &len, &pvalue);

    str = std::string("foo", 4) + std::string("bar", 4);
    StrToX_Hashmap::hashmap_stdstring(&str, heap, &len, &pvalue);

    str = std::string("foo");
    puts(*StrToX_Hashmap::hashmap_stdstring(&str, heap, 0, &pvalue));

    str = std::string("hello");
    puts(*StrToX_Hashmap::hashmap_stdstring(&str, heap, 0, &pvalue));

    for (char **e = 0; (e = StrToX_Hashmap::hashmap_stdstring(e ? (std::string *)*e : 0, heap,
                                                (ptrdiff_t*)heap, &pvalue));) {
        printf("k:%s\tv:%s\n", e[0], &e[0][strlen(e[0]) + 1]);
    }

    free(heap);  // destroy the hashmap
}

TEST(StrToX_Hashmap, Test_StrToX_Hashmap)
{
    void *heap = malloc(1 << 16);
    StrToX_Hashmap *map = new StrToX_Hashmap(heap, 1 << 16);
    *(map->vHashmap((char*)"hello")) = (char *)"world";
    puts(*(map->vHashmap((char*)"hello", true)));
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
