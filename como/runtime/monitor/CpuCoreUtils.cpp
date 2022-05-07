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

#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include "CpuCoreUtils.h"

namespace como {

int CpuCoreUtils::SetThreadAffinity(pthread_t thread, int iCore)
{
    cpu_set_t cpuset;

    if (0 == thread)
        thread = pthread_self();

    CPU_ZERO(&cpuset);
    CPU_SET(iCore, &cpuset);

   return pthread_setaffinity_np(thread, sizeof(cpuset), &cpuset);
}

int CpuCoreUtils::SetProcessAffinity(pid_t pid, int iCore)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(iCore, &cpuset);

    return sched_setaffinity(pid, sizeof(cpuset), &cpuset);
}

} // namespace como

#if 0
#include <stdio.h>
#define handle_error_en(en, msg) \
       do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

int
main(int argc, char *argv[])
{
   int s;
   cpu_set_t cpuset;
   pthread_t thread;

   thread = pthread_self();

   /* Set affinity mask to include CPUs 0 to 7. */

   CPU_ZERO(&cpuset);
   for (int j = 0; j < 8; j++)
       CPU_SET(j, &cpuset);

   s = pthread_setaffinity_np(thread, sizeof(cpuset), &cpuset);
   if (s != 0)
       handle_error_en(s, "pthread_setaffinity_np");

   /* Check the actual affinity mask assigned to the thread. */

   s = pthread_getaffinity_np(thread, sizeof(cpuset), &cpuset);
   if (s != 0)
       handle_error_en(s, "pthread_getaffinity_np");

   printf("Set returned by pthread_getaffinity_np() contained:\n");
   for (int j = 0; j < CPU_SETSIZE; j++)
       if (CPU_ISSET(j, &cpuset))
           printf("    CPU %d\n", j);

   exit(EXIT_SUCCESS);
}
#endif
