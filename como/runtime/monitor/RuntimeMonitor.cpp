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

#include <time.h>
#include "ini.h"
#include "comolog.h"
#include "reflHelpers.h"
#include "registry.h"
#include "CircleBuffer.h"
#include "RuntimeMonitor.h"

namespace como {

CircleBuffer<char> *logCircleBuf;

RuntimeMonitor::RuntimeMonitor() {
    //
}

ECode RuntimeMonitor::StartRuntimeMonitor()
{
    logCircleBuf = new CircleBuffer<char>(RM_LOG_BUFFER_SIZE);
    if (nullptr != logCircleBuf) {
        ECode ec = ECode_CircleBuffer(logCircleBuf);
        if (FAILED(ec)) {
            return ec;
        }
    }
    else
        return E_OUT_OF_MEMORY_ERROR;

    return NOERROR;
}

static int handler(void* user, const char* section, const char* name, const char* value)
{
    if (/*strcmp(section, "") == 0 && */strcmp(name, "ExportObject") == 0) {
        WalkExportObject(RPCType::Remote, *(String*)user);
    }
    else if (/*strcmp(section, "") == 0 && */strcmp(name, "ImportObject") == 0) {
        WalkImportObject(RPCType::Remote, *(String*)user);
    }
    else if (/*strcmp(section, "") == 0 && */strcmp(name, "log") == 0) {
        logCircleBuf->ReadString(*(String*)user);
    }

    return 1;
}

ECode RuntimeMonitor::RuntimeMonitorMsgProcessor(zmq_msg_t& msg, String& string)
{
    // parse monitor commands
    if (ini_parse_string((const char*)zmq_msg_data(&msg), handler, &string) < 0) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    return NOERROR;
}

RuntimeMonitor_Log_::RuntimeMonitor_Log_(const char *functionName)
{
    String strClassInfo, strInterfaceInfo, methodSignature;

    reflHelpers::intfGetObjectInfo((IInterface*)this, functionName, strClassInfo,
                                              strInterfaceInfo, methodSignature);
    char buffer[256];
    Logger::Monitor(buffer, sizeof(buffer), "tag");
    String string = buffer;
    string += "coclass:" + strClassInfo + " interface:" + strInterfaceInfo +
              " method:" + functionName + " signature:" + methodSignature  + "\n";
    logCircleBuf->Write(string);
}

RuntimeMonitor_Log_::~RuntimeMonitor_Log_()
{}

ECode RuntimeMonitor::GetMethodFromRtmInvokeMethod(RTM_InvokeMethod *rtm_InvokeMethod,
                                       IInterface *intf, AutoPtr<IMetaMethod>& method)
{
    InterfaceID iid;
    intf->GetInterfaceID(intf, iid);

    AutoPtr<IMetaCoclass> klass;
    IObject::Probe(intf)->GetCoclass(klass);
    AutoPtr<IMetaInterface> imintf;
    klass->GetInterface(iid, imintf);
    imintf->GetMethod(rtm_InvokeMethod->methodIndexPlus4, method);

    return NOERROR;
}

Byte *RuntimeMonitor::GetRtmInvokeMethodParcel(RTM_InvokeMethod *rtm_InvokeMethod)
{
    return (Byte*)rtm_InvokeMethod + sizeof(RTM_InvokeMethod) - sizeof(Byte*);
}


RTM_InvokeMethod *RuntimeMonitor::WriteRtmInvokeMethod(Long serverObjectId,
                                      Integer methodIndexPlus4, IParcel *parcel)
{
    Long sizeParcel;
    parcel->GetDataSize(sizeParcel);

    Long len = sizeof(RTM_InvokeMethod) - sizeof(Byte*) + sizeParcel;
    RTM_InvokeMethod *rtm_InvokeMethod = (RTM_InvokeMethod*)malloc(len);
    if (nullptr == rtm_InvokeMethod)
        return nullptr;

    clock_gettime(CLOCK_REALTIME, &(rtm_InvokeMethod->time));

    rtm_InvokeMethod->serverObjectId = serverObjectId;
    rtm_InvokeMethod->methodIndexPlus4 = methodIndexPlus4;

    HANDLE parcelData;
    Byte *p = (Byte*)rtm_InvokeMethod + sizeof(RTM_InvokeMethod) - sizeof(Byte*);
    parcel->GetData(parcelData);
    memcpy(p, (Byte*)parcelData, sizeParcel);

    return rtm_InvokeMethod;
}

} // namespace como
