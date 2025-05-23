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
#include "util/mutex.h"
#include "mistring.h"
#include "checksum.h"
#include "ComoConfig.h"
#include "CZMQUtils.h"
#include "StrToX_Hashmap.h"
#include <stdio.h>

namespace como {

#ifdef COMO_FUNCTION_SAFETY_RTOS
    static char heap_StrToX_Hashmap[ComoConfig::ZMQ_SOCKETS_HEAPSIZE];
    static StrToX_Hashmap *heap_StrToX = new StrToX_Hashmap(heap_StrToX_Hashmap,
                                                    sizeof(heap_StrToX_Hashmap));
#else
    static std::unordered_map<std::string, EndpointSocket*> zmq_sockets;
#endif

static zmq_pollitem_t *zmq_pollitems = nullptr;
static int zmq_pollitemNum = 0;
static Mutex zmq_pollitemLock;

static void *comoZmqContext = nullptr;
int CZMQUtils::ZMQ_RCV_TIMEOUT = 1000 * 60 * 60 * 24;   // 1 days

#define TRY_zmq_msg_recv_MOST_TIMES     36500           // 100 years

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

void *CZMQUtils::CzmqGetPollitemsSocket(int idSocket)
{
    if ((idSocket >= 0) && (idSocket < zmq_pollitemNum)) {
        return zmq_pollitems[idSocket].socket;
    }
    return nullptr;
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
void *CZMQUtils::CzmqGetSocket(void *context, const char *endpoint, int type)
{
    EndpointSocket *endpointSocket;
    char bufEndpoint[4096];
    MiString::shrink(bufEndpoint, sizeof(bufEndpoint), endpoint);

    Logger::D("CZMQUtils::CzmqGetSocket",
              "endpoint: %s, ZMQ_REP(4)/ZMQ_REQ(3): %d", bufEndpoint, type);
    {
        Mutex::AutoLock lock(ComoConfig::CZMQUtils_ContextLock);

#ifndef COMO_FUNCTION_SAFETY_RTOS
        std::unordered_map<std::string, EndpointSocket*>::iterator iter =
                                     zmq_sockets.find(std::string(bufEndpoint));
        if (iter != zmq_sockets.end()) {
            endpointSocket = iter->second;
            return endpointSocket->socket;
        }
#else
        char **p = heap_StrToX->vHashmap(bufEndpoint, true);
        if (nullptr != p) {
            return ((EndpointSocket*)*p)->socket;
        }
#endif
    }

    if (nullptr == context) {
        context = CzmqGetContext();
    }

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
            if (0 != rc) {
                Logger::E("CZMQUtils::CzmqGetSocket",
                          "zmq_connect error, endpoint: \"%s\", errno %d, %s", bufEndpoint,
                          zmq_errno(), zmq_strerror(zmq_errno()));
                return nullptr;
            }
        }
        else {                   // accept incoming connections on a socket
            rc = zmq_bind(socket, bufEndpoint);
            if (0 != rc) {
                Logger::E("CZMQUtils::CzmqGetSocket",
                          "zmq_bind error, endpoint: \"%s\", errno %d, %s", bufEndpoint,
                          zmq_errno(), zmq_strerror(zmq_errno()));
                return nullptr;
            }
        }

        endpointSocket = new EndpointSocket();
        if (nullptr == endpointSocket) {
            Logger::E("CZMQUtils::CzmqGetSocket", "new EndpointSocket error");
            return nullptr;
        }

        endpointSocket->socket = socket;
        endpointSocket->endpoint = std::string(bufEndpoint);
        {
            Mutex::AutoLock lock(ComoConfig::CZMQUtils_ContextLock);

#ifndef COMO_FUNCTION_SAFETY_RTOS
            zmq_sockets.emplace(bufEndpoint, endpointSocket);
#else
            char **p = heap_StrToX->vHashmap(bufEndpoint, false);
            if (nullptr != p) {
                *p = (char*)endpointSocket;
            }
            else {
                Logger::E("CZMQUtils::CzmqGetSocket", "heap_StrToX, Out of memory");
                return nullptr;
            }
#endif
        }
    }

    return socket;
}

/**
 * serverName should be unique name
 */
