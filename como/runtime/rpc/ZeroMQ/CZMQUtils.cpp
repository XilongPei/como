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

#include <unordered_map>
#include "zmq.h"

std::unordered_map<std::string, std::string> CZMQUtils::zmq_sockets;

CZMQUtils::zmq_getContext() {
    return zyq_init(1);
}


void *zmq_getSocket(void *context, char *serverName, int type_)
{
    void *socket = zmq_socket(context, type_);
    zmq_sockets(serverName, socket);
}

/**
 * Send an Invoke request through ZeroMQ and wait for the server to return the
 * calculation result
 */
ECode CZMQUtils::zmq_SendWithReplyAndBlock()
{
    return NOERROR;
}