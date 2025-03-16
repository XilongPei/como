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

class TimespecUtils
{
public:
                                                      // 123456789
    static constexpr long NANOSECONDS_PER_SECOND      = 1000000000;
    static constexpr long NANOSECONDS_PER_MILLISECOND = 1000000;
    static constexpr long MILLISECONDS_PER_SECOND     = 1000;
    static constexpr long NANOSECONDS_FOR_SCHEDULE    = 20;

    static inline void TimespecAddNanoseconds(struct timespec& ts, long nanoSeconds)
    {
        long nsec = ts.tv_nsec + nanoSeconds;
        ts.tv_sec += nsec / NANOSECONDS_PER_SECOND;
        ts.tv_nsec = nsec % NANOSECONDS_PER_SECOND;
    }

    /**
     * struct timespec ts will be changed when return.
     */
    static inline int TimespecWaitNanoseconds(struct timespec& ts, long nanoSeconds)
    {
        struct timespec currentTime;
        clock_gettime(CLOCK_MONOTONIC, &currentTime);
        if (ts.tv_sec <= 0) {
            ts = currentTime;
        }
        else {
            long long int nsecTs = (long long int)(ts.tv_sec) *
                                            NANOSECONDS_PER_SECOND + ts.tv_nsec;
            long long int nsecCurrentTime = (long long int)(currentTime.tv_sec) *
                                    NANOSECONDS_PER_SECOND + currentTime.tv_nsec;

            if ((nsecCurrentTime - nsecTs) < NANOSECONDS_FOR_SCHEDULE) {
                /**
                 * The difference between `ts` and current time is too short.
                 * Don't sleep
                 */
                return -1;
            }
        }

        long nsec = ts.tv_nsec + nanoSeconds;
        ts.tv_sec += nsec / NANOSECONDS_PER_SECOND;
        ts.tv_nsec = nsec % NANOSECONDS_PER_SECOND;

        while (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, nullptr) == -1) {
            if (errno != EINTR) {
                break;
            }
        }

        return 0;
    }

    static inline long long int TimespecClockGettime(struct timespec *ts)
    {
        struct timespec ct;
        if (nullptr == ts) {
            ts = &ct;
        }

        clock_gettime(CLOCK_MONOTONIC, ts);
        return (long long int)(ts->tv_sec) *
                             TimespecUtils::NANOSECONDS_PER_SECOND + ts->tv_nsec;
    }

}; // class TimespecUtilities

} // namespace como

#endif  // __TIMESPECUTILITIES_H__
