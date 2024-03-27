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
    MiString::shrink(bufEndpoint, 4096, endpoint);

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
        char **p = heap_StrToX->vHashmap(bufEndpoint);
        if (nullptr != p) {
            return ((EndpointSocket*)*p)->socket;
        }
#endif
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
            char **p = heap_StrToX->vHashmap(bufEndpoint);
            if (nullptr != p) {
                *p = (char*)endpointSocket;
            }
#endif
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

#ifndef COMO_FUNCTION_SAFETY_RTOS
    std::unordered_map<std::string, EndpointSocket*>::iterator iter =
                                        zmq_sockets.find(std::string(serverName));
    if (iter != zmq_sockets.end()) {
        endpointSocket = iter->second;
#else
    char **p = heap_StrToX->vHashmap((char*)serverName);
    if (nullptr != p) {
        endpointSocket = (EndpointSocket*)*p;
#endif

        int rc;
        rc = zmq_disconnect(endpointSocket->socket, endpointSocket->endpoint.c_str());
        if (0 != rc) {
            rc = zmq_close(endpointSocket->socket);
            Logger::E("CZMQUtils::CzmqCloseSocket", "errno %d, %s",
                                        zmq_errno(), zmq_strerror(zmq_errno()));
            return rc;
        }

        delete(endpointSocket);
#ifndef COMO_FUNCTION_SAFETY_RTOS
        int delNum = zmq_sockets.erase(std::string(serverName));
#else
        int delNum;
        *p = nullptr;
        delNum = 1;
#endif
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
    if (bufSize == 0) {
        return -1;
    }

    Long crc64 = crc_64_ecma(reinterpret_cast<const unsigned char *>(buf), bufSize);

    int numberOfBytes;
    COMO_ZMQ_RPC_MSG_HEAD funCodeAndCRC64;
    funCodeAndCRC64.eCode = eventCode;
    funCodeAndCRC64.crc64 = 0;
    funCodeAndCRC64.msgSize = bufSize;
    funCodeAndCRC64.hChannel = hChannel;

    funCodeAndCRC64.crc64 = update_crc_64_ecma_bytes(crc64,
                            reinterpret_cast<const unsigned char *>(&funCodeAndCRC64),
                            sizeof(COMO_ZMQ_RPC_MSG_HEAD));

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
                          void *socket, void *buf, size_t bufSize, int flags)
{
    int numberOfBytes;
    COMO_ZMQ_RPC_MSG_HEAD funCodeAndCRC64;

    // Block until a message is available to be received from socket if flags != ZMQ_DONTWAIT
    numberOfBytes = zmq_recv(socket, &funCodeAndCRC64, sizeof(funCodeAndCRC64), flags);
    if (numberOfBytes != -1) {
        if (EAGAIN == errno) {
            return 0;
        }
        Logger::E("CZMQUtils::CzmqRecvBuf", "error: %d %s",
                                        zmq_errno(), zmq_strerror(zmq_errno()));
        eventCode = MAKE_COMORT_ECODE(0x1, zmq_errno());
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
                eventCode = MAKE_COMORT_ECODE(0x1, zmq_errno());
                return -1;
            }
            if (numberOfBytes >= bufSize) {
                Logger::E("CZMQUtils::CzmqRecvBuf", "Message is bigger than the buffer");
                eventCode = MAKE_COMORT_ECODE(0x1, zmq_errno());
                return -1;
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
        eventCode = MAKE_COMORT_ECODE(0x1, zmq_errno());
        return -1;
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
            if (0 != rc) {
                Logger::E("CZMQUtils::CzmqRecvMsg", "zmq_msg_init error, %d %s",
                                        zmq_errno(), zmq_strerror(zmq_errno()));
                return -1;
            }

            // Block until a message is available to be received from socket
            numberOfBytes = zmq_msg_recv(&msg, socket, 0);
            if (-1 == numberOfBytes) {
                Logger::E("CZMQUtils::CzmqRecvMsg", "zmq_msg_recv error, %d %s",
                                        zmq_errno(), zmq_strerror(zmq_errno()));
                // msg has already been zmq_msg_init
                return -2;
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
                return -3;
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
    char **p = heap_StrToX->vHashmap((char*)endpoint.c_str());
    if (nullptr != p) {
        return (EndpointSocket*)*p;
    }
#endif

    return nullptr;
}

static int get_monitor_event(void *socket, uint32_t& value, char *msg_data, size_t& msg_data_size);
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
    int rc = zmq_socket_monitor(endpointSocket->socket, endpointSocket->endpoint.c_str(), ZMQ_EVENT_ALL);
    if (0 != rc) {
        return nullptr;
    }

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
                    CZMQUtils::CzmqCloseSocket(endpoint);
                }
        }

        rc = zmq_disconnect(socketZMQ_PAIR, endpoint);
        if (0 != rc) {
            Logger::V("ZMQ monitor", "cann't disconnect %s\n", endpoint);
            continue;
        }
    }
    zmq_close(socketZMQ_PAIR);
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

int CZMQUtils::CzmqGetSockets(void *context, const char *endpoint)
{
    zmq_pollitemNum = ComoConfig::ThreadPoolZmqActor_MAX_THREAD_NUM;
    zmq_pollitems = (zmq_pollitem_t*)malloc(sizeof(zmq_pollitem_t) * zmq_pollitemNum);
    if (nullptr == zmq_pollitems) {
        return -1;
    }

    if (nullptr == context) {
        context = CzmqGetContext();
    }

    for (int i = 0;  i < zmq_pollitemNum;  i++) {
        // Socket to talk to dispatcher
        void *socket = zmq_socket(context, ZMQ_REP);
        if (nullptr == socket) {
            return -1;
        }

        if (nullptr == endpoint) {
            zmq_connect(socket, "inproc://workers");
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
 *      tcpEndpoint: "tcp://127.0.0.1:5555"
 *      inprocEndpoint: "inproc://workers"
 */
int CZMQUtils::CzmqProxy(void *context, const char *tcpEndpoint,
                                                     const char *inprocEndpoint)
{
    if (nullptr == context) {
        context = CzmqGetContext();
    }

    // Socket to talk to clients
    void *clients = zmq_socket(context, ZMQ_ROUTER);
    if (nullptr != tcpEndpoint) {
        zmq_bind(clients, tcpEndpoint);
    }
    else {
        zmq_bind(clients, "tcp://127.0.0.1:4800");
    }

    // Socket to talk to workers
    void *workers = zmq_socket(context, ZMQ_DEALER);

    if (nullptr != inprocEndpoint) {
        zmq_bind(workers, inprocEndpoint);
    }
    else {
        zmq_bind(workers, "inproc://workers");
    }


    // Connect Worker threads to client threads via a queue proxy
    zmq_proxy(clients, workers, nullptr);

    // We never get here, but clean up anyhow
    zmq_close(clients);
    zmq_close(workers);
    zmq_ctx_destroy(context);

    return 0;
}

} // namespace como
