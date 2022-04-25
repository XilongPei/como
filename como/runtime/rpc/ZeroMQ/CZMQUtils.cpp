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
#include <unistd.h>
#include "mistring.h"
#include "checksum.h"
#include "ComoConfig.h"
#include "CZMQUtils.h"
#include <stdio.h>

namespace como {

class EndpointSocket {
public:
    ~EndpointSocket() {
        int rc = zmq_close(socket);
    }

public:
    void *socket;
    std::string serverName;
    std::string endpoint;
};

static std::unordered_map<std::string, EndpointSocket*> zmq_sockets;
static zmq_pollitem_t *zmq_pollitems = nullptr;
static int zmq_pollitemNum = 0;
static void *comoZmqContext = nullptr;
int CZMQUtils::ZMQ_RCV_TIMEOUT = 1000 * 60 * 3;

/*
int rc = zmq_ctx_shutdown(context);
*/
void *CZMQUtils::CzmqGetContext() {

    Mutex::AutoLock lock(ComoConfig::CZMQUtils_ContextLock);

    if (nullptr == comoZmqContext) {
        comoZmqContext = zmq_ctx_new();

        /* custom context
        zmq_ctx_set(comoZmqContext, ZMQ_IO_THREADS, 1);
        */
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
void *CZMQUtils::CzmqGetSocket(void *context, const char *serverName,
                                                 const char *endpoint, int type)
{
    EndpointSocket *endpointSocket;
    char bufEndpoint[4096];
    MiString::shrink(bufEndpoint, 4096, endpoint);

    Logger::D("CZMQUtils::CzmqGetSocket",
              "serverName: %s,  endpoint: %s, ZMQ_REP(4)/ZMQ_REQ(3): %d",
              serverName, bufEndpoint, type);
    {
        Mutex::AutoLock lock(ComoConfig::CZMQUtils_ContextLock);

        std::unordered_map<std::string, EndpointSocket*>::iterator iter =
                                     zmq_sockets.find(std::string(bufEndpoint));
        if (iter != zmq_sockets.end()) {
            endpointSocket = iter->second;
            return endpointSocket->socket;
        }
    }

    if (nullptr == context)
        context = CzmqGetContext();

    void *socket;
    if (ZMQ_REP != type) {      // I am client
        socket = zmq_socket(context, ZMQ_REQ);
    }
    else {                      // I am server
        socket = zmq_socket(context, ZMQ_REP);
    }

    if (nullptr != socket) {

        zmq_setsockopt(socket, ZMQ_RCVTIMEO, &CZMQUtils::ZMQ_RCV_TIMEOUT, sizeof(int));

        int rc;

        if (ZMQ_REP != type) {   // create outgoing connection from socket
            rc = zmq_connect(socket, bufEndpoint);
            if (rc != 0) {
                Logger::E("CZMQUtils::CzmqGetSocket",
                          "zmq_connect error, %s errno %d %s", bufEndpoint,
                          zmq_errno(), zmq_strerror(zmq_errno()));
                return nullptr;
            }
        }
        else {                   // accept incoming connections on a socket
            rc = zmq_bind(socket, bufEndpoint);
            if (rc != 0) {
                Logger::E("CZMQUtils::CzmqGetSocket",
                          "zmq_bind error, %s errno %d %s", bufEndpoint,
                          zmq_errno(), zmq_strerror(zmq_errno()));
                return nullptr;
            }

            int i = zmq_pollitemNum;
            zmq_pollitemNum++;
            zmq_pollitems = (zmq_pollitem_t*)realloc(zmq_pollitems,
                                      sizeof(zmq_pollitem_t) * zmq_pollitemNum);
            zmq_pollitems[i].socket = socket;
            zmq_pollitems[i].fd = 0;
            // For ZeroMQ sockets, at least one message may be received from the
            // socket without blocking. For standard sockets this is equivalent
            // to the POLLIN flag of the poll() system call and generally means
            // that at least one byte of data may be read from fd without blocking.
            zmq_pollitems[i].events = ZMQ_POLLIN;
            zmq_pollitems[i].revents = 0;
        }

        endpointSocket = new EndpointSocket();
        if (nullptr == endpointSocket) {
            Logger::E("CZMQUtils::CzmqGetSocket", "new EndpointSocket error");
            return nullptr;
        }

        endpointSocket->socket = socket;
        endpointSocket->serverName = std::string(serverName);
        endpointSocket->endpoint = std::string(bufEndpoint);
        {
            Mutex::AutoLock lock(ComoConfig::CZMQUtils_ContextLock);

            zmq_sockets.emplace(bufEndpoint, endpointSocket);
        }
    }

    return socket;
}

/**
 * serverName should be unique name
 */
int CZMQUtils::CzmqCloseSocket(const char *serverName)
{
    EndpointSocket *endpointSocket;

    Mutex::AutoLock lock(ComoConfig::CZMQUtils_ContextLock);

    std::unordered_map<std::string, EndpointSocket*>::iterator iter =
                                        zmq_sockets.find(std::string(serverName));
    if (iter != zmq_sockets.end()) {
        endpointSocket = iter->second;

        int rc;
        rc = zmq_disconnect(endpointSocket->socket, endpointSocket->endpoint.c_str());
        if (0 != rc) {
            rc = zmq_close(endpointSocket->socket);
            Logger::E("CZMQUtils::CzmqCloseSocket", "errno %d, %s",
                                        zmq_errno(), zmq_strerror(zmq_errno()));
            return rc;
        }

        delete(endpointSocket);
        int delNum = zmq_sockets.erase(std::string(serverName));
        if (1 != delNum) {
            Logger::E("CZMQUtils::CzmqCloseSocket", "serverName should be unique name");
        }
        return delNum;
    }

    return 0;
}

/**
 * Send an buffer through ZeroMQ
 */
Integer CZMQUtils::CzmqSendBuf(HANDLE hChannel, Integer eventCode, void *socket,
                                                const void *buf, size_t bufSize)
{
    if (bufSize <= 0)
        return -1;

    Long crc64 = crc_64_ecma(reinterpret_cast<const unsigned char *>(buf), bufSize);

    int numberOfBytes;
    COMO_ZMQ_RPC_MSG_HEAD funCodeAndCRC64;
    funCodeAndCRC64.eCode = eventCode;
    funCodeAndCRC64.crc64 = crc64;
    funCodeAndCRC64.msgSize = bufSize;

    HANDLE pid = getpid();
    funCodeAndCRC64.hChannel = ((pid & 0xFFFF) << 48) | (hChannel & 0xFFFFFFFFFFFF);
    //                                    1 0                          5 4 3 2 1 0

    numberOfBytes = zmq_send(socket, &funCodeAndCRC64, sizeof(funCodeAndCRC64),
                                                                   ZMQ_SNDMORE);
    if (numberOfBytes != -1) {
        numberOfBytes = zmq_send(socket, buf, bufSize, 0);
    }
    else {
        Logger::E("CZMQUtils::CzmqSendBuf", "zmq_send funCodeAndCRC64 errno, %d %s",
                                        zmq_errno(), zmq_strerror(zmq_errno()));
        return -1;
    }

    if (numberOfBytes == -1) {
        Logger::E("CZMQUtils::CzmqSendBuf", "zmq_send buffer errno, %d %s",
                                        zmq_errno(), zmq_strerror(zmq_errno()));
        return -1;
    }

    return numberOfBytes;
}

/**
 * Receive message into buffer
 */
Integer CZMQUtils::CzmqRecvBuf(HANDLE& hChannel, Integer& eventCode,
                          void *socket, void *buf, size_t bufSize, Boolean wait)
{
    int numberOfBytes;
    COMO_ZMQ_RPC_MSG_HEAD funCodeAndCRC64;

    // Block until a message is available to be received from socket if wait != ZMQ_DONTWAIT
    numberOfBytes = zmq_recv(socket, &funCodeAndCRC64, sizeof(funCodeAndCRC64), wait);
    if (numberOfBytes != -1) {
        if (EAGAIN == errno) {
            return 0;
        }
        Logger::E("CZMQUtils::CzmqRecvBuf", "error: %d %s",
                                        zmq_errno(), zmq_strerror(zmq_errno()));
        return -1;
    }
    else {
        int more;
        size_t more_size = sizeof (more);
        int rc = zmq_getsockopt(socket, ZMQ_RCVMORE, &more, &more_size);
        if (more) {
            numberOfBytes = zmq_recv(socket, buf, bufSize, 0);
            if (-1 == numberOfBytes) {
                Logger::E("CZMQUtils::CzmqRecvBuf", "errno: %d %s",
                                        zmq_errno(), zmq_strerror(zmq_errno()));
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
            hChannel = funCodeAndCRC64.hChannel;
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
Integer CZMQUtils::CzmqRecvMsg(HANDLE& hChannel, Integer& eventCode,
                                     void *socket, zmq_msg_t& msg, Boolean wait)
{
    int numberOfBytes;
    COMO_ZMQ_RPC_MSG_HEAD funCodeAndCRC64;

    // Block until a message is available to be received from socket if wait != ZMQ_DONTWAIT
    numberOfBytes = zmq_recv(socket, &funCodeAndCRC64, sizeof(funCodeAndCRC64), wait);
    if (numberOfBytes == -1) {
        if (EAGAIN == errno) {
            return 0;
        }
        Logger::E("CZMQUtils::CzmqRecvMsg", "zmq_recv error, %d %s",
                                        zmq_errno(), zmq_strerror(zmq_errno()));
        return -1;
    }
    else {
        int more;
        size_t more_size = sizeof(more);
        int rc = zmq_getsockopt(socket, ZMQ_RCVMORE, &more, &more_size);
        if (more) {
            int rc = zmq_msg_init(&msg);

            // Block until a message is available to be received from socket
            numberOfBytes = zmq_msg_recv(&msg, socket, 0);
            if (-1 == numberOfBytes) {
                Logger::E("CZMQUtils::CzmqRecvMsg", "zmq_msg_recv error, %d %s",
                                        zmq_errno(), zmq_strerror(zmq_errno()));
                return -1;
            }

            Long crc64 = crc_64_ecma(reinterpret_cast<const unsigned char *>(
                                            zmq_msg_data(&msg)), numberOfBytes);
            if ((funCodeAndCRC64.crc64 != crc64) ||
                                   (funCodeAndCRC64.msgSize != numberOfBytes)) {
                Logger::E("CZMQUtils::CzmqRecvBuf", "bad packet");
                return -1;
            }
            eventCode = funCodeAndCRC64.eCode;
            hChannel = funCodeAndCRC64.hChannel;
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

    Mutex::AutoLock lock(ComoConfig::CZMQUtils_ContextLock);

    std::unordered_map<std::string, EndpointSocket*>::iterator iter =
                                        zmq_sockets.find(std::string(serverName));
    if (iter != zmq_sockets.end()) {
        endpointSocket = iter->second;
        return endpointSocket->socket;
    }

    return nullptr;
}

static int get_monitor_event(void *socket, uint32_t& value, char *msg_data, size_t& msg_data_size);
static void *rep_socket_monitor(void *endpointSocket);

void *CZMQUtils::CzmqSocketMonitor(const char *serverName)
{
    EndpointSocket *endpointSocket;

    {
        Mutex::AutoLock lock(ComoConfig::CZMQUtils_ContextLock);

        std::unordered_map<std::string, EndpointSocket*>::iterator iter =
                                            zmq_sockets.find(std::string(serverName));
        if (iter != zmq_sockets.end()) {
            endpointSocket = iter->second;
        }
    }

    pthread_t thread ;
    // REP socket monitor, all events
    int rc = zmq_socket_monitor(endpointSocket->socket, endpointSocket->endpoint.c_str(), ZMQ_EVENT_ALL);
    if (0 != rc)
        return nullptr;

    rc = pthread_create(&thread, NULL, rep_socket_monitor, endpointSocket);
    if (0 != rc)
        return nullptr;

    return nullptr;
}

/**
 * Read one event off the monitor socket; return value and address
 * by reference, if not null, and event number by value. Returns -1
 * in case of error.
 */
static int get_monitor_event(void *socket, uint32_t& value, char *msg_data, size_t& msg_data_size)
{
    // First frame in message contains event number and value
    zmq_msg_t msg;
    zmq_msg_init(&msg);
    if (zmq_msg_recv(&msg, socket, ZMQ_DONTWAIT) == -1) {
        return -1;              // Interrupted, presumably
    }

    if (! zmq_msg_more(&msg)) {
        Logger::E("get_monitor_event", "bad packet 1.");
        return -1;
    }

    uint8_t *data = (uint8_t *)zmq_msg_data(&msg);
    uint16_t event = *(uint16_t *)(data);
    value = *(uint32_t *)(data + 2);

    // Second frame in message contains event address
    zmq_msg_init(&msg);

    if (zmq_msg_recv(&msg, socket, ZMQ_DONTWAIT) == -1) {
        return -1;              //  Interrupted, presumably
    }

    if (zmq_msg_more(&msg)) {
        Logger::E("get_monitor_event", "bad packet 2.");
        return -1;
    }

    if (msg_data) {
        uint8_t *data = (uint8_t *)zmq_msg_data(&msg);
        size_t size = zmq_msg_size(&msg);

        if (msg_data_size > (size + 1)) {
            memcpy(msg_data, data, size);
            msg_data[size] = '\0';
        }
        else {
            memcpy(msg_data, data, msg_data_size-1);
            msg_data[msg_data_size-1] = '\0';
        }
        msg_data_size = size;
    }

    zmq_msg_close(&msg);

    return event;
}

static void *socketZMQ_PAIR = nullptr;
static bool shutdown = false;

// REP socket monitor thread
static void *rep_socket_monitor(void *endpointSocket)
{
    if (nullptr == socketZMQ_PAIR) {
        socketZMQ_PAIR = zmq_socket(CZMQUtils::CzmqGetContext(), ZMQ_PAIR);
    }
    if (nullptr == socketZMQ_PAIR)
        return nullptr;

    uint32_t value;
    static char msg_data[1025];
    size_t msg_data_size = sizeof(msg_data);

    std::unordered_map<std::string, EndpointSocket*>::iterator iter = zmq_sockets.begin();
    while (!shutdown) {
        int event;
        int rc;
        const char *endpoint;

        if (nullptr != endpointSocket) {
            endpoint = ((EndpointSocket*)endpointSocket)->endpoint.c_str();
        }
        else {
            if (iter != zmq_sockets.end()) {
                EndpointSocket *epSocket = (EndpointSocket*)(iter->second);
                endpoint = epSocket->endpoint.c_str();
                iter++;
            }
            else {
                iter = zmq_sockets.begin();
            }

        }

        rc = zmq_connect(socketZMQ_PAIR, endpoint);
        if (0 != rc) {
            Logger::V("ZMQ monitor", "cann't connect %s\n", endpoint);
            continue;
        }

        event = get_monitor_event(socketZMQ_PAIR, value, msg_data, msg_data_size);
        switch (event) {
            case ZMQ_EVENT_LISTENING:
                Logger::V("ZMQ monitor", "listening socket descriptor %d\n", value);
                Logger::V("ZMQ monitor", "listening socket address %s\n", msg_data);
                break;
            case ZMQ_EVENT_ACCEPTED:
                Logger::V("ZMQ monitor", "accepted socket descriptor %d\n", value);
                Logger::V("ZMQ monitor", "accepted socket address %s\n", msg_data);
                break;
            case ZMQ_EVENT_CLOSE_FAILED:
                Logger::V("ZMQ monitor", "socket close failure error code %d\n", value);
                Logger::V("ZMQ monitor", "socket address %s\n", msg_data);
                break;
            case ZMQ_EVENT_CLOSED:
                Logger::V("ZMQ monitor", "closed socket descriptor %d\n", value);
                Logger::V("ZMQ monitor", "closed socket address %s\n", msg_data);
                break;
            case ZMQ_EVENT_DISCONNECTED:
                Logger::V("ZMQ monitor", "disconnected socket descriptor %d\n", value);
                Logger::V("ZMQ monitor", "disconnected socket address %s\n", msg_data);
                break;
            default:
                if (zmq_errno() != EAGAIN) {
                    // this socket is invalid, close it
                    CZMQUtils::CzmqCloseSocket(((EndpointSocket*)endpointSocket)->endpoint.c_str());
                }
        }

        rc = zmq_disconnect(socketZMQ_PAIR, endpoint);
        if (0 != rc) {
            Logger::V("ZMQ monitor", "cann't disconnect %s\n", endpoint);
            continue;
        }
    }
    zmq_close(socketZMQ_PAIR);
    return nullptr;
}

void CZMQUtils::CzmqPoll(pthread_cond_t& pthreadCond, bool& signal)
{
    while (true) {
        zmq_poll(zmq_pollitems, zmq_pollitemNum, -1);
        signal = true;
        pthread_cond_signal(&pthreadCond);
        sleep(5);
    }
}

} // namespace como
