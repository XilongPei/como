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

// Ref: https://gitee.com/tjopenlab/scratch/blob/master/misc/hashmap.c
/**
 * about 0x100000001b3:
 * Fowler–Noll–Vo hash function
 * Ref: https://gitee.com/tjopenlab/como-grammar-manual/blob/master/%E5%BC%80%E5%8F%91COMO%E5%8F%82%E8%80%83%E6%96%87%E6%A1%A3/Fowler%E2%80%93Noll%E2%80%93Vo%20hash%20function%20-%20Wikipedia.pdf
 * The FNV_prime is the 64-bit FNV prime value: 1099511628211 (in hex, 0x100000001b3).
 */

// A compact, single function string-to-string hash map
// Ref: https://old.reddit.com/r/C_Programming/comments/18iwhw0
// Ref: https://nullprogram.com/blog/2023/10/05/
// This is free and unencumbered software released into the public domain.
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "StrToX_Hashmap.h"

namespace como {

StrToX_Hashmap::StrToX_Hashmap(void *heap, size_t heaplen)
    : mHeap(heap)
    , mHeaplen(heaplen)
{
    hashmap(0, mHeap, (ptrdiff_t*)&mHeaplen);
}

/**
 * vHashmap
 */
char **StrToX_Hashmap::vHashmap(char *key, bool wantToFind)
{
    if (wantToFind) {
        return hashmap(key, mHeap, (ptrdiff_t*)0);
    }

    return hashmap(key, mHeap, (ptrdiff_t*)&mHeaplen);
}

/**
 * vHashmap_stdstring
 */
char **StrToX_Hashmap::vHashmap_stdstring(std::string *key_stdstring,
                                                 char **pvalue, bool wantToFind)
{
    if (wantToFind) {
        return hashmap_stdstring(key_stdstring, mHeap, (ptrdiff_t*)0, pvalue);
    }

    return hashmap_stdstring(key_stdstring, mHeap, (ptrdiff_t*)&mHeaplen, pvalue);
}


// Initialization: Call with null key. A heap can have only one map at a
// time, and all alloctions will come from this heap. The heap must be
// pointer-aligned and have space for at least two pointers.
//
// Upsert: Pass both a key and heaplen. Returns a pointer to the value
// which you can populate. Stores a copy of the key in the map, and
// updates *heaplen on allocation. Returns null for out-of-memory.
//
// Lookup: Pass null for heaplen, which inhibits allocation. Returns
// null if no such entry exists.
//
// Iterate (insertion order): Pass a null key and heaplen==heap to get
// the first key pointer. Use this key with heaplen==heap to retrieve
// the next key. Returns null from the final key. You _must_ use the
// returned key during iteration, not a copy. Iteration has no internal
// state and can occur concurrently. The return value is a key/value
// tuple and the value is at the second index.
//
// Delete: Lookup and set the value to a gravestone of your choice.

/**
 * hashmap
 */
char **StrToX_Hashmap::hashmap(char *key, void *heap, ptrdiff_t *heaplen)
{
    enum { ARY = 2 }; // 1=slowest+small, 2=faster+larger, 3=fastest+large
    typedef struct node node;
    struct node {
        node *child[1 << ARY];
        node *next;
        char *key;
        char *value;
    };
    struct tagMap {
        node  *head;
        node **tail;
    } *map = (struct tagMap *)heap;

    if ((nullptr == key) && (heaplen != heap)) {     // init
        *heaplen &= -sizeof(void *);        // align high end
        map->head = nullptr;
        map->tail = &map->head;
        return nullptr;
    }
    else if ((nullptr == key) && (heaplen == heap)) { // first key
        return map->head ? &map->head->key : nullptr;
    }
    else if (heaplen == heap) {             // next key
        node *next = ((node *)(key - sizeof(node)))->next;
        return next ? &next->key : nullptr;
    }

    uint64_t hash = 0x100u;
    ptrdiff_t keylen = 0;
    for (;  key[keylen++] != '\0';  hash *= 0x100000001b3u) {
        hash ^= (unsigned char)key[keylen];
    }

    node **n = &(map->head);
    for (;  *n != nullptr;  hash <<= ARY) {
        if (strcmp(key, (*n)->key) == 0) {
            return &(*n)->value;
        }
        n = &(*n)->child[hash >> (64 - ARY)];   // 64 = sizeof(hash) * 8
    }
    if (nullptr == heaplen) {
        return nullptr;                         // lookup failed
    }

    ptrdiff_t total = sizeof(node) + keylen + (-keylen & (sizeof(void *) - 1));
    if (*heaplen - (ptrdiff_t)sizeof(*map) < total) {
        return nullptr;                         // out of memory
    }

    *n = (node *)((char *)heap + (*heaplen -= total));
    memset(*n, 0, sizeof(node));

    (*n)->key = (char *)*n + sizeof(node);
    memcpy((*n)->key, key, keylen);

    *map->tail = *n;
    map->tail = &(*n)->next;

    return &(*n)->value;
}

/**
 * hashmap_stdstring
 *
 * key_stdstring = std::string(key\0value\0)
 */
char **StrToX_Hashmap::hashmap_stdstring(std::string *key_stdstring, void *heap, ptrdiff_t *heaplen, char **pvalue)
{
    enum { ARY = 2 }; // 1=slowest+small, 2=faster+larger, 3=fastest+large
    typedef struct node node;
    struct node {
        node *child[1 << ARY];
        node *next;
        char *key;
    };
    struct tagMap {
        node  *head;
        node **tail;
    } *map = (struct tagMap *)heap;

    if ((nullptr == key_stdstring) && (heaplen != heap)) {       // init
        *heaplen &= -sizeof(void *);                    // align high end
        map->head = nullptr;
        map->tail = &map->head;
        return nullptr;
    }
    else if ((nullptr == key_stdstring) && (heaplen == heap)) {  // first key
        return map->head ? &map->head->key : nullptr;
    }
    else if (heaplen == heap) {                                  // next key
        node *next = ((node *)((char *)key_stdstring - sizeof(node)))->next;
        return next ? &next->key : nullptr;
    }

    uint64_t hash = 0x100u;
    ptrdiff_t keylen = 0;
    const char *key = key_stdstring->c_str();
    for (;  key[keylen++] != '\0';  hash *= 0x100000001b3u) {
        hash ^= (unsigned char)key[keylen];
    }

    node **n = &map->head;
    for (;  *n != nullptr;  hash <<= ARY) {
        if (strcmp(key, (*n)->key) == 0) {
            *pvalue = (*n)->key;
            *pvalue += strlen(*pvalue) + 1;             // +1 for tail '\0'

            return pvalue;
        }
        n = &(*n)->child[hash >> (64 - ARY)];           // 64 = sizeof(hash) * 8
    }
    if (nullptr == heaplen) {
        return nullptr;                                 // lookup failed
    }

    keylen = key_stdstring->size();
    ptrdiff_t total = sizeof(node) + keylen + (-keylen & (sizeof(void *) - 1));
    if (*heaplen - (ptrdiff_t)sizeof(*map) < total) {
        return nullptr;                     // out of memory
    }

    *n = (node *)((char *)heap + (*heaplen -= total));
    memset(*n, 0, sizeof(node));

    (*n)->key = (char *)*n + sizeof(node);
    memcpy((*n)->key, key, keylen);

    *map->tail = *n;
    map->tail = &(*n)->next;

    return pvalue;
}

} // namespace como
