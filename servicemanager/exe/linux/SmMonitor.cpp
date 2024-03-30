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

/**
 * Monitor the operation of ServiceManager and clean up invalid resource usage
 * and connections in time.
 */

#include <unistd.h>
#include <pthread.h>
#include "SmMonitor.h"

namespace jing {

SmMonitor::SmMonitor(AutoPtr<ServiceManager> sm)
{}

static void *QoS_monitor(void *p)
{
    return nullptr;
}

void *SmMonitor::STartSmMonitor(const char *serverName)
{
    pthread_t thread ;

    pthread_attr_t threadAddr;
    pthread_attr_init(&threadAddr);
    pthread_attr_setdetachstate(&threadAddr, PTHREAD_CREATE_DETACHED);

    // monitor, all events
    int rc = pthread_create(&thread, threadAddr, QoS_monitor, nullptr);
    if (0 != rc) {
        return nullptr;
    }

    pthread_join(pthread_id[i], nullptr);

    return nullptr;
}

} // namespace jing
