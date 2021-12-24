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
#include "checksum.h"

std::unordered_map<std::string, std::string> CZMQUtils::zmq_sockets;

void *comoZmqContext = nullptr;
Mutex comoZmqContextLock;


void *CZMQUtils::zmq_getContext() {
    Mutex::AutoLock lock(comoZmqContextLock);
    if (nullptr == comoZmqContext) {
        comoZmqContext = zmq_ctx_new();
    }
    return comoZmqContext;
}

void *zmq_getSocket(void *context, char *serverName)
{
    void *socket;

    socket = zmq_sockets.find(serverName);
    if (socket != zmq_sockets.end())
        return socket;

    if (nullptr == context)
        context = zmq_getContext();

    void *socket = zmq_socket(context, ZMQ_REQ);
    if (nullptr != socket)
        zmq_sockets.insert(serverName, socket);

    return socket;
}

/**
 * Send an Invoke request through ZeroMQ and wait for the server to return the
 * calculation result
 */
ECode CZMQUtils::zmq_SendWithReplyAndBlock(Integer funCode, void *socket, const void *buf, size_t len)
{
    Long crc64 = crc_64_ecma(reinterpret_cast<const unsigned char *>(buf), len);

    int numberOfBytes;
    struct funCodeAndCRC64 {
        Integer funCode,
        Long crc64
    } = {funCode, crc64};

    numberOfBytes = zmq_send(socket, &funCodeAndCRC64, sizeof(funCodeAndCRC64), ZMQ_SNDMORE);
    numberOfBytes = zmq_send(socket, buf, len, 0);
    // errno

    //int zmq_recv (void *s_, void *buf_, size_t len_, int flags_);
    return NOERROR;
}