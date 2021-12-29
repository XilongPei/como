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
#include "CZMQUtils.h"

namespace como {

class EndpointSocket {
public:
    ~EndpointSocket() {
        int rc = zmq_close(socket);
    }

public:
    std::string endpoint;
    void *socket;
};

static std::unordered_map<std::string, EndpointSocket*> zmq_sockets;

void *comoZmqContext = nullptr;
Mutex comoZmqContextLock;


/*
int rc = zmq_ctx_destroy(context);
*/
void *CZMQUtils::CzmqGetContext() {
    Mutex::AutoLock lock(comoZmqContextLock);
    if (nullptr == comoZmqContext) {
        comoZmqContext = zmq_ctx_new();
    }
    return comoZmqContext;
}

/**
 * ZMQ_REQ
 * ^^^^^^^
 * A socket of type 'ZMQ_REQ' is used by a _client_ to send requests to and
 * receive replies from a _service_. This socket type allows only an alternating
 * sequence of _zmq_send(request)_ and subsequent _zmq_recv(reply)_ calls. Each
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
 * sequence of _zmq_recv(request)_ and subsequent _zmq_send(reply)_ calls. Each
 * request received is fair-queued from among all _clients_, and each reply sent
 * is routed to the _client_ that issued the last request. If the original
 * requester does not exist any more the reply is silently discarded.
 */
void *CZMQUtils::CzmqGetSocket(void *context, const char *identity, size_t identityLen,
                              const char *serverName, const char *endpoint, int type)
{
    EndpointSocket *endpointSocket;

    std::unordered_map<std::string, EndpointSocket*>::iterator tmp =
                                        zmq_sockets.find(std::string(serverName));
    if (tmp != zmq_sockets.end()) {
        endpointSocket = tmp->second;
        return endpointSocket->socket;
    }

    if (nullptr == context)
        context = CzmqGetContext();

    void *socket;
    if (ZMQ_REP != type) {
        socket = zmq_socket(context, ZMQ_REQ);
        if (identityLen > 254)
            identityLen = 254;
        if (identityLen > 0) {
            zmq_setsockopt(socket, ZMQ_ROUTING_ID, identity, identityLen);
        }
        else {
            static const char *identityDefault = "COMO";
            zmq_setsockopt(socket, ZMQ_ROUTING_ID, identityDefault, strlen(identityDefault));
        }
    }
    else {
        socket = zmq_socket(context, ZMQ_REP);
        if (identity != nullptr) {
            if (zmq_getsockopt(context, ZMQ_ROUTING_ID, (char *)identity, &identityLen) != 0) {
                Logger::E("CZMQUtils::CzmqGetSocket", "errno %d", zmq_errno());
            }
        }
    }


    if (nullptr != socket) {
        if (nullptr != endpoint) {
            int rc = zmq_connect(socket, endpoint);
            if (rc != 0) {
                Logger::E("CZMQUtils::CzmqGetSocket", "endpoint: %s errno %d", endpoint, zmq_errno());
            }
        }
        endpointSocket = new EndpointSocket();
        endpointSocket->endpoint = std::string(endpoint);
        endpointSocket->socket = socket;
        zmq_sockets.emplace(serverName, endpointSocket);
    }

    return socket;
}

/**
 * serverName should be unique name
 */
int CZMQUtils::CzmqCloseSocket(const char *serverName)
{
    EndpointSocket *endpointSocket;

    std::unordered_map<std::string, EndpointSocket*>::iterator tmp =
                                        zmq_sockets.find(std::string(serverName));
    if (tmp != zmq_sockets.end()) {
        endpointSocket = tmp->second;

        int rc;
        rc = zmq_disconnect(endpointSocket->socket, endpointSocket->endpoint.c_str());
        if (zmq_disconnect != 0) {
            rc = zmq_close(endpointSocket->socket);
        }
        if (rc != 0) {
            Logger::E("CZMQUtils::CzmqCloseSocket", "errno %d", zmq_errno());
            return rc;
        }

        delete(endpointSocket);
        int delNum = zmq_sockets.erase(std::string(serverName));
        if (delNum != 1) {
            Logger::E("CZMQUtils::CzmqCloseSocket", "serverName should be unique name");
        }
        return delNum;
    }

    return 0;
}

/**
 * Send an buffer through ZeroMQ
 */
Integer CZMQUtils::CzmqSendBuf(Integer eventCode, void *socket, const void *buf, size_t bufSize)
{
    Long crc64 = crc_64_ecma(reinterpret_cast<const unsigned char *>(buf), bufSize);

    int numberOfBytes;
    COMO_ZMQ_RPC_MSG_HEAD funCodeAndCRC64;
    funCodeAndCRC64.eCode = eventCode;
    funCodeAndCRC64.crc64 = crc64;
    funCodeAndCRC64.msgSize = bufSize;

    numberOfBytes = zmq_send(socket, &funCodeAndCRC64, sizeof(funCodeAndCRC64), ZMQ_SNDMORE);
    if (numberOfBytes != -1) {
        numberOfBytes = zmq_send(socket, buf, bufSize, 0);
    }
    else {
        Logger::E("CZMQUtils::CzmqSendBuf", "errno %d", zmq_errno());
        return -1;
    }

    if (numberOfBytes == -1) {
        Logger::E("CZMQUtils::CzmqSendBuf", "errno %d", zmq_errno());
        return -1;
    }

    return numberOfBytes;
}

/**
 * Receive message into buffer
 */
Integer CZMQUtils::CzmqRecvBuf(Integer& eventCode, void *socket, void *buf,
                                                            size_t bufSize, Boolean wait)
{
    int numberOfBytes;
    COMO_ZMQ_RPC_MSG_HEAD funCodeAndCRC64;

    // Block until a message is available to be received from socket if wait != ZMQ_DONTWAIT
    numberOfBytes = zmq_recv(socket, &funCodeAndCRC64, sizeof(funCodeAndCRC64), wait);
    if (numberOfBytes != -1) {
        if (EAGAIN == errno) {
            return 0;
        }
        Logger::E("CZMQUtils::CzmqRecvBuf", "errno %d", zmq_errno());
        return -1;
    }
    else {
        int more;
        size_t more_size = sizeof (more);
        int rc = zmq_getsockopt (socket, ZMQ_RCVMORE, &more, &more_size);
        if (more) {
            numberOfBytes = zmq_recv(socket, buf, bufSize, 0);
            if (-1 == numberOfBytes) {
                Logger::E("CZMQUtils::CzmqRecvBuf", "errno %d", zmq_errno());
                return -1;
            }
            if (numberOfBytes >= bufSize) {
                Logger::E("CZMQUtils::CzmqRecvBuf", "Message is bigger than the buffer");
                return -1;
            }

            Long crc64 = crc_64_ecma(reinterpret_cast<const unsigned char *>(buf), numberOfBytes);
            if ((funCodeAndCRC64.crc64 != crc64) || (funCodeAndCRC64.msgSize != numberOfBytes)) {
                Logger::E("CZMQUtils::CzmqRecvBuf", "bad packet");
                return -1;
            }
            eventCode = funCodeAndCRC64.eCode;
        }
        else {
            numberOfBytes = 0;
        }
    }

    if (numberOfBytes == -1) {
        Logger::E("CZMQUtils::CzmqSendBuf", "errno %d", zmq_errno());
        return -1;
    }

    return numberOfBytes;
}

/**
 * Receive message into zmq_msg_t, it should be rleased with zmq_msg_close(&msg)
 */
Integer CZMQUtils::CzmqRecvMsg(Integer& eventCode, void *socket, zmq_msg_t& msg, Boolean wait)
{
    int numberOfBytes;
    COMO_ZMQ_RPC_MSG_HEAD funCodeAndCRC64;

    // Block until a message is available to be received from socket if wait != ZMQ_DONTWAIT
    numberOfBytes = zmq_recv(socket, &funCodeAndCRC64, sizeof(funCodeAndCRC64), wait);
    if (numberOfBytes == -1) {
        if (EAGAIN == errno) {
            return 0;
        }
        Logger::E("CZMQUtils::CzmqRecvMsg", "errno %d", zmq_errno());
        return -1;
    }
    else {
        int more;
        size_t more_size = sizeof (more);
        int rc = zmq_getsockopt (socket, ZMQ_RCVMORE, &more, &more_size);
        if (more) {
            int rc = zmq_msg_init(&msg);

            // Block until a message is available to be received from socket
            numberOfBytes = zmq_msg_recv(&msg, socket, 0);
             if (-1 == numberOfBytes) {
                Logger::E("CZMQUtils::CzmqRecvMsg", "errno %d", zmq_errno());
                return -1;
            }

            Long crc64 = crc_64_ecma(reinterpret_cast<const unsigned char *>(zmq_msg_data(&msg)),
                                                                                    numberOfBytes);
            if ((funCodeAndCRC64.crc64 != crc64) || (funCodeAndCRC64.msgSize != numberOfBytes)) {
                Logger::E("CZMQUtils::CzmqRecvBuf", "bad packet");
                return -1;
            }
            eventCode = funCodeAndCRC64.eCode;
        }
        else {
            numberOfBytes = 0;
        }
    }

    return numberOfBytes;
}


void *CZMQUtils::CzmqFindSocket(const char *serverName)
{
    EndpointSocket *endpointSocket;

    std::unordered_map<std::string, EndpointSocket*>::iterator tmp =
                                        zmq_sockets.find(std::string(serverName));
    if (tmp != zmq_sockets.end()) {
        endpointSocket = tmp->second;
        return endpointSocket->socket;
    }

    return nullptr;
}

} // namespace como
