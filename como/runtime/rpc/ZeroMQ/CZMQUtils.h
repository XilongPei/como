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

#include "zmq.h"
#include "comotypes.h"
#include "comoref.h"
#include "util/arraylist.h"
#include "util/mutex.h"

namespace como {

class ZmqFunCode {
public:
    enum {
        GetComponentMetadata = 0x0,
        Method_Invoke        = 0x0101,
        Actor_IsPeerAlive    = 0x0102,
        Object_Release       = 0x0103,
        AddService           = 0x0201,
        GetService           = 0x0202,
        RemoveService        = 0x0203,
        AnswerECode          = 0x1111
    };
};

constexpr ECode E_ZMQ_FUN_CODE_ERROR = MAKE_COMORT_ECODE(0, 0xE0);
constexpr ECode ZMQ_BAD_REPLY_DATA = MAKE_COMORT_ECODE(0, 0xE1);

#pragma pack(1)
typedef struct tagCOMO_ZMQ_RPC_MSG_HEAD {
    HANDLE  hChannel;
    Integer eCode;      // eventCode, ECode
    Long crc64;
    Integer msgSize;
} COMO_ZMQ_RPC_MSG_HEAD;
#pragma pack()

class CZMQUtils {
public:
    static void *CzmqGetContext();

    static void *CzmqGetSocket(void *context, const char *identity, size_t identityLen,
                              const char *serverName, const char *endpoint, int type);

    static Integer CzmqRecvBuf(HANDLE& hChannel, Integer& eventCode, void *socket, void *buf,
                                                                size_t bufSize, Boolean wait);

    static Integer CzmqRecvMsg(HANDLE& hChannel, Integer& eventCode, void *socket, zmq_msg_t& msg, Boolean wait);

    static int CzmqCloseSocket(const char *serverName);

    static Integer CzmqSendBuf(HANDLE hChannel, Integer eventCode, void *socket, const void *buf, size_t bufSize);

    static void *CzmqFindSocket(const char *serverName);

    static void *CzmqSocketMonitor(const char *serverName);
};

} // namespace como

#endif // __CZMQUTILS_H__
