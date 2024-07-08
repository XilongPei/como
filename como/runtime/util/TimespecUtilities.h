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

#ifndef __TIMESPECUTILITIES_H__
#define __TIMESPECUTILITIES_H__

#include <assert.h>
#include <time.h>
#include <climits>
#include <iostream>

namespace como {

class TimespecUtilities
{
public:
                                                      // 123456789
    static constexpr long NANOSECONDS_PER_SECOND      = 1000000000;
    static constexpr long NANOSECONDS_PER_MILLISECOND = 1000000;
    static constexpr long MILLISECONDS_PER_SECOND     = 1000;

    static inline void TimespecAddSeconds(struct timespec& ts, long nanoSeconds)
    {
        long nsec = ts.tv_nsec + nanoSeconds;
        ts.tv_sec += nsec / NANOSECONDS_PER_SECOND;
        ts.tv_nsec += nanoSeconds;
    }

}; // class TimespecUtilities

} // namespace como

#endif  // __TIMESPECUTILITIES_H__
