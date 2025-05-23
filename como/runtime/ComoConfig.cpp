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

#include "ComoConfig.h"

namespace como {

ComoConfig::ComoConfig() {
    //
}

ServerNodeInfo::ServerNodeInfo()
    : socket(nullptr)
    , inChannel(0)
{}

std::string ComoConfig::AddZeroMQEndpoint(std::string serverName, std::string endpoint)
{
    Mutex::AutoLock lock(CZMQUtils_ContextLock);

    ServerNodeInfo *sni = new ServerNodeInfo();
    if (nullptr != sni) {
        sni->socket = nullptr;
        sni->endpoint = endpoint;
        ServerNameEndpointMap.emplace(serverName, sni);
        return serverName;
    }

    return nullptr;
}

std::string ComoConfig::GetZeroMQEndpoint(std::string serverName)
{
    std::map<std::string, ServerNodeInfo*>::iterator iterBegin =
                                                  ServerNameEndpointMap.begin();

    std::map<std::string, ServerNodeInfo*>::iterator iterFind =
                                         ServerNameEndpointMap.find(serverName);
    if (iterFind != ServerNameEndpointMap.end()) {
        return iterFind->second->endpoint;
    }

    return nullptr;
}

// REM in ThreadPoolExecutor::Worker::Run():
// The number ComoConfig::ThreadPool_MAX_THREAD_NUM must be greater than the
// number ComoConfig::ThreadPool_MAX_DBUS_DISPATCHER
int ComoConfig::ThreadPool_MAX_THREAD_NUM = 2;
int ComoConfig::ThreadPool_MAX_DBUS_DISPATCHER = 1;

// The period of detecting whether the object is overdue
int  ComoConfig::DBUS_CONNECTION_MAX_NUM        = 20;
long ComoConfig::DBUS_BUS_CHECK_EXPIRES_PERIOD  = 300;   // unit second
long ComoConfig::DBUS_BUS_SESSION_EXPIRES       = 1000000000LL * 60 * 60 * 48;
Long ComoConfig::TPCI_TASK_EXPIRES              = 1000000000LL * 30; // ns, 30s
Long ComoConfig::TPZA_TASK_EXPIRES              = 1000000000LL * 30; // ns, 30s
                                                 /*987654321*/

int ComoConfig::ThreadPoolChannelInvoke_MAX_THREAD_NUM = 2;
CpuInvokeDsa ComoConfig::cpuInvokeDsa[MAX_DSA_IN_ONE_SYSTEM] = {nullptr};

int ComoConfig::ThreadPoolZmqActor_MAX_THREAD_NUM = 1;
// RC = Redundant Computation
int ComoConfig::ThreadPoolZmqActor_RC_THREAD_NUM = 0;

int ComoConfig::MAX_SIZE_WorkerList = 100;

char *ComoConfig::localhostTcpEndpoint = (char*)"tcp://127.0.0.1:8083";
char *ComoConfig::localhostInprocEndpoint = (char*)"inproc://workers";

/*
 * /como/como/como/runtime/util/comolog.h
 * int Logger::sLevel = DEBUG;
 */
int Logger_sLevel = 0;

std::map<std::string, ServerNodeInfo*> ComoConfig::ServerNameEndpointMap;

Mutex ComoConfig::CZMQUtils_ContextLock;

FSCP_MEM_AREA_INFO ComoConfig::FscpMemAreaInfo[] = {
    {
        .mem_size  = 0,
        .base      = nullptr,
        .allocated = 0
    },
    {
        .mem_size  = 0,
        .base      = nullptr,
        .allocated = 0
    }
};
int ComoConfig::numFscpMemAreaInfo = sizeof(ComoConfig::FscpMemAreaInfo) / sizeof(FSCP_MEM_AREA_INFO);

std::string ComoConfig::sPhxPaxosDataPath = std::string("/ComoRuntimeData/PhxPaxosData");

int ComoConfig::POOL_SIZE_Parcel = 20;
int ComoConfig::POOL_SIZE_InterfacePack = 20;
int ComoConfig::POOL_SIZE_Channel = 20;
int ComoConfig::POOL_SIZE_Proxy = 20;
int ComoConfig::POOL_SIZE_InterfaceProxy = 20;

CMemPoolSet::MemPoolItem ComoConfig::itemsSharedBuffer[] = {{10, 64, nullptr}, {10, 128, nullptr}, {10, 256, nullptr}};
int ComoConfig::numSharedBuffer = 3;
int ComoConfig::unitNumGeneralBuffer = 0;
int ComoConfig::unitSizeGeneralBuffer = 0;

int ComoConfig::MAX_SIZE_Parcel = 4 * 1024 * 1024;      // 4 MB

} // namespace como
