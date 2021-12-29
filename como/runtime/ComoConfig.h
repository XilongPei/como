//=========================================================================
// Copyright (C) 2021 The C++ Component Model(COMO) Open Source Project
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

#ifndef __COMO_CONFIG_H__
#define __COMO_CONFIG_H__

#include <unordered_map>
#include "comotypes.h"

namespace como {

// CPU, DSA(Domain Specific Architecture)
using CpuInvokeDsa = ECode(*)(
    /* [in] */ HANDLE func,
    /* [in] */ Byte* params,
    /* [in] */ Integer paramNum,
    /* [in] */ Integer stackParamNum,
    /* [in] */ struct ParameterInfo* paramInfos);

#define MAX_DSA_IN_ONE_SYSTEM   2
#define ENABLE_RUNTIME_LOGGER   1

class COM_PUBLIC ComoConfig
{
public:
    ComoConfig();

    static std::string AddZeroMQEndpoint(std::string serverName, std::string endpoint);

    static int ThreadPool_MAX_THREAD_NUM;
    static int ThreadPool_MAX_DBUS_DISPATCHER;

    static long DBUS_BUS_CHECK_EXPIRES_PERIOD;
    static long DBUS_BUS_SESSION_EXPIRES;
    static int ThreadPoolChannelInvoke_MAX_THREAD_NUM;
    static int Logger_sLevel;

    static int ThreadPoolZmqActor_MAX_THREAD_NUM;

    static CpuInvokeDsa cpuInvokeDsa[MAX_DSA_IN_ONE_SYSTEM];

    // TPCI: Thread Pool Channel Invoke
    static Long TPCI_TASK_EXPIRES;

    // TPZA: Thread Pool ZMQ Actor
    static Long TPZA_TASK_EXPIRES;

    static std::unordered_map<std::string, std::string> ServerNameEndpointMap;
    static std::string ComoRuntimeInstanceIdentity;
};

} // namespace como

#endif // __COMO_CONFIG_H__
