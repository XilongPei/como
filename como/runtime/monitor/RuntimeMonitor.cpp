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

// If the definitions of these two global variables are placed in class
// RuntimeMonitor, it will require references to the header CircleBuffer.h,
// which will make header RuntimeMonitor.h cumbersome
static CircleBuffer<char> *logCircleBuf = nullptr;
static CircleBuffer<char> *loggerOutputCircleBuf = nullptr;

Mutex RuntimeMonitor::rtmInvokeMethodServerQueue_Lock;
Mutex RuntimeMonitor::rtmInvokeMethodClientQueue_Lock;

lwrb_t *RuntimeMonitor::rtmLwRB_ServerQueue;
lwrb_t *RuntimeMonitor::rtmLwRB_ClientQueue;

RuntimeMonitor::RuntimeMonitor()
{
    logCircleBuf = nullptr;
    loggerOutputCircleBuf = nullptr;
}

RuntimeMonitor::~RuntimeMonitor()
{
    if (nullptr == logCircleBuf) {
        return;
    }

    delete logCircleBuf;
    delete loggerOutputCircleBuf;

    logCircleBuf = nullptr;
    loggerOutputCircleBuf = nullptr;

    free(rtmLwRB_ServerQueue->buff);
    free(rtmLwRB_ClientQueue->buff);

    // just set buffer handle to `NULL`
    lwrb_free(rtmLwRB_ServerQueue);
    lwrb_free(rtmLwRB_ClientQueue);
}

/**
 * If the current environment is COMO_FUNCTION_SAFETY_RTOS, applying for dynamic
 * memory in this function does not matter, because it is the job of the system
 * initialization phase.
 */
ECode RuntimeMonitor::StartRuntimeMonitor()
{
    logCircleBuf = new CircleBuffer<char>(RTM_LOG_BUFFER_SIZE);
    if (nullptr != logCircleBuf) {
        ECode ec = ECode_CircleBuffer(logCircleBuf);
        if (FAILED(ec)) {
            return ec;
        }
    }
    else {
        return E_OUT_OF_MEMORY_ERROR;
    }

    loggerOutputCircleBuf = new CircleBuffer<char>(RTM_LOG_BUFFER_SIZE);
    if (nullptr != loggerOutputCircleBuf) {
        ECode ec = ECode_CircleBuffer(loggerOutputCircleBuf);
        if (FAILED(ec)) {
            delete logCircleBuf;
            logCircleBuf = nullptr;

            return ec;
        }
    }
    else {
        delete logCircleBuf;
        logCircleBuf = nullptr;

        return E_OUT_OF_MEMORY_ERROR;
    }

    // Collect the logs generated during COMO running for monitoring by RuntimeMonitor
    Logger::SetLoggerWriteLog(WriteLog);

    void *buff_data;
    buff_data = malloc(RTM_INVOKEMETHOD_QUEUE_SIZE);
    if (nullptr == buff_data) {
        delete logCircleBuf;
        delete loggerOutputCircleBuf;

        logCircleBuf = nullptr;
        loggerOutputCircleBuf = nullptr;

        return E_OUT_OF_MEMORY_ERROR;
    }
    (void)lwrb_init(rtmLwRB_ServerQueue, buff_data, RTM_INVOKEMETHOD_QUEUE_SIZE);

    void *buff_data2;
    buff_data2 = malloc(RTM_INVOKEMETHOD_QUEUE_SIZE);
    if (nullptr == buff_data2) {
        delete logCircleBuf;
        delete loggerOutputCircleBuf;

        logCircleBuf = nullptr;
        loggerOutputCircleBuf = nullptr;
        free(buff_data);

        return E_OUT_OF_MEMORY_ERROR;
    }
    (void)lwrb_init(rtmLwRB_ClientQueue, buff_data2, RTM_INVOKEMETHOD_QUEUE_SIZE);

    return NOERROR;
}

