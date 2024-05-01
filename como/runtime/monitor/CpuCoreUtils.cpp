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

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include "CpuCoreUtils.h"

namespace como {

int CpuCoreUtils::SetThreadAffinity(pthread_t thread, int iCore)
{
    cpu_set_t cpuset;

    if (0 == thread) {
        thread = pthread_self();
    }

    CPU_ZERO(&cpuset);
    CPU_SET(iCore, &cpuset);

#if defined __GLIBC__ && defined __linux__
   return pthread_setaffinity_np(thread, sizeof(cpuset), &cpuset);
#else
   return sched_setaffinity(thread, sizeof(cpuset), &cpuset);
#endif
}

int CpuCoreUtils::SetProcessAffinity(pid_t pid, int iCore)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(iCore, &cpuset);

    return sched_setaffinity(pid, sizeof(cpuset), &cpuset);
}

// proc - process information pseudo-file system, https://linux.die.net/man/5/proc

#define VMRSS_LINE 17
#define VMSIZE_LINE 13
#define PROCESS_ITEM 14

typedef struct {
    unsigned long user;
    unsigned long nice;
    unsigned long system;
    unsigned long idle;
} TotalCpuOccupy;

typedef struct {
    unsigned int pid;
    unsigned long utime;    // user time
    unsigned long stime;    // kernel time
    unsigned long cutime;   // all user time
    unsigned long cstime;   // all dead time
} ProcCpuOccupy;

// Gets the pointer to the beginning of item n
static const char* get_items(const char*buffer , unsigned int item)
{
    const char *p = buffer;

    int len = strlen(buffer);
    int count = 0;

    for (int i = 0;  i < len;  i++) {
        if (' ' == *p) {
            count ++;
            if (count == (item - 1)) {
                p++;
                break;
            }
        }
        p++;
    }

    return p;
}

// Get total CPU time
unsigned long CpuCoreUtils::GetCpuTotalOccupy()
{
    FILE *fd;
    char buff[1024] = {0};
    TotalCpuOccupy t;

    fd = fopen("/proc/stat", "r");
    if (nullptr == fd) {
        return 0;
    }

    if (fgets(buff, sizeof(buff), fd) == nullptr) {
        return 0;
    }

    char name[64] = {0};
    sscanf(buff, "%s %ld %ld %ld %ld", name, &t.user, &t.nice, &t.system, &t.idle);
    fclose(fd);

    return (t.user + t.nice + t.system + t.idle);
}

// Gets the CPU time of the process
unsigned long CpuCoreUtils::GetCpuProcOccupy(unsigned int pid) {

    char file_name[64] = {0};
    ProcCpuOccupy t;
    FILE *fd;
    char line_buff[1024] = {0};
    sprintf(file_name, "/proc/%d/stat", pid);

    fd = fopen(file_name, "r");
    if (nullptr == fd) {
        return 0;
    }

    if (fgets(line_buff, sizeof(line_buff), fd) == nullptr) {
        return 0;
    }

    sscanf(line_buff, "%u", &t.pid);
    const char *q = get_items(line_buff, PROCESS_ITEM);
/**
 * return ms
 *
 * utime %lu
 * Amount of time that this process has been scheduled in user mode, measured in
 * clock ticks (divide by sysconf(_SC_CLK_TCK)). This includes guest time,
 * guest_time (time spent running a virtual CPU, see below), so that applications
 * that are not aware of the guest time field do not lose that time from their
 * calculations.
 *
 * stime %lu
 * Amount of time that this process has been scheduled in kernel mode, measured
 * in clock ticks (divide by sysconf(_SC_CLK_TCK)).
 *
 * cutime %ld
 * Amount of time that this process's waited-for children have been scheduled in
 * user mode, measured in clock ticks (divide by sysconf(_SC_CLK_TCK)).
 * This includes guest time, cguest_time (time spent running a virtual CPU).
 *
 * cstime %ld
 * Amount of time that this process's waited-for children have been scheduled in
 * kernel mode, measured in clock ticks (divide by sysconf(_SC_CLK_TCK)).
*/
    sscanf(q, "%ld %ld %ld %ld", &t.utime, &t.stime, &t.cutime, &t.cstime);
    fclose(fd);

    return (t.utime + t.stime + t.cutime + t.cstime) * 1000 / sysconf(_SC_CLK_TCK);
}

