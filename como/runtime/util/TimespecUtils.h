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

#include <time.h>
#include <stdint.h>
#include <errno.h>

namespace como {

class TimespecUtils
{
public:
                                                         // 123456789
    static constexpr int64_t NANOSECONDS_PER_SECOND      = 1000000000LL;
    static constexpr int64_t NANOSECONDS_PER_MILLISECOND = 1000000LL;
    static constexpr int64_t MILLISECONDS_PER_SECOND     = 1000LL;
    static constexpr int64_t NANOSECONDS_FOR_SCHEDULE    = 20LL;
    static constexpr int64_t SPIN_AHEAD_NS               = 100000LL;

    static inline void TimespecAddNanoseconds(struct timespec& ts, int64_t nanoSeconds)
    {
        int64_t nsec = (int64_t)(ts.tv_sec) * NANOSECONDS_PER_SECOND +
                                                       ts.tv_nsec + nanoSeconds;

        ts.tv_sec = nsec / NANOSECONDS_PER_SECOND;
        ts.tv_nsec = nsec % NANOSECONDS_PER_SECOND;
    }

    /**
     * struct timespec ts will be changed when return.
     */
    static inline int TimespecWaitNanoseconds(struct timespec& ts, int64_t nanoSeconds)
    {
        struct timespec currentTime;
        clock_gettime(CLOCK_MONOTONIC, &currentTime);
        if (ts.tv_sec <= 0) {
            ts = currentTime;
        }
        else {
            int64_t nsecTs = (int64_t)(ts.tv_sec) *
                                            NANOSECONDS_PER_SECOND + ts.tv_nsec;
            int64_t nsecCurrentTime = (int64_t)(currentTime.tv_sec) *
                                    NANOSECONDS_PER_SECOND + currentTime.tv_nsec;

            nsecTs += nanoSeconds;

            /**
             * =======+===========+============== timeline ======
             *        ^           ^
             *        |           + nsecCurrentTime
             *        + nsecTs(Time point to wait)
             */
            if (nsecTs < nsecCurrentTime) {
                /**
                 * The time for me to wait has expired.
                 * Don't sleep
                 */
                return -1;
            }

            /**
             * =======+====+============== timeline ======
             *        ^    ^
             *        |    + nsecTs(Time point to wait)
             *        + nsecCurrentTime
             */
            if ((nsecTs - nsecCurrentTime) < NANOSECONDS_FOR_SCHEDULE) {
                /**
                 * The time for me to wait is too short.
                 * Don't sleep
                 */
                return -2;
            }
        }

        TimespecAddNanoseconds(ts, nanoSeconds);

#ifdef __NOT_PRECISE_SLEEP_UNTIL__
        /**
         * 1. If you use CLOCK_MONOTONIC as the clock_id, the change of system time
         * will not affect dormancy.
         * 2. You can use the SA_RESTART flag to prevent certain signals from
         * interrupting the system call, but this will not return the remaining
         * time - the call will be automatically restarted.
         * 3. When using the TIMER_ABSTIME flag (absolute time), even if there is a
         * signal interruption, rem may not return valid values because the
         * remaining time is no longer tracked.
         */
        while (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts,
                                                                nullptr) == -1) {
            if (errno != EINTR) {
                break;
            }
        }
#else
        currentTime = ts;
        TimespecAddNanoseconds(currentTime, -SPIN_AHEAD_NS);

        while (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &currentTime,
                                                                nullptr) == -1) {
            if (errno != EINTR) {
                break;
            }
        }

        do {
            clock_gettime(CLOCK_MONOTONIC, &currentTime);
        } while (TimespecDiffNs(&currentTime, &ts) < 0);
#endif
        return 0;
    }

    static inline int64_t TimespecClockGettime(struct timespec *ts)
    {
        struct timespec ct;
        if (nullptr == ts) {
            ts = &ct;
        }

        clock_gettime(CLOCK_MONOTONIC, ts);
        return (int64_t)(ts->tv_sec) * NANOSECONDS_PER_SECOND + ts->tv_nsec;
    }

    static inline int64_t TimespecToInt64time(struct timespec *ts)
    {
        if (nullptr == ts) {
            return 0;
        }

        return (int64_t)(ts->tv_sec) * NANOSECONDS_PER_SECOND + ts->tv_nsec;
    }

    static inline int64_t TimespecDiffNs(const struct timespec *a,
                                                       const struct timespec *b)
    {
        return (int64_t)(a->tv_sec - b->tv_sec) * NANOSECONDS_PER_SECOND +
                                                      (a->tv_nsec - b->tv_nsec);
    }

}; // class TimespecUtilities

} // namespace como

#endif  // __TIMESPECUTILITIES_H__