static RTM_CpuMemoryStatus GetRtmCpuMemoryStatus()
{
    RTM_CpuMemoryStatus status;
    status.cpuUsagePercent = CpuCoreUtils::GetProcCpuUsagePercent(getpid());

#if defined __GLIBC__ && defined __linux__
#if __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 33
    struct mallinfo2 mi;
    mi = mallinfo2();
    status.totalAllocdSpace = mi.uordblks;
    status.totalFreeSpace = mi.fordblks;
#else
    struct mallinfo mi;
    mi = mallinfo();
    status.totalAllocdSpace = mi.uordblks;
    status.totalFreeSpace = mi.fordblks;
#endif
#endif
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
        case RTM_CommandType::CMD_BY_STRING: {
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
            else {
                return E_OUT_OF_MEMORY_ERROR;
            }

            break;
        }
        case RTM_CommandType::CMD_Server_InvokeMethod: {
            Mutex::AutoLock lock(rtmInvokeMethodServerQueue_Lock);

            if (lwrb_get_free(rtmLwRB_ServerQueue) != 0) {
                size_t len;
                if (lwrb_peek(rtmLwRB_ServerQueue, 0, &len, sizeof(size_t))
                                                                    != sizeof(size_t)) {
                    return E_OUT_OF_MEMORY_ERROR;
                }

                resBuffer = Array<Byte>(len);
                if (! resBuffer.IsNull()) {
                    if (lwrb_read(rtmLwRB_ServerQueue, resBuffer.GetPayload(), len)!= len) {
                        return E_OUT_OF_MEMORY_ERROR;
                    }
                }
                else {
                    return E_OUT_OF_MEMORY_ERROR;
                }
            }
            break;
        }
        case RTM_CommandType::CMD_Server_Dump_InvokeMethod: {
            Mutex::AutoLock lock(rtmInvokeMethodServerQueue_Lock);

            String strBuffer = "[";

            while (lwrb_get_free(rtmLwRB_ServerQueue) != 0) {
                size_t len;
                if (lwrb_peek(rtmLwRB_ServerQueue, 0, &len, sizeof(size_t))
                                                                    != sizeof(size_t)) {
                    return E_OUT_OF_MEMORY_ERROR;
                }

                Array<Byte> buffer = Array<Byte>(len);
                if (! buffer.IsNull()) {
                    if (lwrb_read(rtmLwRB_ServerQueue, buffer.GetPayload(), len) != len) {
                        return E_OUT_OF_MEMORY_ERROR;
                    }
                }
                else {
                    return E_OUT_OF_MEMORY_ERROR;
                }

                DeserializeRtmInvokeMethod(reinterpret_cast<como::RTM_InvokeMethod*>(
                                                                    buffer.GetPayload()));

                String str;
                ECode ec = DumpRtmInvokeMethod(reinterpret_cast<como::RTM_InvokeMethod*>(
                                                                buffer.GetPayload()), str);
                if (SUCCEEDED(ec)) {
                    strBuffer += (str + ",");
                }
                else {
                    return ec;
                }
            }

            if (strBuffer.GetByteLength() > 1) {
                // replace the last ',' with ']'
                char* str = (char*)strBuffer.string();
                str[strBuffer.GetByteLength() - 1] = ']';
            }
            else {
                strBuffer += "]";
            }

            resBuffer = Array<Byte>(strBuffer.GetByteLength()+1);
            if (! resBuffer.IsNull()) {
                resBuffer.CopyRaw((Byte*)strBuffer.string(), strBuffer.GetByteLength()+1);
            }
            else {
                return E_OUT_OF_MEMORY_ERROR;
            }
            break;
        }
        case RTM_CommandType::CMD_Server_CpuMemoryStatus: {
            RTM_CpuMemoryStatus status = GetRtmCpuMemoryStatus();
            resBuffer = Array<Byte>(sizeof(RTM_CpuMemoryStatus));
            if (! resBuffer.IsNull()) {
                // resBuffer.Copy works very slowly
                memcpy(resBuffer.GetPayload(), &status,
                                                   sizeof(RTM_CpuMemoryStatus));
            }
            else {
                return E_OUT_OF_MEMORY_ERROR;
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
    FAIL_RETURN(IObject::Probe(intf)->GetCoclass(klass));
    AutoPtr<IMetaInterface> imintf;
    FAIL_RETURN(klass->GetInterface(iid, imintf));
    FAIL_RETURN(imintf->GetMethod(rtm_InvokeMethod->methodIndexPlus4, method));

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
    if (nullptr == mUri_) {
        mUri_ = "";
    }

    Array<Byte> arrCid_(sizeof(ComponentID) + strlen(mUri_) + 1);
    Byte *pByte = arrCid_.GetPayload();
    if (nullptr == pByte) {
        return E_OUT_OF_MEMORY_ERROR;
    }

    memcpy(pByte, cid, sizeof(ComponentID));
    strcpy((char*)(pByte + sizeof(ComponentID)), mUri_);

    arrCid = arrCid_;

    return NOERROR;
}

/**
 * Write Runtime Invoke Method information into rtmLwRB_Client(/Server)Queue
 */
ECode RuntimeMonitor::WriteRtmInvokeMethod(Long uuid64,
                                      Long serverObjectId, CoclassID& clsId,
                                      InterfaceID iid, RTM_ParamTransDirection in_out,
                                      Integer methodIndexPlus4, IParcel *parcel,
                                      RTM_WhichQueue whichQueue)
{
    Array<Byte> arrCid;
    SerializeComponentID(clsId.mCid, arrCid);

    Long sizeParcel;
    parcel->GetDataSize(sizeParcel);

    size_t len = sizeof(RTM_InvokeMethod) + sizeParcel + arrCid.GetLength();
    if (len > RTM_INVOKEMETHOD_QUEUE_SIZE / 2) {
        // Too much parameter data to monitor
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    RTM_InvokeMethod *rtm_InvokeMethod;
#ifdef COMO_FUNCTION_SAFETY_RTOS
    char buf[RTM_MAX_PARCEL_SIZE];
    if (len > RTM_MAX_PARCEL_SIZE) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    rtm_InvokeMethod = reinterpret_cast<RTM_InvokeMethod *>(buf);
#else
    rtm_InvokeMethod = (RTM_InvokeMethod *)malloc(len);
    if (nullptr == rtm_InvokeMethod) {
        return E_OUT_OF_MEMORY_ERROR;
    }
#endif

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    // The time from January 1, 1970. Unit: microseconds
    rtm_InvokeMethod->time = ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
                                       // 654321
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
    if (RTM_WhichQueue::InvokeMethodClientQueue == whichQueue) {
        Mutex::AutoLock lock(rtmInvokeMethodClientQueue_Lock);

        while ((lwrb_get_free(rtmLwRB_ClientQueue) < rtm_InvokeMethod->length) &&
                            (lwrb_get_full(rtmLwRB_ClientQueue) > sizeof(size_t))) {
            if (lwrb_peek(rtmLwRB_ClientQueue, 0, &len, sizeof(size_t)) != sizeof(size_t)) {
#ifndef COMO_FUNCTION_SAFETY_RTOS
                free(rtm_InvokeMethod);
#endif
                return E_OUT_OF_MEMORY_ERROR;
            }

            if (lwrb_advance(rtmLwRB_ClientQueue, len) != len) {
#ifndef COMO_FUNCTION_SAFETY_RTOS
                free(rtm_InvokeMethod);
#endif
                return E_OUT_OF_MEMORY_ERROR;
            }
        }

        if (lwrb_write(rtmLwRB_ClientQueue, rtm_InvokeMethod,
                       rtm_InvokeMethod->length) < rtm_InvokeMethod->length) {
#ifndef COMO_FUNCTION_SAFETY_RTOS
            free(rtm_InvokeMethod);
#endif
            return E_OUT_OF_MEMORY_ERROR;
        }

#ifndef COMO_FUNCTION_SAFETY_RTOS
        free(rtm_InvokeMethod);
#endif
    }
    else {
        Mutex::AutoLock lock(rtmInvokeMethodServerQueue_Lock);

        while ((lwrb_get_free(rtmLwRB_ServerQueue) < rtm_InvokeMethod->length) &&
                            (lwrb_get_full(rtmLwRB_ServerQueue) > sizeof(size_t))) {
            if (lwrb_peek(rtmLwRB_ServerQueue, 0, &len, sizeof(size_t)) != sizeof(size_t)) {
#ifndef COMO_FUNCTION_SAFETY_RTOS
                free(rtm_InvokeMethod);
#endif
                return E_OUT_OF_MEMORY_ERROR;
            }

            if (lwrb_advance(rtmLwRB_ServerQueue, len) != len) {
#ifndef COMO_FUNCTION_SAFETY_RTOS
                free(rtm_InvokeMethod);
#endif
                return E_OUT_OF_MEMORY_ERROR;
            }
        }

        if (lwrb_write(rtmLwRB_ServerQueue, rtm_InvokeMethod,
                       rtm_InvokeMethod->length) < rtm_InvokeMethod->length) {
#ifndef COMO_FUNCTION_SAFETY_RTOS
            free(rtm_InvokeMethod);
#endif
            return E_OUT_OF_MEMORY_ERROR;
        }

#ifndef COMO_FUNCTION_SAFETY_RTOS
        free(rtm_InvokeMethod);
#endif
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
    if (FAILED(ec)) {
        return ec;
    }

    strBuffer = "{\"tag\":\"RM_IM_\",";

    snprintf(buf, sizeof(buf), "\"id\":%llu,", (unsigned long long)rtm_InvokeMethod->uuid64);
    strBuffer += buf;

    struct timeval curTime;                  // 123456
    curTime.tv_sec = rtm_InvokeMethod->time /  1000000;
    curTime.tv_usec = rtm_InvokeMethod->time % 1000000;
    int milli = curTime.tv_usec / 1000;

    // format examlpe: "2021-11-30 12:34:56"
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

    FAIL_RETURN(klass->GetName(name));
    FAIL_RETURN(klass->GetNamespace(ns));
    strBuffer += "\"coclass\":\"" + ns + "::" + name + "\",";

    memcpy(&iid, &(rtm_InvokeMethod->interfaceID_mUuid), sizeof(UUID));
    FAIL_RETURN(klass->GetInterface(iid, intf));
    FAIL_RETURN(klass->GetName(name));
    FAIL_RETURN(klass->GetNamespace(ns));
    strBuffer += "\"interface\":\"" + ns + "::" + name + "\",";

    intf->GetMethod(rtm_InvokeMethod->methodIndexPlus4, method);
    String strMethod;
    String methodSignature;
    FAIL_RETURN(method->GetName(strMethod));
    FAIL_RETURN(method->GetSignature(methodSignature));
    strBuffer += "\"method\":\"" + strMethod + "{" + methodSignature + "}\",";

    String strArgBuffer;
    AutoPtr<IParcel> parcel;
    FAIL_RETURN(CoCreateParcel(RPCType::Remote, parcel));

    Long len = rtm_InvokeMethod->length -
                          (sizeof(RTM_InvokeMethod) + sizeof(ComponentID) + 1) -
                          strlen(rtm_InvokeMethod->coclassID.mCid->mUri);

    parcel->SetData((HANDLE)rtm_InvokeMethod->parcel, len);

    if (rtm_InvokeMethod->in_out == RTM_ParamTransDirection::CALL_METHOD) {
        FAIL_RETURN(MarshalUtils::UnMarshalArguments(method, parcel, strArgBuffer));
    }
    else {
        FAIL_RETURN(MarshalUtils::UnUnmarshalResults(method, parcel, strArgBuffer));
    }

    strBuffer += "\"parcel\":" + strArgBuffer;

    strBuffer += "}";

    return NOERROR;
}

RTM_Command* RuntimeMonitor::GenRtmCommand(RTM_CommandType command, Short param,
                                        const Byte *buffer, size_t bufferSize,
                                        Byte *bytesRtmCommand, size_t bytesSize)
{
    RTM_Command *rtmCommand;
    Integer len = sizeof(RTM_Command) + bufferSize;

    if (nullptr != bytesRtmCommand) {
        if (len > bytesSize) {
            return nullptr;
        }
        rtmCommand = (RTM_Command*)bytesRtmCommand;
    }
    else {
        rtmCommand = (RTM_Command*)malloc(len);
    }

    if (nullptr != rtmCommand) {
        rtmCommand->length = len;
        rtmCommand->command = command;
        rtmCommand->param = param;
        memcpy(rtmCommand->parcel, buffer, bufferSize);
    }

    return rtmCommand;
}

RTM_Command* RuntimeMonitor::GenRtmCommand(RTM_CommandType command, Short param,
                                        const char *cstr,
                                        Byte *bytesRtmCommand, size_t bytesSize)
{
    RTM_Command *rtmCommand;
    Integer len = sizeof(RTM_Command) + strlen(cstr) + 1;

    if (nullptr != bytesRtmCommand) {
        if (len > bytesSize) {
            return nullptr;
        }
        rtmCommand = (RTM_Command*)bytesRtmCommand;
    }
    else {
        rtmCommand = (RTM_Command*)malloc(len);
    }

    if (nullptr != rtmCommand) {
        rtmCommand->length = len;
        rtmCommand->command = command;
        rtmCommand->param = param;
        strcpy((char*)rtmCommand->parcel, cstr);
    }

    return rtmCommand;
}

ECode RuntimeMonitor::GenRtmCommand(RTM_CommandType command, Short param,
                                          const Byte *buffer, size_t bufferSize,
                                          Array<Byte>& arrayRtmCommand)
{
    Integer len = sizeof(RTM_Command) + bufferSize;
    arrayRtmCommand = Array<Byte>::Allocate(len);
    if (! arrayRtmCommand.IsNull()) {
        RTM_Command *rtmCommand = (RTM_Command*)arrayRtmCommand.GetPayload();
        rtmCommand->length = len;
        rtmCommand->command = command;
        rtmCommand->param = param;
        memcpy(rtmCommand->parcel, buffer, bufferSize);
        return NOERROR;
    }

    return E_OUT_OF_MEMORY_ERROR;
}

ECode RuntimeMonitor::GenRtmCommand(RTM_CommandType command, Short param,
                                const char *cstr, Array<Byte>& arrayRtmCommand)
{
    Integer len = sizeof(RTM_Command) + strlen(cstr) + 1;
    arrayRtmCommand = Array<Byte>::Allocate(len);
    if (! arrayRtmCommand.IsNull()) {
        RTM_Command *rtmCommand = (RTM_Command*)arrayRtmCommand.GetPayload();
        rtmCommand->length = len;
        rtmCommand->command = command;
        rtmCommand->param = param;
        strcpy((char*)rtmCommand->parcel, cstr);
        return NOERROR;
    }

    return E_OUT_OF_MEMORY_ERROR;
}

} // namespace como
