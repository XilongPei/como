//=========================================================================
// Copyright (C) 2024 The C++ Component Model(COMO) Open Source Project
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

#include "CZMQUtils.h"
#include "ComoPhxUtils.h"
#include "PaxosUtils.h"
#include "PhxPaxosC.h"

namespace como {

/**
 * In redundant computing, data is synchronized through PhxSendBuf
 */
Integer PaxosUtils::PhxSendBuf(void *oPhxFscpServer, HANDLE hChannel, Integer eventCode,
                               void *socket, const void *buf, size_t bufSize)
{
    int iRet;

    COMO_ZMQ_RPC_MSG_HEAD funCodeAndCRC64;
    funCodeAndCRC64.eCode = eventCode;
    funCodeAndCRC64.crc64 = 0;
    funCodeAndCRC64.msgSize = bufSize;
    funCodeAndCRC64.hChannel = hChannel;

    std::string sEchoReqValue;
    std::string sEchoRespValue;
    std::string sname = std::string("keyName");
    std::string ss = std::string((char*)&funCodeAndCRC64, sizeof(funCodeAndCRC64)) +
                     std::string((char*)buf, bufSize);
#ifdef FORCE_USE_ALGORITHM_PAXOS
    iRet = como::ComoPhxUtils::SyncStateData((PhxFscpServer *)oPhxFscpServer,
                                    como::ComoPhxUtils::LevelDbWrite, sname, ss,
                                    sEchoRespValue);
#else
    iRet = PhxProposeDirectly((PhxFscpServer *)oPhxFscpServer,
                              como::ComoPhxUtils::LevelDbWrite,
                              (char*)sname.data(), (char*)ss.data(), ss.size());
#endif
    if (0 != iRet) {
        Logger_E("PhxpaxosUtils::PhxSendBuf", "SyncStateData fail, ret %d", iRet);
    }

    return 0;
}

} // namespace como
