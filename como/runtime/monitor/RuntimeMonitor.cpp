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

#include "reflHelpers.h"
#include "RuntimeMonitor.h"

namespace como {

RuntimeMonitor::RuntimeMonitor() {
    //
}

ECode RuntimeMonitor::RuntimeMonitorMsgProcessor(zmq_msg_t& msg, String& string)
{
    return NOERROR;
}

RuntimeMonitor_Log_::RuntimeMonitor_Log_(const char *functionName)
{
    String strClassInfo, strInterfaceInfo, methodSignature;

    reflHelpers::intfGetObjectInfo((IInterface*)this, functionName, strClassInfo,
                                              strInterfaceInfo, methodSignature);
    mFunctionName = functionName;
    mStrClassInfo = strClassInfo;
    mStrInterfaceInfo = strInterfaceInfo;
    mMethodSignature = methodSignature;
}

RuntimeMonitor_Log_::~RuntimeMonitor_Log_()
{

}

} // namespace como
