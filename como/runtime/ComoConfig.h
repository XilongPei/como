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

#include <map>
#include <string>
#include "comotypes.h"
#include "mutex.h"

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


class ServerNodeInfo {
public:
    ServerNodeInfo();

    void *socket;
    std::string endpoint;
    int inChannel;
};


// FSCP: Function Safety Computing Platform
typedef struct tagFSCP_MEM_AREA_INFO {
    size_t mem_size;        // Size of space to be managed
    void  *base;            // Base address of space to be managed
    size_t allocated;       // Allocated size
} FSCP_MEM_AREA_INFO;

class COM_PUBLIC ComoConfig
{
public:
    ComoConfig();

    static std::string AddZeroMQEndpoint(std::string serverName, std::string endpoint);
    static std::string GetZeroMQEndpoint(std::string serverName);

    // REM in ThreadPoolExecutor::Worker::Run():
    // The number ComoConfig::ThreadPool_MAX_THREAD_NUM must be greater than the
    // number ComoConfig::ThreadPool_MAX_DBUS_DISPATCHER
    static int ThreadPool_MAX_THREAD_NUM;
    static int ThreadPool_MAX_DBUS_DISPATCHER;

    static int  DBUS_CONNECTION_MAX_NUM;
    static long DBUS_BUS_CHECK_EXPIRES_PERIOD;
    static long DBUS_BUS_SESSION_EXPIRES;
    // TPCI: Thread Pool Channel Invoke
    static Long TPCI_TASK_EXPIRES;
    // TPZA: Thread Pool ZMQ Actor
    static Long TPZA_TASK_EXPIRES;

    static char *localhostTcpEndpoint;
    static char *localhostInprocEndpoint;

    static int ThreadPoolChannelInvoke_MAX_THREAD_NUM;
    static int Logger_sLevel;

    static int ThreadPoolZmqActor_MAX_THREAD_NUM;

    static CpuInvokeDsa cpuInvokeDsa[MAX_DSA_IN_ONE_SYSTEM];

    static std::map<std::string, ServerNodeInfo*> ServerNameEndpointMap;

    static Mutex CZMQUtils_ContextLock;

    static FSCP_MEM_AREA_INFO FscpMemAreaInfo[];
    static int sizeofFscpMemAreaInfo;

    static std::string sPhxPaxosDataPath;

    static int POOL_SIZE_Parcel;
    static int POOL_SIZE_InterfacePack;
    static int POOL_SIZE_Channel;
    static int POOL_SIZE_Proxy;
    static int POOL_SIZE_InterfaceProxy;
};

// defined in VerifiedU64Pointer.cpp
extern int gUnfixedMemError;
extern int gFixedMemError;

} // namespace como

#endif // __COMO_CONFIG_H__
