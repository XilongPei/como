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

#include <malloc.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <ini.h>
#include <comoapi.h>
#include "comolog.h"
#include "reflHelpers.h"
#include "registry.h"
#include "CircleBuffer.h"
#include "comorpc.h"
#include "RuntimeMonitor.h"
#include "MarshalUtils.h"
#include "CpuCoreUtils.h"

namespace como {

CircleBuffer<char> *logCircleBuf = nullptr;
CircleBuffer<char> *loggerOutputCircleBuf = nullptr;
std::deque<RTM_InvokeMethod*> RuntimeMonitor::rtmInvokeMethodServerQueue;
std::deque<RTM_InvokeMethod*> RuntimeMonitor::rtmInvokeMethodClientQueue;
Mutex RuntimeMonitor::rtmInvokeMethodServerQueue_Lock;
Mutex RuntimeMonitor::rtmInvokeMethodClientQueue_Lock;

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

static RTM_CpuMemoryStatus GetRtmCpuMemoryStatus()
{
    RTM_CpuMemoryStatus status;
    status.cpuUsagePercent = CpuCoreUtils::GetProcCpuUsagePercent(getpid());

    struct mallinfo mi;
    mi = mallinfo();
    status.totalAllocdSpace = mi.uordblks;
    status.totalFreeSpace = mi.fordblks;

    return status;
}

static int handler(void* user, const char* section, const char* name, const char* value)
{
    if (/*strcmp(section, "") == 0 && */strcmp(name, "ExportObject") == 0) {
        WalkExportObject(RPCType::Remote, *(String*)user);
    }
    else if (/*strcmp(section, "") == 0 && */strcmp(name, "ImportObject") == 0) {
        WalkImportObject(RPCType::Remote, *(String*)user);
    }
    else if (/*strcmp(section, "") == 0 && */strcmp(name, "RtmLog") == 0) {
        String string;
        logCircleBuf->ReadString(string);
        *(String*)user += string;
    }
    else if (/*strcmp(section, "") == 0 && */strcmp(name, "ComoLogger") == 0) {
        String string;
        loggerOutputCircleBuf->ReadString(string);
        *(String*)user += string;
    }

    return 1;
}

/**
 * RTM_CommandType::COMMAND_BY_STRING:
 *      RtmLog:
 *          RuntimeMonitor_Log(), RuntimeMonitor_Log_()
 *      ComoLogger:
 *          Logger::E(), Logger::D(), Logger::V()
 *      ImportObject:
 *          registry.sLocalImportRegistry, registry.sRemoteImportRegistry
 * RTM_CommandType::COMMAND_InvokeMethod
 *      RuntimeMonitor::WriteRtmInvokeMethod()
 */
#ifdef RPC_OVER_ZeroMQ_SUPPORT
ECode RuntimeMonitor::RuntimeMonitorMsgProcessor(zmq_msg_t& msg, Array<Byte>& resBuffer)
{
    String string;
    RTM_Command *rtmCommand = (RTM_Command *)zmq_msg_data(&msg);
    switch (rtmCommand->command) {
        case (Short)RTM_CommandType::CMD_BY_STRING: {
            String string;

            // parse monitor commands
            if (ini_parse_string((const char*)rtmCommand->parcel, handler, &string) < 0) {
                return E_ILLEGAL_ARGUMENT_EXCEPTION;
            }

            Integer len = string.GetByteLength() + 1;
            resBuffer = Array<Byte>(len);
            if (! resBuffer.IsNull()) {
                // resBuffer.Copy works very slowly
                memcpy(resBuffer.GetPayload(), (const char*)string, len);
            }

            break;
        }
        case (Short)RTM_CommandType::CMD_Server_InvokeMethod: {
            Mutex::AutoLock lock(RuntimeMonitor::rtmInvokeMethodServerQueue_Lock);

            if (! rtmInvokeMethodServerQueue.empty()) {
                RTM_InvokeMethod *rtm_InvokeMethod = rtmInvokeMethodServerQueue.front();

                if (nullptr != rtm_InvokeMethod) {
                    resBuffer = Array<Byte>(rtm_InvokeMethod->length);
                    if (! resBuffer.IsNull()) {
                        // resBuffer.Copy works very slowly
                        memcpy(resBuffer.GetPayload(), rtm_InvokeMethod,
                                                      rtm_InvokeMethod->length);
                        rtmInvokeMethodServerQueue.pop_front();
                    }
                }
            }
            break;
        }
        case (Short)RTM_CommandType::CMD_Server_CpuMemoryStatus: {
            RTM_CpuMemoryStatus status = GetRtmCpuMemoryStatus();
            resBuffer = Array<Byte>(sizeof(RTM_CpuMemoryStatus));
            if (! resBuffer.IsNull()) {
                // resBuffer.Copy works very slowly
                memcpy(resBuffer.GetPayload(), &status,
                                                   sizeof(RTM_CpuMemoryStatus));
            }

            break;
        }
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
    Logger::Monitor(buffer, sizeof(buffer), "RM_log_");
    String string = "{\"tag\":\"";
    string += buffer;
    string += "\",\"coclass\":\""   + strClassInfo +
              "\",\"interface\":\"" + strInterfaceInfo +
              "\",\"method\":\""    + functionName +
              "\",\"signature\":\"" + methodSignature  +
              "\"}\n";
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

static ECode SerializeComponentID(
    /* [in] */ const ComponentID* cid,
    /* [out] */ Array<Byte>& arrCid)
{
    const char* mUri_ = cid->mUri;
    if (mUri_== nullptr) {
        mUri_ = "";
    }

    Array<Byte> arrCid_(sizeof(ComponentID) + strlen(mUri_) + 1);
    Byte *pByte = arrCid_.GetPayload();
    if (nullptr == pByte)
        return E_OUT_OF_MEMORY_ERROR;

    memcpy(pByte, cid, sizeof(ComponentID));
    strcpy((char*)(pByte + sizeof(ComponentID)), mUri_);

    arrCid = arrCid_;

    return NOERROR;
}

/**
 * in_out: 0, in; 1, out
 * whichQueue: 0, rtmInvokeMethodClientQueue; 1, rtmInvokeMethodServerQueue
 */
ECode RuntimeMonitor::WriteRtmInvokeMethod(Long uuid64,
                                      Long serverObjectId, CoclassID& clsId,
                                      InterfaceID iid, Integer in_out,
                                      Integer methodIndexPlus4, IParcel *parcel,
                                      Integer whichQueue)
{
    Array<Byte> arrCid;
    SerializeComponentID(clsId.mCid, arrCid);

    Long sizeParcel;
    parcel->GetDataSize(sizeParcel);

    Long len = sizeof(RTM_InvokeMethod) + sizeParcel + arrCid.GetLength();
    RTM_InvokeMethod *rtm_InvokeMethod = (RTM_InvokeMethod*)malloc(len);
    if (nullptr == rtm_InvokeMethod)
        return E_OUT_OF_MEMORY_ERROR;

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    // The time from January 1, 1970. Unit: microseconds
    rtm_InvokeMethod->time = ts.tv_sec * 1000000 + ts.tv_nsec / 1000;

    rtm_InvokeMethod->length = len;
    rtm_InvokeMethod->uuid64 = uuid64;
    rtm_InvokeMethod->serverObjectId = serverObjectId;
    rtm_InvokeMethod->coclassID = clsId;
    rtm_InvokeMethod->interfaceID_mUuid = iid.mUuid;
    rtm_InvokeMethod->in_out = in_out;
    rtm_InvokeMethod->methodIndexPlus4 = methodIndexPlus4;

    HANDLE parcelData;
    Byte *pByte = (Byte*)rtm_InvokeMethod + sizeof(RTM_InvokeMethod);
    parcel->GetData(parcelData);
    memcpy(pByte, (Byte*)parcelData, sizeParcel);

    pByte += sizeParcel;
    memcpy(pByte, (Byte*)arrCid.GetPayload(), arrCid.GetLength());
    rtm_InvokeMethod->coclassID.mCid = (ComponentID*)(sizeof(RTM_InvokeMethod) +
                                                                    sizeParcel);
    if (0 == whichQueue) {
        Mutex::AutoLock lock(RuntimeMonitor::rtmInvokeMethodClientQueue_Lock);

        if (rtmInvokeMethodClientQueue.size() >= RTM_INVOKEMETHOD_QUEUE_SIZE) {
            RTM_InvokeMethod *rtmMethod = rtmInvokeMethodClientQueue.front();
            free(rtmMethod);
            rtmInvokeMethodClientQueue.pop_front();
        }
        rtmInvokeMethodClientQueue.push_back(rtm_InvokeMethod);
    }
    else {
        Mutex::AutoLock lock(RuntimeMonitor::rtmInvokeMethodServerQueue_Lock);

        if (rtmInvokeMethodServerQueue.size() >= RTM_INVOKEMETHOD_QUEUE_SIZE) {
            RTM_InvokeMethod *rtmMethod = rtmInvokeMethodServerQueue.front();
            free(rtmMethod);
            rtmInvokeMethodServerQueue.pop_front();
        }
        rtmInvokeMethodServerQueue.push_back(rtm_InvokeMethod);
    }

    return NOERROR;
}

RTM_InvokeMethod* RuntimeMonitor::DeserializeRtmInvokeMethod(
                                             RTM_InvokeMethod *rtm_InvokeMethod)
{
    if ((HANDLE)rtm_InvokeMethod->coclassID.mCid < (HANDLE)rtm_InvokeMethod->length) {
        HANDLE hp = (HANDLE)rtm_InvokeMethod + (HANDLE)rtm_InvokeMethod->coclassID.mCid;
        rtm_InvokeMethod->coclassID.mCid = (ComponentID*)hp;
        *(HANDLE*)&(rtm_InvokeMethod->coclassID.mCid->mUri) = hp + sizeof(ComponentID);
    }
    return rtm_InvokeMethod;
}

ECode RuntimeMonitor::DumpRtmInvokeMethod(RTM_InvokeMethod *rtm_InvokeMethod,
                                                               String& strBuffer)
{
    char buf[64];

    AutoPtr<IMetaCoclass> klass;
    ECode ec = CoGetCoclassMetadata(rtm_InvokeMethod->coclassID, nullptr, klass);
    if (FAILED(ec))
        return ec;

    strBuffer = "{\"tag\":\"RM_IM_\",";

    snprintf(buf, sizeof(buf), "\"id\":%lld,", rtm_InvokeMethod->uuid64);
    strBuffer += buf;

    struct timeval curTime;
    curTime.tv_sec = rtm_InvokeMethod->time / 1000000;
    curTime.tv_usec = rtm_InvokeMethod->time % 1000000;
    int milli = curTime.tv_usec / 1000;

    // 2021-11-30 12:34:56
    char ymdhms[20];
    // The +hhmm or -hhmm numeric timezone
    char timezone[8];
    struct tm nowTime;
    localtime_r(&curTime.tv_sec, &nowTime);
    strftime(ymdhms, sizeof(ymdhms), "%Y-%m-%d %H:%M:%S", &nowTime);
    strftime(timezone, sizeof(timezone), "%z", &nowTime);
    snprintf(buf, sizeof(buf), "\"time\":\"%s.%03d %s\",", ymdhms, milli, timezone);
    strBuffer += buf;

    snprintf(buf, sizeof(buf), "\"direction\":%d,", rtm_InvokeMethod->in_out);
    strBuffer += buf;

    InterfaceID iid;
    AutoPtr<IMetaInterface> intf;
    AutoPtr<IMetaMethod> method;
    String name, ns;

    klass->GetName(name);
    klass->GetNamespace(ns);
    strBuffer += "\"coclass\":\"" + ns + "::" + name + "\",";

    memcpy(&iid, &(rtm_InvokeMethod->interfaceID_mUuid), sizeof(UUID));
    klass->GetInterface(iid, intf);
    klass->GetName(name);
    klass->GetNamespace(ns);
    strBuffer += "\"interface\":\"" + ns + "::" + name + "\",";

    intf->GetMethod(rtm_InvokeMethod->methodIndexPlus4, method);
    String strMethod;
    String methodSignature;
    method->GetName(strMethod);
    method->GetSignature(methodSignature);
    strBuffer += "\"method\":\"" + strMethod + "{" + methodSignature + "}\",";

    String strArgBuffer;
    AutoPtr<IParcel> parcel;
    CoCreateParcel(RPCType::Remote, parcel);

    Long len = rtm_InvokeMethod->length -
                          (sizeof(RTM_InvokeMethod) + sizeof(ComponentID) + 1) -
                          strlen(rtm_InvokeMethod->coclassID.mCid->mUri);

    parcel->SetData((HANDLE)rtm_InvokeMethod->parcel, len);

    if (rtm_InvokeMethod->in_out == 0)
        MarshalUtils::UnMarshalArguments(method, parcel, strArgBuffer);
    else
        MarshalUtils::UnUnmarshalResults(method, parcel, strArgBuffer);

    strBuffer += "\"parcel\":" + strArgBuffer;

    strBuffer += "}\n";

    return NOERROR;
}

RTM_Command* RuntimeMonitor::GenRtmCommand(RTM_CommandType command, Short param,
                                             const Byte *buffer, int bufferSize)
{
    Integer len = sizeof(RTM_Command) + bufferSize;
    RTM_Command *rtmCommand = (RTM_Command*)malloc(len);
    if (nullptr != rtmCommand) {
        rtmCommand->length = len;
        rtmCommand->command = (Short)command;
        rtmCommand->param = param;
        memcpy(rtmCommand->parcel, buffer, bufferSize);
    }
    return rtmCommand;
}

RTM_Command* RuntimeMonitor::GenRtmCommand(RTM_CommandType command, Short param,
                                                               const char *cstr)
{
    Integer len = strlen(cstr) + 1;
    RTM_Command *rtmCommand = (RTM_Command*)malloc(sizeof(RTM_Command) + len);
    if (nullptr != rtmCommand) {
        rtmCommand->length = sizeof(RTM_Command) + len;
        rtmCommand->command = (Short)command;
        rtmCommand->param = param;
        memcpy(rtmCommand->parcel, cstr, len);
    }
    return rtmCommand;
}

} // namespace como
