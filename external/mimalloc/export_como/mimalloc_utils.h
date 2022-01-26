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

#ifndef __MIMALLOC_UTILS_H__
#define __MIMALLOC_UTILS_H__

namespace como {

class MimallocUtils {
public:
/*  Management of memory distributed in multiple areas.
    FSCP: Function Safety Computing Platform

The map of multiple areas memory:
    +----+----------+--           -+--------------+--      -+--------+--
    |    | area_0   |  ::::::::::: | area_1       |  :::::: | area_2 |
    +----+----------+--           -+--------------+--      -+--------+--
         base0                     base1                    base2
         0:funFscpCalloc           1:funFscpCalloc          2:funFscpCalloc
*/
    /**
     * Tell mimalloc areas information
     */
    static int setupFscpMemAreas(void *MemAreasInfo, int numAreas);

    void *malloc(size_t size);
    void free (void* ptr);
};

} // namespace como

#endif // __MIMALLOC_UTILS_H__
