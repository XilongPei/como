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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <sys/mman.h>
#include <gtest/gtest.h>
#include "TimespecUtils.h"

#define PERIOD_NS        1000000L     // 1ms
#define SPIN_AHEAD_NS    50000L       // 提前50us
#define CPU_CORE_ID      3            // 默认绑定 CPU 3
#define CYCLE_COUNT      10

using namespace como;

void set_realtime_priority(int prio) {
    struct sched_param param = { .sched_priority = prio };
    if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
        perror("sched_setscheduler");
        exit(EXIT_FAILURE);
    }
}

void lock_memory() {
    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
        perror("mlockall");
        exit(EXIT_FAILURE);
    }
}

void bind_to_cpu(int cpu_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);
    if (sched_setaffinity(0, sizeof(cpuset), &cpuset) != 0) {
        perror("sched_setaffinity");
        exit(EXIT_FAILURE);
    }
}

TEST(testTimespecUtils, basic)
{
    struct timespec ts = {0, 0};
    int ret = TimespecUtils::TimespecWaitNanoseconds(ts, 1000);
    EXPECT_EQ(ret, 0);
}

TEST(testTimespecUtils, basic_highres_timer)
{
    lock_memory();
    //set_realtime_priority(80);
    //bind_to_cpu(CPU_CORE_ID);

    struct timespec next;
    clock_gettime(CLOCK_MONOTONIC, &next);

    int64_t min_jitter = INT64_MAX;
    int64_t max_jitter = 0;
    int64_t total_jitter = 0;

    //printf("Running on CPU %d...\n\n", CPU_CORE_ID);

    for (int i = 0;  i < CYCLE_COUNT;  i++) {
        struct timespec target = next;
        TimespecUtils::TimespecAddNanoseconds(target, PERIOD_NS);

        TimespecUtils::TimespecWaitNanoseconds(next, PERIOD_NS);

        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);

        int64_t jitter_ns = TimespecUtils::TimespecDiffNs(&now, &target);
        if (jitter_ns < 0) {
            jitter_ns = 0;
        }

        if (jitter_ns > max_jitter) {
            max_jitter = jitter_ns;
        }
        if (jitter_ns < min_jitter) {
            min_jitter = jitter_ns;
        }
        total_jitter += jitter_ns;

        printf("Cycle %4d | jitter = %6ld ns | time = %ld.%09ld\n",
               i, jitter_ns, now.tv_sec, now.tv_nsec);
    }

    printf("\nJitter Stats:\n");
    printf("  Min  = %ld ns\n", min_jitter);
    printf("  Max  = %ld ns\n", max_jitter);
    printf("  Avg  = %ld ns\n", total_jitter / CYCLE_COUNT);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