int CZMQUtils::CzmqCloseSocketByName(const char *serverName)
{
    EndpointSocket *endpointSocket;

    Mutex::AutoLock lock(ComoConfig::CZMQUtils_ContextLock);

#ifndef COMO_FUNCTION_SAFETY_RTOS
    std::unordered_map<std::string, EndpointSocket*>::iterator iter =
                                        zmq_sockets.find(std::string(serverName));
    if (iter != zmq_sockets.end()) {
        endpointSocket = iter->second;
#else
    char **p = heap_StrToX->vHashmap((char*)serverName, true);
    if (nullptr != p) {
        endpointSocket = (EndpointSocket*)*p;
#endif

        int rc;
        rc = zmq_disconnect(endpointSocket->socket, endpointSocket->endpoint.c_str());
        if (0 != rc) {
            Logger::E("CZMQUtils::CzmqCloseSocketByName", "errno %d, %s",
                                        zmq_errno(), zmq_strerror(zmq_errno()));
            (void)zmq_close(endpointSocket->socket);
            return rc;
        }

        (void)zmq_close(endpointSocket->socket);
        delete(endpointSocket);
#ifndef COMO_FUNCTION_SAFETY_RTOS
        int delNum = zmq_sockets.erase(std::string(serverName));
#else
        int delNum;
        *p = nullptr;
        delNum = 1;
#endif
        if (1 != delNum) {
            Logger::E("CZMQUtils::CzmqCloseSocketByName",
                                            "serverName should be unique name");
        }
        return delNum;
    }

    return 0;
}

/**
 * CzmqCloseSocket by socket
 */
int CZMQUtils::CzmqCloseSocket(void *zmqSocket)
{
    return zmq_close(zmqSocket);
}

/**
 * Send an buffer through ZeroMQ
 */
Integer CZMQUtils::CzmqSendBuf(HANDLE hChannel, Integer eventCode, void *socket,
                                                const void *buf, size_t bufSize)
{
    if (0 == bufSize) {
        return -1;
    }

    Long crc64 = crc_64_ecma(reinterpret_cast<const unsigned char *>(buf), bufSize);

    int numberOfBytes;
    COMO_ZMQ_RPC_MSG_HEAD funCodeAndCRC64;
    funCodeAndCRC64.eCode = eventCode;
    // [Comment:1] To calculate the crc64 value of structure funCodeAndCRC64,
    //             first set its member variable to 0.
    funCodeAndCRC64.crc64 = 0;
    funCodeAndCRC64.msgSize = bufSize;
    funCodeAndCRC64.hChannel = hChannel;

    // It is correct to assign this member variable twice, see [Comment:1].
    funCodeAndCRC64.crc64 = update_crc_64_ecma_bytes(crc64,
                            reinterpret_cast<const unsigned char *>(&funCodeAndCRC64),
                            sizeof(COMO_ZMQ_RPC_MSG_HEAD));

    numberOfBytes = zmq_send(socket, &funCodeAndCRC64, sizeof(funCodeAndCRC64),
                                                                   ZMQ_SNDMORE);
    if (numberOfBytes != -1) {
        numberOfBytes = zmq_send(socket, buf, bufSize, 0);
    }
    else {
        Logger::E("CZMQUtils::CzmqSendBuf", "zmq_send funCodeAndCRC64, errno %d, %s",
                                        zmq_errno(), zmq_strerror(zmq_errno()));
        return (-2 - zmq_errno());
    }

    if (numberOfBytes == -1) {
        Logger::E("CZMQUtils::CzmqSendBuf", "zmq_send buffer, errno %d, %s",
                                        zmq_errno(), zmq_strerror(zmq_errno()));
        return (-2 - zmq_errno());
    }

    return numberOfBytes;
}

/**
 * Receive message into buffer
 */
Integer CZMQUtils::CzmqRecvBuf(HANDLE& hChannel, Integer& eventCode,
                          void *socket, void *buf, size_t bufSize, int flags)
{
    if (0 == bufSize) {
        return -1;
    }

    int numberOfBytes;
    COMO_ZMQ_RPC_MSG_HEAD funCodeAndCRC64;

    // Block until a message is available to be received from socket if flags != ZMQ_DONTWAIT
    numberOfBytes = zmq_recv(socket, &funCodeAndCRC64, sizeof(funCodeAndCRC64), flags);
    if (-1 == numberOfBytes) {
        if (EAGAIN == errno) {
            int try_zmq_msg_recv = 0;
            do {
                numberOfBytes = zmq_recv(socket, &funCodeAndCRC64,
                                                sizeof(funCodeAndCRC64), flags);
                if (-1 == numberOfBytes) {
                    if (EAGAIN == errno) {
                        try_zmq_msg_recv++;
                    }
                    else {
                        break;
                    }
                }
            } while (try_zmq_msg_recv < TRY_zmq_msg_recv_MOST_TIMES);
        }

        if (-1 == numberOfBytes) {
            Logger::E("CZMQUtils::CzmqRecvBuf", "zmq_recv funCodeAndCRC64, errno %d, %s",
                                            zmq_errno(), zmq_strerror(zmq_errno()));
            eventCode = MAKE_COMORT_ECODE(0x1, zmq_errno());
            return (-2 - zmq_errno());
        }
    }
    else {
        int more;
        size_t more_size = sizeof(more);
        int rc = zmq_getsockopt(socket, ZMQ_RCVMORE, &more, &more_size);
        if (more) {
            numberOfBytes = zmq_recv(socket, buf, bufSize, 0);
            if (-1 == numberOfBytes) {
                if (EAGAIN == errno) {
                    int try_zmq_msg_recv = 0;
                    do {
                        numberOfBytes = zmq_recv(socket, buf, bufSize, 0);
                        if (-1 == numberOfBytes) {
                            if (EAGAIN == errno) {
                                try_zmq_msg_recv++;
                            }
                            else {
                                break;
                            }
                        }
                    } while (try_zmq_msg_recv < TRY_zmq_msg_recv_MOST_TIMES);
                }

                if (-1 == numberOfBytes) {
                    Logger::E("CZMQUtils::CzmqRecvBuf", "zmq_recv buf, errno %d, %s",
                                            zmq_errno(), zmq_strerror(zmq_errno()));
                    eventCode = MAKE_COMORT_ECODE(0x1, zmq_errno());
                    return -1;
                }
            }
            if (numberOfBytes >= bufSize) {
                Logger::E("CZMQUtils::CzmqRecvBuf", "Message is bigger than the buffer");
                eventCode = MAKE_COMORT_ECODE(0x1, zmq_errno());
                return (-2 - zmq_errno());
            }

            Long crc64 = crc_64_ecma(reinterpret_cast<const unsigned char *>(buf), numberOfBytes);

            Long crcTmp = funCodeAndCRC64.crc64;
            funCodeAndCRC64.crc64 = 0;
            crc64 = update_crc_64_ecma_bytes(crc64,
                                    reinterpret_cast<const unsigned char *>(&funCodeAndCRC64),
                                    sizeof(COMO_ZMQ_RPC_MSG_HEAD));

            if ((crcTmp != crc64) || (funCodeAndCRC64.msgSize != numberOfBytes)) {
                Logger::E("CZMQUtils::CzmqRecvBuf", "bad packet");
                eventCode = MAKE_COMORT_ECODE(0x1, zmq_errno());
                return (-2 - zmq_errno());
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
        eventCode = MAKE_COMORT_ECODE(0x1, zmq_errno());
        return (-2 - zmq_errno());
    }

    return numberOfBytes;
}

/**
 * Receive message into zmq_msg_t, it should be rleased with zmq_msg_close(&msg)
 */
Integer CZMQUtils::CzmqRecvMsg(HANDLE& hChannel, Integer& eventCode,
                                     void *socket, zmq_msg_t& msg, int flags)
{
    int numberOfBytes;
    COMO_ZMQ_RPC_MSG_HEAD funCodeAndCRC64;

    // Block until a message is available to be received from socket if flags != ZMQ_DONTWAIT
    numberOfBytes = zmq_recv(socket, &funCodeAndCRC64, sizeof(funCodeAndCRC64), flags);
    if (-1 == numberOfBytes) {
        if (EAGAIN == errno) {
            int try_zmq_msg_recv = 0;
            do {
                numberOfBytes = zmq_recv(socket, &funCodeAndCRC64,
                                                sizeof(funCodeAndCRC64), flags);
                if (-1 == numberOfBytes) {
                    if (EAGAIN == errno) {
                        try_zmq_msg_recv++;
                    }
                    else {
                        break;
                    }
                }
            } while (try_zmq_msg_recv < TRY_zmq_msg_recv_MOST_TIMES);
        }

        if (-1 == numberOfBytes) {
            Logger::E("CZMQUtils::CzmqRecvMsg", "zmq_recv error, %d, %s",
                                            zmq_errno(), zmq_strerror(zmq_errno()));
            eventCode = MAKE_COMORT_ECODE(0x1, zmq_errno());
            return (-2 - zmq_errno());
        }
    }
    else {
        int more;
        size_t more_size = sizeof(more);
        int rc = zmq_getsockopt(socket, ZMQ_RCVMORE, &more, &more_size);
        if (more) {
            int rc = zmq_msg_init(&msg);
            if (0 != rc) {
                Logger::E("CZMQUtils::CzmqRecvMsg", "zmq_msg_init error, errno %d, %s",
                                        zmq_errno(), zmq_strerror(zmq_errno()));
                eventCode = MAKE_COMORT_ECODE(0x1, zmq_errno());
                return (-2 - zmq_errno());
            }

            // Block until a message is available to be received from socket
            numberOfBytes = zmq_msg_recv(&msg, socket, 0);
            if (-1 == numberOfBytes) {
                if (EAGAIN == errno) {
                    int try_zmq_msg_recv = 0;
                    do {
                        numberOfBytes = zmq_msg_recv(&msg, socket, 0);
                        if (-1 == numberOfBytes) {
                            if (EAGAIN == errno) {
                                try_zmq_msg_recv++;
                            }
                            else {
                                break;
                            }
                        }
                    } while (try_zmq_msg_recv < TRY_zmq_msg_recv_MOST_TIMES);
                }

                if (-1 == numberOfBytes) {
                    Logger::E("CZMQUtils::CzmqRecvMsg", "zmq_msg_recv error, errno %d, %s",
                                            zmq_errno(), zmq_strerror(zmq_errno()));
                    // msg has already been zmq_msg_init
                    eventCode = MAKE_COMORT_ECODE(0x1, zmq_errno());
                    return (-2 - zmq_errno());
                }
            }

            Long crc64 = crc_64_ecma(reinterpret_cast<const unsigned char *>(
                                            zmq_msg_data(&msg)), numberOfBytes);

            Long crcTmp = funCodeAndCRC64.crc64;
            funCodeAndCRC64.crc64 = 0;
            crc64 = update_crc_64_ecma_bytes(crc64,
                        reinterpret_cast<const unsigned char *>(&funCodeAndCRC64),
                        sizeof(COMO_ZMQ_RPC_MSG_HEAD));

            if ((crcTmp != crc64) || (funCodeAndCRC64.msgSize != numberOfBytes)) {
                Logger::E("CZMQUtils::CzmqRecvBuf", "bad packet");
                // msg has already been zmq_msg_init
                eventCode = MAKE_COMORT_ECODE(0x1, zmq_errno());
                return (-2 - zmq_errno());
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


EndpointSocket *CZMQUtils::CzmqFindSocket(std::string& endpoint)
{
    Mutex::AutoLock lock(ComoConfig::CZMQUtils_ContextLock);

#ifndef COMO_FUNCTION_SAFETY_RTOS
    std::unordered_map<std::string, EndpointSocket*>::iterator iter =
                                                     zmq_sockets.find(endpoint);
    if (iter != zmq_sockets.end()) {
        return iter->second;
    }
#else
    char **p = heap_StrToX->vHashmap((char*)endpoint.c_str(), true);
    if (nullptr != p) {
        return (EndpointSocket*)*p;
    }
#endif

    return nullptr;
}

static int get_monitor_event(void *socket, uint32_t& value, char *msg_data,
                                                         size_t& msg_data_size);
static void *rep_socket_monitor(void *endpointSocket);

void *CZMQUtils::CzmqSocketMonitor(const char *serverName)
{
#ifndef COMO_FUNCTION_SAFETY_RTOS
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
    int rc = zmq_socket_monitor(endpointSocket->socket,
                                endpointSocket->endpoint.c_str(), ZMQ_EVENT_ALL);
    if (0 != rc) {
        return nullptr;
    }

    Logger::D("CZMQUtils::CzmqSocketMonitor", "pthread_create");
    rc = pthread_create(&thread, NULL, rep_socket_monitor, endpointSocket);
    if (0 != rc) {
        return nullptr;
    }
#endif
    return nullptr;
}

/**
 * Read one event off the monitor socket; return value and address
 * by reference, if not null, and event number by value. Returns -1
 * in case of error.
 */
static int get_monitor_event(void *socket, uint32_t& value, char *msg_data,
                                                          size_t& msg_data_size)
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
#ifndef COMO_FUNCTION_SAFETY_RTOS
    if (nullptr == socketZMQ_PAIR) {
        socketZMQ_PAIR = zmq_socket(CZMQUtils::CzmqGetContext(), ZMQ_PAIR);
    }
    if (nullptr == socketZMQ_PAIR) {
        return nullptr;
    }

    uint32_t value;
    static char msg_data[1025];
    size_t msg_data_size = sizeof(msg_data);

    std::unordered_map<std::string, EndpointSocket*>::iterator iter = zmq_sockets.begin();
    while (! shutdown) {
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
                EndpointSocket *epSocket = (EndpointSocket*)(iter->second);
                endpoint = epSocket->endpoint.c_str();
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
                    CZMQUtils::CzmqCloseSocketByName(endpoint);
                }
        }

        rc = zmq_disconnect(socketZMQ_PAIR, endpoint);
        if (0 != rc) {
            Logger::V("ZMQ monitor", "cann't disconnect %s\n", endpoint);
            continue;
        }
    }
    (void)zmq_close(socketZMQ_PAIR);
#endif
    return nullptr;
}

void *CZMQUtils::CzmqPoll()
{
    {
        Mutex::AutoLock lock(zmq_pollitemLock);
        if (zmq_pollitemNum <= 0) {
            CzmqGetSockets(nullptr, nullptr);
        }
    }

    /**
     * If none of the requested events have occurred on any zmq_pollitem_t item,
     * zmq_poll() shall wait timeout microseconds for an event to occur on any
     * of the requested items. If the value of timeout is 0, zmq_poll() shall
     * return immediately. If the value of timeout is -1, zmq_poll() shall block
     * indefinitely until a requested event has occurred on at least one
     * zmq_pollitem_t. The resolution of timeout is 1 millisecond.
     *
     * The events and revents members of zmq_pollitem_t are bit masks constructed
     * by OR'ing a combination of the following event flags: ZMQ_POLLIN,
     * ZMQ_POLLOUT, ZMQ_POLLERR.
     */
    zmq_poll(zmq_pollitems, zmq_pollitemNum, -1);

    {
        Mutex::AutoLock lock(zmq_pollitemLock);

        for (int i = 0;  i < zmq_pollitemNum;  i++) {
            if (zmq_pollitems[i].revents & ZMQ_POLLIN) {
                zmq_pollitems[i].revents = 0;
                return zmq_pollitems[i].socket;
            }
        }
    }

    return nullptr;
}

/**
 * Binding (zmq_bind or zmq_connect) multiple ports to one endpoint is
 * incorrect, so the functionality of this function needs to be redesigned.
 */
int CZMQUtils::CzmqGetSockets(void *context, const char *endpoint)
{
    zmq_pollitemNum = ComoConfig::ThreadPoolZmqActor_MAX_THREAD_NUM +
                                        ComoConfig::ThreadPoolZmqActor_RC_THREAD_NUM;
    zmq_pollitems = (zmq_pollitem_t*)malloc(sizeof(zmq_pollitem_t) * zmq_pollitemNum);
    if (nullptr == zmq_pollitems) {
        return -1;
    }

    if (nullptr == context) {
        context = CzmqGetContext();
    }

    for (int i = 0;  i < zmq_pollitemNum;  i++) {
        // Socket to talk to dispatcher
        void *socket;
        if (i < ComoConfig::ThreadPoolZmqActor_MAX_THREAD_NUM) {
            socket = zmq_socket(context, ZMQ_REP);    // I am server
        }
        else {
            socket = zmq_socket(context, ZMQ_SUB);    // I am subscriber
        }
        if (nullptr == socket) {
            return -1;
        }

        if (nullptr == endpoint) {
            zmq_connect(socket, ComoConfig::localhostInprocEndpoint);
        }
        else {
            zmq_connect(socket, endpoint);
        }

        //
        // ZMQ_POLLIN
        // For ZeroMQ sockets, at least one message may be received from the
        // socket without blocking. For standard sockets this is equivalent
        // to the POLLIN flag of the poll() system call and generally means
        // that at least one byte of data may be read from fd without blocking.
        // ZMQ_POLLOUT
        // For ZeroMQ sockets, at least one message may be sent to the socket
        // without blocking. For standard sockets this is equivalent to the
        // POLLOUT flag of the poll() system call and generally means that at
        // least one byte of data may be written to fd without blocking.
        //
        zmq_pollitems[i].socket = socket;
        zmq_pollitems[i].fd = 0;
        zmq_pollitems[i].revents = 0;
        zmq_pollitems[i].events = ZMQ_POLLIN;
    }

    return 0;
}

/**
 * parameter example:
 *      tcpEndpoint:
 *          "tcp://127.0.0.1:1239"
 *          "tcp://*:1239"
 *      inprocEndpoint: "inproc://workers"(ComoConfig::localhostInprocEndpoint)
 */
int CZMQUtils::CzmqProxy(void *context, const char *tcpEndpoint,
                                                     const char *inprocEndpoint)
{
    int rc;

    if (nullptr == tcpEndpoint) {
        Logger::E("CZMQUtils::CzmqProxy", "tcpEndpoint is nullptr");
        return -1;
    }

    char bufEndpoint[4096];
    MiString::shrink(bufEndpoint, sizeof(bufEndpoint), tcpEndpoint);

    if (nullptr == context) {
        context = CzmqGetContext();
    }

    if (nullptr == context) {
        Logger::E("CZMQUtils::CzmqProxy", "context is nullptr");
        return -1;
    }

    /**
     *   ZMQ_REQ(3) <---> ROUTER (frontend)
     * (tcpEndpoint)        |
     *                  zmq_proxy
     *                      |
     *                   DEALER (backend) <---> ZMQ_REP(4)(inprocEndpoint)
     */

    // Socket to talk to clients
    void *clients = zmq_socket(context, ZMQ_ROUTER);
    rc = zmq_bind(clients, tcpEndpoint);
    if (0 != rc) {
        Logger::E("CZMQUtils::CzmqProxy",
                  "zmq_bind error, endpoint: \"%s\", errno %d, %s", tcpEndpoint,
                  zmq_errno(), zmq_strerror(zmq_errno()));
        return -2;
    }

    /**
     * Socket to talk to workers
     *
     * Shared Queue
     * When the frontend is a ZMQ_ROUTER socket, and the backend is a ZMQ_DEALER
     * socket, the proxy shall act as a shared queue that collects requests from
     * a set of clients, and distributes these fairly among a set of services.
     * Requests shall be fair-queued from frontend connections and distributed
     * evenly across backend connections. Replies shall automatically return to
     * the client that made the original request.
     */
    void *workers = zmq_socket(context, ZMQ_DEALER);

    if (nullptr == inprocEndpoint) {
        inprocEndpoint = ComoConfig::localhostInprocEndpoint;
    }
    rc = zmq_bind(workers, inprocEndpoint);
    if (0 != rc) {
        Logger::E("CZMQUtils::CzmqProxy",
                  "zmq_bind error, endpoint: \"%s\", errno %d, %s", inprocEndpoint,
                  zmq_errno(), zmq_strerror(zmq_errno()));
        return -3;
    }

    /**
     * Connect Worker threads to client threads via a queue proxy
     * The zmq_proxy() function starts the built-in ZMQ proxy in the current
     * application thread.
     * The proxy connects a frontend socket to a backend socket. Conceptually,
     * data flows from frontend to backend. Depending on the socket types,
     * replies may flow in the opposite direction. The direction is conceptual
     * only; the proxy is fully symmetric and there is no technical difference
     * between frontend and backend.
     *
     * Before calling zmq_proxy() you must set any socket options, and connect
     * or bind both frontend and backend sockets. The two conventional proxy
     * models are:
     * zmq_proxy() runs in the current thread and returns only if/when the
     * current context is closed.
     * If the capture socket is not NULL, the proxy shall send all messages,
     * received on both frontend and backend, to the capture socket. The capture
     * socket should be a ZMQ_PUB, ZMQ_DEALER, ZMQ_PUSH, or ZMQ_PAIR socket.
     *
     * The zmq_proxy() function always returns -1 and errno set to ETERM (the 0MQ
     * context associated with either of the specified sockets was terminated).
     */
    (void)zmq_proxy(clients, workers, nullptr);

    // We never get here, but clean up anyhow
    (void)zmq_close(clients);
    (void)zmq_close(workers);
    zmq_ctx_destroy(context);

    return 0;
}

/**
 * CZMQUtils::CzmqGetRepSocket, Obtain an ZMQ_REP-type socket, zmq_connect to
 * an endpoint.
 */
void *CZMQUtils::CzmqGetRepSocket(void *context, const char *endpoint)
{
    char bufEndpoint[4096];

    if (nullptr == endpoint) {
        endpoint = ComoConfig::localhostInprocEndpoint;
    }
    else {
        endpoint = MiString::shrink(bufEndpoint, sizeof(bufEndpoint), endpoint);
    }

    if (nullptr == context) {
        context = CzmqGetContext();
        if (nullptr == context) {
            return nullptr;
        }
    }

    void *worker  = zmq_socket(context, ZMQ_REP);
    if (nullptr != worker ) {
        int rc = zmq_connect(worker, endpoint);
        if (0 != rc) {
            Logger::E("CZMQUtils::CzmqGetRepSocket",
                      "zmq_bind error, endpoint: \"%s\", errno %d, %s", endpoint,
                      zmq_errno(), zmq_strerror(zmq_errno()));
            (void)zmq_close(worker);
            return nullptr;
        }

        return worker;
    }

    return nullptr;
}

/**
 * A socket of type ZMQ_PUB is used by a publisher to distribute data. Messages
 * sent are distributed in a fan out fashion to all connected peers.
 *
 * The zmq_recv function is not implemented for this socket type. This means
 * that it can only send messages, not receive messages.
 *
 * When a ZMQ_PUB socket enters the mute state due to having reached the high
 * water mark for a subscriber, then any messages that would be sent to the
 * subscriber in question shall instead be dropped until the mute state ends.
 * The zmq_send() function shall never block for this socket type.
 */
void *CZMQUtils::CzmqGetPubSocket(void *context, const char *endpoint)
{
    char bufEndpoint[4096];

    if (nullptr == endpoint) {
        return nullptr;
    }

    endpoint = MiString::shrink(bufEndpoint, sizeof(bufEndpoint), endpoint);

    if (nullptr == context) {
        context = CzmqGetContext();
        if (nullptr == context) {
            return nullptr;
        }
    }

    void *publisher = zmq_socket(context, ZMQ_PUB);
    if (nullptr != publisher) {
        int rc = zmq_bind(publisher, endpoint);
        if (0 != rc) {
            Logger::E("CZMQUtils::CzmqGetPubSocket",
                      "zmq_bind error, endpoint: \"%s\", errno %d, %s", endpoint,
                      zmq_errno(), zmq_strerror(zmq_errno()));
            (void)zmq_close(publisher);
            return nullptr;
        }

        //zmq_setsockopt(publisher, ZMQ_SNDHWM, &nMaxNum, sizeof(nMaxNum));
        return publisher;
    }

    return nullptr;
}

/**
 * A socket of type ZMQ_SUB is used by a subscriber to subscribe to data
 * distributed by a publisher. Initially a ZMQ_SUB socket is not subscribed to
 * any messages, use the ZMQ_SUBSCRIBE option of zmq_setsockopt to specify which
 * messages to subscribe to. The zmq_send() function is not implemented for this
 * socket type.
 */
void *CZMQUtils::CzmqGetSubSocket(void *context, const char *endpoint)
{
    char bufEndpoint[4096];

    if (nullptr == endpoint) {
        return nullptr;
    }

    endpoint = MiString::shrink(bufEndpoint, sizeof(bufEndpoint), endpoint);

    if (nullptr == context) {
        context = CzmqGetContext();
        if (nullptr == context) {
            return nullptr;
        }
    }

    void *subscriber = zmq_socket(context, ZMQ_SUB);
    if (nullptr != subscriber) {
        int rc = zmq_connect(subscriber, endpoint);
        if (0 != rc) {
            Logger::E("CZMQUtils::CzmqGetSubSocket",
                      "zmq_connect error, endpoint: \"%s\", errno %d, %s", endpoint,
                      zmq_errno(), zmq_strerror(zmq_errno()));
            (void)zmq_close(subscriber);
            return nullptr;
        }

        return subscriber;
    }

    return nullptr;
}

} // namespace como
