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

#include "comotypes.h"
#include "comoref.h"
#include "util/arraylist.h"
#include "util/mutex.h"

namespace como {

enum ZmqFunCode {
    GetComponentMetadata = 0,
    Method_Invoke = 1,
    Actor_IsPeerAlive = 2,
    Object_Release = 3
};

typedef struct tagCOMO_ZMQ_RPC_MSG_HEAD {
    Integer eventCode,
    Long crc64,
    Integer msgSize
} COMO_ZMQ_RPC_MSG_HEAD;

class CZMQUtils {

    void *CzmqGetContext();

    void *CzmqGetSocket(void *context, const char *identity, int identityLen,
                              const char *serverName, const char *endpoint, int type);

    Integer CzmqRecvBuf(Integer& eventCode, void *socket, const void *buf, size_t bufSize, Boolean wait);

    Integer CzmqRecvMsg(Integer& eventCode, void *socket, zmq_msg_t& msg, Boolean wait);

    int CzmqCloseSocket(const char *serverName);

    Integer CzmqSendWithReplyAndBlock(Integer eventCode, void *socket, const void *buf, size_t bufSize);

    static void *CzmqFindSocket(const char *serverName);

private:
    static void *comoZmqContext;
    static Mutex comoZmqContextLock;
};

} // namespace como

#endif // __CZMQUTILS_H__
