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

#include <pthread.h>
#include <sched.h>

namespace como {

class CpuCoreUtils {
public:
    static int SetThreadAffinity(pthread_t thread, int iCore);
    static int SetProcessAffinity(pid_t pid, int iCore);
    static unsigned long GetCpuTotalOccupy();

    // return ms, millisecond
    static unsigned long GetCpuProcOccupy(unsigned int pid);

    // CPU usage percentage
    static float GetProcCpu(unsigned int pid);

    static unsigned int GetProcMem(unsigned int pid);
    static unsigned int GetProcVirtualmem(unsigned int pid);
    static int GetPidByNameAndUser(const char* process_name, const char* user = nullptr);
};

} // namespace como
