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
#include "Czmq.h"
#include "checksum.h"

namespace como {

class EndpointSocket {
    ~EndpointSocket() {
        int rc = Czmq_close(socket);
    }

public:
    std::string endpoint;
    void *socket;
};

std::unordered_map<std::string, EndpointSocket*> CZMQUtils::Czmq_sockets;

void *comoZmqContext = nullptr;
Mutex comoZmqContextLock;


void *CZMQUtils::CzmqGetContext() {
    Mutex::AutoLock lock(comoZmqContextLock);
    if (nullptr == comoZmqContext) {
        comoZmqContext = Czmq_ctx_new();
    }
    return comoZmqContext;
}

/**
 * ZMQ_REQ
 * ^^^^^^^
 * A socket of type 'ZMQ_REQ' is used by a _client_ to send requests to and
 * receive replies from a _service_. This socket type allows only an alternating
 * sequence of _Czmq_send(request)_ and subsequent _Czmq_recv(reply)_ calls. Each
 * request sent is round-robined among all _services_, and each reply received is
 * matched with the last issued request.
 * For connection-oriented transports, If the ZMQ_IMMEDIATE option is set and there
 * is no service available, then any send operation on the socket shall block until
 * at least one _service_ becomes available. The REQ socket shall not discard messages.
 *
 * ZMQ_REP
 * ^^^^^^^
 * A socket of type 'ZMQ_REP' is used by a _service_ to receive requests from and
 * send replies to a _client_. This socket type allows only an alternating
 * sequence of _Czmq_recv(request)_ and subsequent _Czmq_send(reply)_ calls. Each
 * request received is fair-queued from among all _clients_, and each reply sent
 * is routed to the _client_ that issued the last request. If the original
 * requester does not exist any more the reply is silently discarded.
 */
void *CZMQUtils::CzmqGetSocket(void *context, const char *identity, int identityLen,
                              const char *serverName, const char *endpoint, int type)
{
    EndpointSocket *endpointSocket;

    endpointSocket = Czmq_sockets.find(serverName);
    if (endpointSocket != Czmq_sockets.end())
        return endpointSocket->socket;

    if (nullptr == context)
        context = Czmq_getContext();

    void *socket;
    if (ZMQ_REP != type) {
        socket = Czmq_socket(context, ZMQ_REQ);
        if (identityLen > 254)
            identityLen = 254;
        if (identityLen > 0) {
            Czmq_setsockopt(requester, ZMQ_ROUTING_ID, identity, identityLen);
        }
        else {
            static const char *identityDefault = "COMO";
            Czmq_setsockopt(requester, ZMQ_ROUTING_ID, identityDefault, strlen(identityDefault));
        }
    }
    else {
        socket = Czmq_socket(context, ZMQ_REP);
        if (identity != nullptr) {
            if (Czmq_getsockopt(requester, ZMQ_ROUTING_ID, identity, identityLen) != 0) {
                Logger::E("CZMQUtils::CzmqGetSocket", "errno %d", errno);
            }
        }
    }


    if (nullptr != socket) {
        if (nullptr != endpoint) {
            int rc = Czmq_connect(socket, endpoint);
            if (rc != 0)
                Logger::E("CZMQUtils::CzmqGetSocket", "endpoint: %s errno %d", endpoint, errno);
        }
        endpointSocket = new EndpointSocket();
        endpointSocket->endpoint = std::string(endpoint);
        endpointSocket->socket = socket;
        Czmq_sockets.emplace(serverName, endpointSocket);
    }

    /*
    int rc = Czmq_connect(socket, endpoint);
    int rc = Czmq_disconnect(socket, endpoint);
    int rc = Czmq_close(socket);
    int rc = Czmq_ctx_destroy(context);
    */
    return socket;
}

/**
 * serverName should be unique name
 */
int CZMQUtils::CzmqCloseSocket(const char *serverName)
{
    EndpointSocket *endpointSocket;

    endpointSocket = Czmq_sockets.find(serverName);
    if (endpointSocket != Czmq_sockets.end()) {
        int rc;

        rc = Czmq_disconnect(endpointSocket->socket, endpointSocket->endpoint);
        if (Czmq_disconnect != 0) {
            rc = Czmq_close(endpointSocket->socket);
        }
        if (rc != 0) {
            Logger::E("CZMQUtils::CzmqCloseSocket", "errno %d", errno);
            return rc;
        }

        delete(endpointSocket);
        int delNum = unordered_map.erase(serverName);
        if (delNum != 1) {
            Logger::E("CZMQUtils::CzmqCloseSocket", "serverName should be unique name");
        }
        return delNum;
    }

    return 0;
}

/**
 * Send an Invoke request through ZeroMQ and wait for the server to return the
 * calculation result
 */
ECode CZMQUtils::CzmqSendWithReplyAndBlock(Integer eventCode, void *socket, const void *buf, size_t len)
{
    Long crc64 = crc_64_ecma(reinterpret_cast<const unsigned char *>(buf), len);

    int numberOfBytes;
    COMO_ZMQ_RPC_MSG_HEAD funCodeAndCRC64;
    funCodeAndCRC64.funCode = methodCode;
    funCodeAndCRC64.crc64 = crc64;
    funCodeAndCRC64.msgSize = len;

    numberOfBytes = Czmq_send(socket, &funCodeAndCRC64, sizeof(funCodeAndCRC64), ZMQ_SNDMORE);
    if (numberOfBytes != -1) {
        numberOfBytes = Czmq_send(socket, buf, len, 0);
    }
    else {
        Logger::E("CZMQUtils::CzmqSendWithReplyAndBlock", "errno %d", errno);
        return -1;
    }

    if (numberOfBytes != -1) {
        //int Czmq_recv (void *s_, void *buf_, size_t len_, int flags_);
    }
    else {
        Logger::E("CZMQUtils::CzmqSendWithReplyAndBlock", "errno %d", errno);
        return -1;
    }

    return NOERROR;
}

} // namespace como