// Get CPU usage
float CpuCoreUtils::GetProcCpuUsagePercent(unsigned int pid)
{
    unsigned long totalcputime1, totalcputime2;
    unsigned long procputime1, procputime2;

    totalcputime1 = GetCpuTotalOccupy();
    procputime1 = GetCpuProcOccupy(pid);

    usleep(200000);

    totalcputime2 = GetCpuTotalOccupy();
    procputime2 = GetCpuProcOccupy(pid);

    float pcpu = 0.0;
    if (0 != totalcputime2 - totalcputime1) {
        pcpu = 100.0 * (procputime2 - procputime1) / (totalcputime2 - totalcputime1);
    }

    return pcpu;
}

// Get the memory occupied by the process
unsigned int CpuCoreUtils::GetProcMem(unsigned int pid)
{
    char file_name[64] = {0};
    FILE *fd;
    char line_buff[512] = {0};
    sprintf(file_name, "/proc/%d/status", pid);

    fd = fopen(file_name, "r");
    if (nullptr == fd) {
        return 0;
    }

    char name[64];
    int vmrss;
    for (int i = 0;  i < VMRSS_LINE - 1;  i++) {
        if (fgets(line_buff, sizeof(line_buff), fd) == nullptr) {
            return 0;
        }
    }

    if (fgets(line_buff, sizeof(line_buff), fd) == nullptr) {
        return 0;
    }

    sscanf(line_buff, "%s %d", name, &vmrss);
    fclose(fd);

    return vmrss;
}

// Get virtual memory occupied by process
unsigned int CpuCoreUtils::GetProcVirtualmem(unsigned int pid)
{
    char file_name[64] = {0};
    FILE *fd;
    char line_buff[512] = {0};
    sprintf(file_name, "/proc/%d/status", pid);

    fd = fopen(file_name, "r");
    if (nullptr == fd) {
        return 0;
    }

    char name[64];
    int vmsize;
    for (int i = 0;  i < VMSIZE_LINE - 1;  i++) {
        if (fgets(line_buff, sizeof(line_buff), fd) == nullptr) {
            return 0;
        }
    }

    if (fgets(line_buff, sizeof(line_buff), fd) == nullptr) {
        return 0;
    }

    sscanf(line_buff, "%s %d", name, &vmsize);
    fclose(fd);

    return vmsize;
}

// Get Process ID by process_name
int CpuCoreUtils::GetPidByNameAndUser(const char* process_name, const char* user)
{
    if (nullptr == user) {
        user = getlogin();
    }

    char buff[512];
    if (nullptr != user) {
        sprintf(buff, "pgrep %s -u %s", process_name, user);
    }
    else {
        sprintf(buff, "pgrep %s", process_name);
    }

    FILE *pstr = popen(buff, "r");
    if (nullptr == pstr) {
        return 0;
    }

    memset(buff, 0, sizeof(buff));
    if (nullptr == fgets(buff, 512, pstr)) {
        pclose(pstr);
        return 0;
    }

    pclose(pstr);
    return atoi(buff);
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

#if 0
int main(int argc, char *argv[])
{
    unsigned int pid = atoi(argv[1]);

    printf("pid=%d\n", pid);
    printf("pcpu=%f\n", GetProcCpu(pid));
    printf("procmem=%d\n", GetProcMem(pid));
    printf("virtualmem=%d\n", GetProcVirtualmem(pid));
    return 0;
}
#endif
