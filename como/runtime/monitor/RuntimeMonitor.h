//=========================================================================
// Copyright (C) 2022 The C++ Component Model(COMO) Open Source Project
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

#ifndef __COMO_RUNTIMEMONITOR_H__
#define __COMO_RUNTIMEMONITOR_H__

#include "zmq.h"
#include "comodef.h"
#include "comotypes.h"

namespace como {

class COM_PUBLIC RuntimeMonitor
{
public:
    RuntimeMonitor();

    static ECode RuntimeMonitorMsgProcessor(zmq_msg_t& msg, String& string);

};

#define RuntimeMonitor_Log()  \
           RuntimeMonitor_Log_ __runtimeMonitor_Log_((const char*)__FUNCTION__);

class COM_PUBLIC RuntimeMonitor_Log_
{
public:
    // define at the first line of a Function
    RuntimeMonitor_Log_(const char *functionName);

    ~RuntimeMonitor_Log_();
private:
    const char *mFunctionName;
    String mStrClassInfo;
    String mStrInterfaceInfo;
    String mMethodSignature;
};


} // namespace como

using namespace como;

#endif // __COMO_RUNTIMEMONITOR_H__
