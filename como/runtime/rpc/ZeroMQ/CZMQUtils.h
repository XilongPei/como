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

#ifndef __CZMQUTILS_H__
#define __CZMQUTILS_H__

#include <string>
#include <pthread.h>
#include "zmq.h"
#include "comotypes.h"
#include "comoref.h"

namespace como {

class ZmqFunCode {
public:
    enum  : std::uint16_t {
        GetComponentMetadata = 0x0,
        Method_Invoke        = 0x0101U,
        Actor_IsPeerAlive    = 0x0102U,
        Object_Release       = 0x0103U,
        ReleasePeer          = 0x0104U,
        AddService           = 0x0201U,
        GetService           = 0x0202U,
        RemoveService        = 0x0203U,
        RuntimeMonitor       = 0x0301U,
        AnswerECode          = 0x1111U
    };
};

/**
 * These ECode ecErrors are registered in ThreadPoolChannelInvoke.cpp
 */
constexpr ECode E_ZMQ_FUN_CODE_ERROR = MAKE_COMORT_ECODE(0, 0xE0U);
constexpr ECode ZMQ_BAD_REPLY_DATA = MAKE_COMORT_ECODE(0, 0xE1U);
constexpr ECode ZMQ_BAD_PACKET = MAKE_COMORT_ECODE(0, 0xE2U);

#pragma pack(1)
typedef struct tagCOMO_ZMQ_RPC_MSG_HEAD {
    HANDLE  hChannel;
    Integer eCode;      // eventCode, ECode
    Long crc64;
    Integer msgSize;
} COMO_ZMQ_RPC_MSG_HEAD;
#pragma pack()

class EndpointSocket {
public:
    ~EndpointSocket() {
        int rc = zmq_close(socket);
    }

public:
    void *socket;
    std::string endpoint;
};

class COM_PUBLIC CZMQUtils {
public:
    static void *CzmqGetContext();

    static void *CzmqGetPollitemsSocket(int idSocket);

    static void *CzmqGetSocket(void *context, const char *endpoint, int type);

    static Integer CzmqRecvBuf(HANDLE& hChannel, Integer& eventCode,
                         void *socket, void *buf, size_t bufSize, int flags);

    static Integer CzmqRecvMsg(HANDLE& hChannel, Integer& eventCode,
                                    void *socket, zmq_msg_t& msg, int flags);

    static int CzmqCloseSocketByName(const char *serverName);

    static int CzmqCloseSocket(void *zmqSocket);

    static Integer CzmqSendBuf(HANDLE hChannel, Integer eventCode, void *socket,
                                                const void *buf, size_t bufSize);

    static EndpointSocket *CzmqFindSocket(std::string& endpoint);

    static void *CzmqSocketMonitor(const char *serverName);

    static void *CzmqPoll();

    COM_PUBLIC static int CzmqProxy(void *context, const char *tcpEndpoint,
                                                    const char *inprocEndpoint);

    static int CzmqGetSockets(void *context, const char *endpoint);

    static void *CzmqGetRepSocket(void *context, const char *endpoint);

    static void *CzmqGetPubSocket(void *context, const char *endpoint);

    static void *CzmqGetSubSocket(void *context, const char *endpoint);

    // milliseconds
    static int ZMQ_RCV_TIMEOUT;
};

} // namespace como

#endif // __CZMQUTILS_H__
