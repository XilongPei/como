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

#include "ini.h"
#include "reflHelpers.h"
#include "RuntimeMonitor.h"
#include "registry.h"

namespace como {

RuntimeMonitor::RuntimeMonitor() {
    //
}

static int handler(void* user, const char* section, const char* name, const char* value)
{
    if (/*strcmp(section, "") == 0 && */strcmp(name, "ExportObject") == 0) {
        (*(int*)user) = 1;
    }
    else if (/*strcmp(section, "") == 0 && */strcmp(name, "ImportObject") == 0) {
        (*(int*)user) = 2;
    }
    else {
        (*(int*)user) = atoi(value);
    }
    return 1;
}

ECode RuntimeMonitor::RuntimeMonitorMsgProcessor(zmq_msg_t& msg, String& string)
{
    int command;

    // parse monitor commands
    if (ini_parse_string((const char*)zmq_msg_data(&msg), handler, &command) < 0) {
        command = 0;
    }

    string = "";
    switch (command) {
        case 1: {
            WalkExportObject(RPCType::Remote, string);
            break;
        }
        case 2: {
            WalkImportObject(RPCType::Remote, string);
            break;
        }
    }
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
