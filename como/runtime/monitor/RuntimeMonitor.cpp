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
#include "queue"
#include "ini.h"
#include "comolog.h"
#include "reflHelpers.h"
#include "registry.h"
#include "CircleBuffer.h"
#include "RuntimeMonitor.h"

namespace como {

CircleBuffer<char> *logCircleBuf = nullptr;
CircleBuffer<char> *loggerOutputCircleBuf = nullptr;
std::deque<RTM_InvokeMethod*> rtmInvokeMethodQueue(RuntimeMonitor::RTM_INVOKEMETHOD_QUEUE_SIZE);

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

    loggerOutputCircleBuf = new CircleBuffer<char>(RM_LOG_BUFFER_SIZE);
    if (nullptr != loggerOutputCircleBuf) {
        ECode ec = ECode_CircleBuffer(loggerOutputCircleBuf);
        if (FAILED(ec)) {
            return ec;
        }
    }
    else
        return E_OUT_OF_MEMORY_ERROR;

    // Collect the logs generated during COMO running for monitoring by RuntimeMonitor
    Logger::SetLoggerWriteLog(RuntimeMonitor::WriteLog);

    return NOERROR;
}

static int handler(void* user, const char* section, const char* name, const char* value)
{
    String string;
    if (/*strcmp(section, "") == 0 && */strcmp(name, "ExportObject") == 0) {
        WalkExportObject(RPCType::Remote, string);
        ((Array<Byte>*)user)->Copy((Byte*)(const char*)string, string.GetByteLength());
    }
    else if (/*strcmp(section, "") == 0 && */strcmp(name, "ImportObject") == 0) {
        WalkImportObject(RPCType::Remote, string);
        ((Array<Byte>*)user)->Copy((Byte*)(const char*)string, string.GetByteLength());
    }
    else if (/*strcmp(section, "") == 0 && */strcmp(name, "RTM_log") == 0) {
        logCircleBuf->ReadString(string);
        ((Array<Byte>*)user)->Copy((Byte*)(const char*)string, string.GetByteLength());
    }
    else if (/*strcmp(section, "") == 0 && */strcmp(name, "COMO_Logger") == 0) {
        loggerOutputCircleBuf->ReadString(string);
        ((Array<Byte>*)user)->Copy((Byte*)(const char*)string, string.GetByteLength());
    }
    else if (/*strcmp(section, "") == 0 && */strcmp(name, "InvokeMethod") == 0) {
        if (! rtmInvokeMethodQueue.empty()) {
            RTM_InvokeMethod *rtmMethod = rtmInvokeMethodQueue.front();
            rtmInvokeMethodQueue.pop_front();
        }
    }
    return 1;
}

#ifdef RPC_OVER_ZeroMQ_SUPPORT
ECode RuntimeMonitor::RuntimeMonitorMsgProcessor(zmq_msg_t& msg, Array<Byte>& resBuffer)
{
    String string;

    // parse monitor commands
    if (ini_parse_string((const char*)zmq_msg_data(&msg), handler, &string) < 0) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    return NOERROR;
}
#endif

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

int RuntimeMonitor::WriteLog(const char *log, size_t strLen)
{
    return loggerOutputCircleBuf->Write(log, strLen+1);
}

ECode RuntimeMonitor::WriteRtmInvokeMethod(Long serverObjectId, CoclassID& cid,
                                      InterfaceID iid, Integer in_out,
                                      Integer methodIndexPlus4, IParcel *parcel)
{
    Long sizeParcel;
    parcel->GetDataSize(sizeParcel);

    Long len = sizeof(RTM_InvokeMethod) + sizeParcel;
    RTM_InvokeMethod *rtm_InvokeMethod = (RTM_InvokeMethod*)malloc(len);
    if (nullptr == rtm_InvokeMethod)
        return E_OUT_OF_MEMORY_ERROR;

    clock_gettime(CLOCK_REALTIME, &(rtm_InvokeMethod->time));
    rtm_InvokeMethod->length = len;
    rtm_InvokeMethod->serverObjectId = serverObjectId;
    rtm_InvokeMethod->coclassID_mUuid = cid.mUuid;
    rtm_InvokeMethod->interfaceID_mUuid = iid.mUuid;
    rtm_InvokeMethod->in_out = in_out;
    rtm_InvokeMethod->methodIndexPlus4 = methodIndexPlus4;

    HANDLE parcelData;
    Byte *p = (Byte*)rtm_InvokeMethod + sizeof(RTM_InvokeMethod);
    parcel->GetData(parcelData);
    memcpy(p, (Byte*)parcelData, sizeParcel);

    if (rtmInvokeMethodQueue.size() >= RTM_INVOKEMETHOD_QUEUE_SIZE) {
        RTM_InvokeMethod *rtmMethod = rtmInvokeMethodQueue.front();
        free(rtmMethod);
        rtmInvokeMethodQueue.pop_front();
    }
    rtmInvokeMethodQueue.push_back(rtm_InvokeMethod);

    return NOERROR;
}

} // namespace como
