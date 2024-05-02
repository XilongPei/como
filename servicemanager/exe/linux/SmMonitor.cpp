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
#include <time.h>
#include "comolog.h"
#include "ServiceManager.h"
#include "SmMonitor.h"

static unsigned int gTimeout = 600;

namespace jing {

//using HashMapWalker = void(*)(String&,Key&,Val&,struct timespec&);
static void ServicesWalker(String& str, String& strKey, ServiceManager::InterfacePack*& ipack,
                                                struct timespec& ts)
{
    struct timespec curTime;
    clock_gettime(CLOCK_REALTIME, &curTime);

    Long msts = ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
                          // 654321
    Long msCur = curTime.tv_sec * 1000000 + curTime.tv_nsec / 1000;
                                // 654321

    if ((msCur - msts) < gTimeout) {
        ECode ec;
        AutoPtr<IInterface> object;
        String str = ipack->mServerName;
        if ((nullptr == str) || str.IsEmpty()) {
            ec = CoUnmarshalInterface((como::IInterfacePack*)ipack, RPCType::Local, object);
        }
        else {
            ec = CoUnmarshalInterface((como::IInterfacePack*)ipack, RPCType::Remote, object);
        }

        if (FAILED(ec)) {
            IProxy* proxy = IProxy::Probe(object);
            if (proxy != nullptr) {
                Long state;
                Boolean alive;
                proxy->IsStubAlive(state, alive);
                if (alive) {
                    ts = curTime;
                    return;
                }
            }
        }
    }

    // this service is not alive
    ts = {0, 0};
}

SmMonitor::SmMonitor(AutoPtr<ServiceManager> sm)
{}

static void *QoS_monitor(void *p)
{
    AutoPtr<ServiceManager> instance = ServiceManager::GetInstance();

    while (1) {
        Mutex::AutoLock lock(instance->mServicesLock);
        String strBuffer;
        instance->mServices.GetValues(strBuffer, ServicesWalker);

        // Clean up services that don't work
        instance->mServices.CleanUpExpiredData();

        sleep(SmMonitor::CHECK_EXPIRES_PERIOD);
    }

    return nullptr;
}

void *SmMonitor::StartSmMonitor(const char *serverName)
{
    pthread_t thread ;

    pthread_attr_t threadAddr;
    pthread_attr_init(&threadAddr);
    pthread_attr_setdetachstate(&threadAddr, PTHREAD_CREATE_DETACHED);

    // monitor, all events
    int rc = pthread_create(&thread, &threadAddr, QoS_monitor, nullptr);
    if (0 != rc) {
        return nullptr;
    }

    // Waits for the end of the thread specified by thread in a blocking manner
    if (pthread_join(thread, nullptr)) {
        Logger_E("SmMonitor::StartSmMonitor", "pthread_join() failed");
    }

    return nullptr;
}

} // namespace jing
