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

std::string ComoConfig::AddZeroMQEndpoint(std::string serverName, std::string endpoint)
{
    Mutex::AutoLock lock(CZMQUtils_ContextLock);

    ServerNodeInfo *sni = (ServerNodeInfo*)malloc(sizeof(ServerNodeInfo));
    if (nullptr != sni) {
        sni->socket = nullptr;
        sni->endpoint = endpoint;
        ServerNameEndpointMap.emplace(serverName, sni);
        return serverName;
    }

    return nullptr;
}

int ComoConfig::ThreadPool_MAX_THREAD_NUM = 2;

// should less than ThreadPool_MAX_THREAD_NUM
int ComoConfig::ThreadPool_MAX_DBUS_DISPATCHER = 1;

// The period of detecting whether the object is overdue, in ns
long ComoConfig::DBUS_BUS_CHECK_EXPIRES_PERIOD = 1000000000L * 60 * 2;

long ComoConfig::DBUS_BUS_SESSION_EXPIRES = 1000000000L * 60 * 60 * 48;

int ComoConfig::ThreadPoolChannelInvoke_MAX_THREAD_NUM = 2;
CpuInvokeDsa ComoConfig::cpuInvokeDsa[MAX_DSA_IN_ONE_SYSTEM] = {nullptr};

int ComoConfig::ThreadPoolZmqActor_MAX_THREAD_NUM = 2;

/*
 * /como/como/como/runtime/util/comolog.h
 * int Logger::sLevel = DEBUG;
 */
int Logger_sLevel = 0;

// ns, 30s
Long ComoConfig::TPCI_TASK_EXPIRES = 1000000000L * 30;

// ns, 30s
Long ComoConfig::TPZA_TASK_EXPIRES = 1000000000L * 30;

std::unordered_map<std::string, ServerNodeInfo*> ComoConfig::ServerNameEndpointMap;
std::string ComoConfig::ComoRuntimeInstanceIdentity = std::string("localhost");


Mutex ComoConfig::CZMQUtils_ContextLock;

} // namespace como
