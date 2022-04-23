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
long ComoConfig::DBUS_BUS_SESSION_EXPIRES       = 1000000000L * 60 * 60 * 48;
Long ComoConfig::TPCI_TASK_EXPIRES              = 1000000000L * 30; // ns, 30s
Long ComoConfig::TPZA_TASK_EXPIRES              = 1000000000L * 30; // ns, 30s
                                                 /*987654321*/

int ComoConfig::ThreadPoolChannelInvoke_MAX_THREAD_NUM = 2;
CpuInvokeDsa ComoConfig::cpuInvokeDsa[MAX_DSA_IN_ONE_SYSTEM] = {nullptr};

int ComoConfig::ThreadPoolZmqActor_MAX_THREAD_NUM = 2;

/*
 * /como/como/como/runtime/util/comolog.h
 * int Logger::sLevel = DEBUG;
 */
int Logger_sLevel = 0;

std::map<std::string, ServerNodeInfo*> ComoConfig::ServerNameEndpointMap;

// identity of this ZeroMQ node
std::string ComoConfig::ComoRuntimeInstanceIdentity = std::string("localhost");

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
int ComoConfig::sizeofFscpMemAreaInfo = sizeof(ComoConfig::FscpMemAreaInfo) / sizeof(FSCP_MEM_AREA_INFO);

std::string ComoConfig::sPhxPaxosDataPath = std::string("/ComoRuntimeData/PhxPaxosData");

} // namespace como
