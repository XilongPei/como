//=========================================================================
// Copyright (C) 2018 The C++ Component Model(COMO) Open Source Project
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

#include <unordered_map>
#include <dbus/dbus.h>
#include "ServiceManager.h"
#include "comolog.h"
#ifdef RPC_OVER_ZeroMQ_SUPPORT
#include "CZMQUtils.h"
#endif
#include "ComoConfig.h"

namespace jing {

AutoPtr<ServiceManager> ServiceManager::sInstance = new ServiceManager();

AutoPtr<ServiceManager> ServiceManager::GetInstance()
{
    return sInstance;
}

ECode ServiceManager::AddService(
    /* [in] */ const String& name,
    /* [in] */ IInterface* object)
{
    if (name.IsEmpty() || object == nullptr) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    AutoPtr<IInterfacePack> ipack;
    ECode ec = CoMarshalInterface(object, RPCType::Local, ipack);
    if (FAILED(ec)) {
        Logger_E("ServiceManager::AddService",
            "Marshal the interface which named \"%s\" failed.",  name.string());
        return ec;
    }
    ipack->SetServerName(String(""));

    AutoPtr<IParcel> parcel;
    ec = CoCreateParcel(RPCType::Local, parcel);
    if (FAILED(ec)) {
        Logger_E("ServiceManager::AddService", "CoCreateParcel fail, ECode is %d", ec);
        return ec;
    }

    IParcelable::Probe(ipack)->WriteToParcel(parcel);
    HANDLE buffer;
    parcel->GetData(buffer);
    Long size;
    parcel->GetDataSize(size);

    DBusError err;
    DBusConnection* conn = nullptr;
    DBusMessage* msg = nullptr;
    DBusMessage* reply = nullptr;
    DBusMessageIter args, subArg;
    const char* str = nullptr;

    dbus_error_init(&err);

    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        Logger_E("ServiceManager::AddService",
                 "Connect to bus daemon failed, error is \"%s\".", err.message);
        ec = E_RUNTIME_EXCEPTION;
        goto Exit;
    }

    msg = dbus_message_new_method_call(DBUS_NAME, OBJECT_PATH, INTERFACE_PATH,
                                                                  "AddService");
    if (msg == nullptr) {
        Logger_E("ServiceManager::AddService", "Fail to create dbus message.");
        ec = E_RUNTIME_EXCEPTION;
        goto Exit;
    }

    dbus_message_iter_init_append(msg, &args);
    str = name.string();
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &str);
    dbus_message_iter_open_container(&args,
                            DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE_AS_STRING, &subArg);
    dbus_message_iter_append_fixed_array(&subArg,
                                          DBUS_TYPE_BYTE, (void*)&buffer, size);
    dbus_message_iter_close_container(&args, &subArg);

    reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);
    if (dbus_error_is_set(&err)) {
        Logger_E("ServiceManager::AddService",
                          "Fail to send message, error is \"%s\"", err.message);
        ec = E_REMOTE_EXCEPTION;
        goto Exit;
    }

    if (!dbus_message_iter_init(reply, &args)) {
        Logger_E("ServiceManager::AddService", "Reply has no results.");
        ec = E_REMOTE_EXCEPTION;
        goto Exit;
    }

    if (DBUS_TYPE_INT32 != dbus_message_iter_get_arg_type(&args)) {
        Logger_E("ServiceManager::AddService", "The first result is not Integer.");
        ec = E_REMOTE_EXCEPTION;
        goto Exit;
    }

    dbus_message_iter_get_basic(&args, &ec);

    if (FAILED(ec)) {
        Logger_E("ServiceManager::AddService", "Remote call failed with ec = 0x%X", ec);
    }

Exit:
    if (msg != nullptr) {
        dbus_message_unref(msg);
    }
    if (reply != nullptr) {
        dbus_message_unref(reply);
    }
    if (conn != nullptr) {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }

    dbus_error_free(&err);

    return ec;
}

#ifdef RPC_OVER_ZeroMQ_SUPPORT

ECode ServiceManager::AddRemoteService(
    /* [in] */ const String& thisServerName,
    /* [in] */ const String& snServManager,
    /* [in] */ const String& name,
    /* [in] */ IInterface* object)
{
    if (name.IsEmpty() || object == nullptr) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    Logger_D("ServiceManager::AddRemoteService",
            "thisServerName: %s  snServManager: %s  name: %s",
            thisServerName.string(), snServManager.string(), name.string());

    AutoPtr<IInterfacePack> ipack;
    ECode ec = CoMarshalInterface(object, RPCType::Remote, ipack);
    if (FAILED(ec)) {
        Logger_E("ServiceManager::AddRemoteService",
                                "Marshal the interface which named '%s' failed",
                                name.string());
        return ec;
    }

    if ((nullptr == thisServerName) || thisServerName.IsEmpty() ||
        (nullptr == snServManager) || snServManager.IsEmpty()) {
        return AddService(name, object);
    }

    /**
     * Check whether we should call GiveMeAhand to setup waiting endpoint
     */
    std::string str(thisServerName.string());
    std::string strep = ComoConfig::GetZeroMQEndpoint(str);
    if (strep.empty()) {
        Logger_E("ServiceManager::AddRemoteService",
                              "GetZeroMQEndpoint \"%s\" failed", strep.c_str());
        return E_NOT_FOUND_EXCEPTION;
    }

    // Tell ZeroMQ which port to listen to and wait for the request from the client
    // "localhost" is in ComoConfig::ServerNameEndpointMap,
    // added by ComoConfig::AddZeroMQEndpoint
    if (thisServerName.Compare("localhost") != 0) {
        ipack->GiveMeAhand(snServManager);
    }

    /**
     * Prepare parcel
     */
    // Tell others my (thisServerName) identification information as a service
    // provider so that others can find me
    // name, hold name of Service, `ZeroMQ_ServiceNameAndEndpoint`
    ipack->SetServerName(name + "\n" + thisServerName + ";" +
                                                         String(strep.c_str()));

    AutoPtr<IParcel> parcel;
    CoCreateParcel(RPCType::Remote, parcel);
    IParcelable::Probe(ipack)->WriteToParcel(parcel);
    HANDLE buffer;
    parcel->GetData(buffer);
    Long size;
    parcel->GetDataSize(size);

    /**
     * tell ServiceManager to add service
     */
    std::map<std::string, ServerNodeInfo*>::iterator iter =
            ComoConfig::ServerNameEndpointMap.find(std::string(snServManager.string()));
    std::string strServerEndpoint;
    void *socket;

    bool found;
    if (iter != ComoConfig::ServerNameEndpointMap.end()) {
        strServerEndpoint = iter->second->endpoint;
        found = true;
    }
    else {
        found = false;
        strServerEndpoint = "";
        Logger::E("ServiceManager::AddRemoteService",
                        "Unregistered ServerName: %s", snServManager.string());
    }

    if (found && (nullptr != iter->second->socket)) {
        socket = iter->second->socket;
    }
    else {
        socket = CZMQUtils::CzmqGetSocket(nullptr,
                                            snServManager.string(),
                                            strServerEndpoint.c_str(), ZMQ_REQ);
    }

    if (nullptr == socket) {
        Logger::E("ServiceManager::AddRemoteService",
                        "socket is nullptr, ServerManager:%s ServerEndpoint:%s",
                        snServManager.string(), strServerEndpoint.c_str());
        return E_REMOTE_EXCEPTION;
    }

    Integer rc = CZMQUtils::CzmqSendBuf(reinterpret_cast<HANDLE>(nullptr),
                                        ZmqFunCode::AddService,
                                        socket, (const void *)buffer, size);
    if (rc <= 0) {
        Logger::E("ServiceManager::AddRemoteService",
                                               "CzmqSendBuf error, rc: %d", rc);
        return E_REMOTE_EXCEPTION;
    }

    HANDLE hChannel;
    Integer eventCode;
    zmq_msg_t msg;
    rc = CZMQUtils::CzmqRecvMsg(hChannel, eventCode, socket, msg, 0);
    if (rc <= 0) {
        Logger::E("ServiceManager::AddRemoteService",
                                       "CzmqRecvMsg error, rc: %d, ECode: 0x%X",
                                       rc, eventCode);
        return E_REMOTE_EXCEPTION;
    }

    return ec;
}

#else

ECode ServiceManager::AddRemoteService(
    /* [in] */ const String& thisServerName,
    /* [in] */ const String& thatServerName,
    /* [in] */ const String& name,
    /* [in] */ IInterface* object)
{
    return NOERROR;
}
#endif

ECode ServiceManager::GetService(
    /* [in] */ const String& name,
    /* [out] */ AutoPtr<IInterface>& object)
{
    if (name.IsEmpty()) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    object = nullptr;

    ECode ec = NOERROR;
    DBusError err;
    DBusConnection* conn = nullptr;
    DBusMessage* msg = nullptr;
    DBusMessage* reply = nullptr;
    DBusMessageIter args, subArg;
    const char* str = nullptr;

    dbus_error_init(&err);

    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        Logger_E("ServiceManager::GetService",
                 "Connect to bus daemon failed, error is \"%s\".", err.message);
        ec = E_RUNTIME_EXCEPTION;
        goto Exit;
    }

    msg = dbus_message_new_method_call(DBUS_NAME, OBJECT_PATH, INTERFACE_PATH,
                                                                  "GetService");
    if (msg == nullptr) {
        Logger_E("ServiceManager::GetService", "Fail to create dbus message.");
        ec = E_RUNTIME_EXCEPTION;
        goto Exit;
    }

    dbus_message_iter_init_append(msg, &args);
    str = name.string();
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &str);

    reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);
    if (dbus_error_is_set(&err)) {
        Logger_E("ServiceManager::GetService",
                          "Fail to send message, error is \"%s\"", err.message);
        ec = E_REMOTE_EXCEPTION;
        goto Exit;
    }

    if (!dbus_message_iter_init(reply, &args)) {
        Logger_E("ServiceManager::GetService", "Reply has no results.");
        ec = E_REMOTE_EXCEPTION;
        goto Exit;
    }

    if (DBUS_TYPE_INT32 != dbus_message_iter_get_arg_type(&args)) {
        Logger_E("ServiceManager::GetService", "The first result is not Integer.");
        ec = E_REMOTE_EXCEPTION;
        goto Exit;
    }

    dbus_message_iter_get_basic(&args, &ec);

    if (SUCCEEDED(ec)) {
        if (!dbus_message_iter_next(&args)) {
            Logger_E("ServiceManager::GetService", "Reply has no out arguments.");
            ec = E_REMOTE_EXCEPTION;
            goto Exit;
        }
        if (DBUS_TYPE_ARRAY != dbus_message_iter_get_arg_type(&args)) {
            Logger_E("ServiceManager::GetService", "Reply arguments is not array.");
            ec = E_REMOTE_EXCEPTION;
            goto Exit;
        }

        void* replyData = nullptr;
        Integer replySize;
        dbus_message_iter_recurse(&args, &subArg);
        dbus_message_iter_get_fixed_array(&subArg, &replyData, &replySize);
        if (replyData != nullptr) {
            AutoPtr<IParcel> parcel;
            ec = CoCreateParcel(RPCType::Local, parcel);
            if (FAILED(ec)) {
                Logger_E("ServiceManager::GetService",
                                    "CoCreateParcel failed with ec = 0x%X", ec);
                goto Exit;
            }

            parcel->SetData(reinterpret_cast<HANDLE>(replyData), replySize);

            AutoPtr<IInterfacePack> ipack;
            ec = CoCreateInterfacePack(RPCType::Local, ipack);
            if (SUCCEEDED(ec)) {
                ec = IParcelable::Probe(ipack)->ReadFromParcel(parcel);
                if (FAILED(ec)) {
                    Logger_E("ServiceManager::GetService",
                                    "ReadFromParcel failed with ec = 0x%X", ec);
                }

                String str = nullptr;
                ec = ipack->GetServerName(str);
                if ((nullptr == str) || str.IsEmpty()) {
                    ec = CoUnmarshalInterface(ipack, RPCType::Local, object);
                }
                else {
                    ec = CoUnmarshalInterface(ipack, RPCType::Remote, object);
                }

                if (FAILED(ec)) {
                    Logger_E("ServiceManager::GetService",
                              "CoUnmarshalInterface failed with ec = 0x%X", ec);
                }
            }
            else {
                Logger_E("ServiceManager::GetService",
                             "CoCreateInterfacePack failed with ec = 0x%X", ec);
            }
        }
    }
    else {
        Logger_E("ServiceManager::GetService", "Remote call failed with ec = 0x%X", ec);
    }

Exit:
    if (msg != nullptr) {
        dbus_message_unref(msg);
    }
    if (reply != nullptr) {
        dbus_message_unref(reply);
    }
    if (conn != nullptr) {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }

    dbus_error_free(&err);

    return ec;
}

ECode ServiceManager::RemoveService(
    /* [in] */ const String& name)
{
    if (name.IsEmpty()) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    ECode ec = NOERROR;
    DBusError err;
    DBusConnection* conn = nullptr;
    DBusMessage* msg = nullptr;
    DBusMessage* reply = nullptr;
    DBusMessageIter args;
    const char* str = nullptr;

    dbus_error_init(&err);

    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        Logger_E("ServiceManager::RemoveService",
                 "Connect to bus daemon failed, error is \"%s\".", err.message);
        ec = E_RUNTIME_EXCEPTION;
        goto Exit;
    }

    msg = dbus_message_new_method_call(DBUS_NAME, OBJECT_PATH, INTERFACE_PATH,
                                                                "RemoveService");
    if (msg == nullptr) {
        Logger_E("ServiceManager::RemoveService", "Fail to create dbus message.");
        ec = E_RUNTIME_EXCEPTION;
        goto Exit;
    }

    dbus_message_iter_init_append(msg, &args);
    str = name.string();
    dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &str);

    reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);
    if (dbus_error_is_set(&err)) {
        Logger_E("ServiceManager::RemoveService",
                          "Fail to send message, error is \"%s\"", err.message);
        ec = E_REMOTE_EXCEPTION;
        goto Exit;
    }

    if (!dbus_message_iter_init(reply, &args)) {
        Logger_E("ServiceManager", "Reply has no results.");
        ec = E_REMOTE_EXCEPTION;
        goto Exit;
    }

    if (DBUS_TYPE_INT32 != dbus_message_iter_get_arg_type(&args)) {
        Logger_E("ServiceManager::RemoveService",
                                            "The first result is not Integer.");
        ec = E_REMOTE_EXCEPTION;
        goto Exit;
    }

    dbus_message_iter_get_basic(&args, &ec);

    if (FAILED(ec)) {
        Logger_E("ServiceManager::RemoveService",
                                       "Remote call failed with ec = 0x%X", ec);
    }

Exit:
    if (msg != nullptr) {
        dbus_message_unref(msg);
    }
    if (reply != nullptr) {
        dbus_message_unref(reply);
    }
    if (conn != nullptr) {
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
    }

    dbus_error_free(&err);

    return ec;
}

#ifdef RPC_OVER_ZeroMQ_SUPPORT

ECode ServiceManager::RemoveRemoteService(
    /* [in] */ const String& snServManager,
    /* [in] */ const String& name)
{
    if (name.IsEmpty()) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    Logger_D("ServiceManager::RemoveRemoteService",
                                         "snServManager: %s  name: %s",
                                         snServManager.string(), name.string());

    if ((nullptr == snServManager) || snServManager.IsEmpty()) {
        return RemoveService(name);
    }

    /**
     * tell ServiceManager to add service
     */
    std::map<std::string, ServerNodeInfo*>::iterator iter =
            ComoConfig::ServerNameEndpointMap.find(std::string(snServManager.string()));
    std::string strServerEndpoint;
    void *socket;

    bool found;
    if (iter != ComoConfig::ServerNameEndpointMap.end()) {
        strServerEndpoint = iter->second->endpoint;
        found = true;
    }
    else {
        found = false;
        strServerEndpoint = "";
        Logger::E("ServiceManager::RemoveRemoteService",
                        "Unregistered ServerName: %s", snServManager.string());
    }

    if (found && (nullptr != iter->second->socket)) {
        socket = iter->second->socket;
    }
    else {
        socket = CZMQUtils::CzmqGetSocket(nullptr,
                                snServManager.string(),
                                strServerEndpoint.c_str(), ZMQ_REQ);
    }

    if (nullptr == socket) {
        Logger::E("ServiceManager::RemoveRemoteService",
                        "socket is nullptr, ServerManager:%s ServerEndpoint:%s",
                        snServManager.string(), strServerEndpoint.c_str());
        return E_REMOTE_EXCEPTION;
    }

    const char *str = name.string();
    Integer rc = CZMQUtils::CzmqSendBuf(reinterpret_cast<HANDLE>(nullptr),
                                      ZmqFunCode::RemoveService,
                                      socket, (const void *)str, strlen(str)+1);
    if (rc <= 0) {
        Logger::E("ServiceManager::RemoveRemoteService",
                                               "CzmqSendBuf error, rc: %d", rc);
        return E_REMOTE_EXCEPTION;
    }

    HANDLE hChannel;
    Integer eventCode;
    zmq_msg_t msg;
    rc = CZMQUtils::CzmqRecvMsg(hChannel, eventCode, socket, msg, 0);
    if (rc <= 0) {
        Logger::E("ServiceManager::RemoveRemoteService",
                                       "CzmqRecvMsg error, rc: %d, ECode: 0x%X",
                                       rc, eventCode);
        return E_REMOTE_EXCEPTION;
    }

    return NOERROR;
}

ECode ServiceManager::GetRemoteService(
    /* [in] */ const String& snServManager,
    /* [in] */ const String& name,
    /* [out] */ AutoPtr<IInterface>& object)
{
    if (name.IsEmpty()) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    Logger_D("ServiceManager::GetRemoteService",
                                         "snServManager: %s  name: %s",
                                         snServManager.string(), name.string());

    if ((nullptr == snServManager) || snServManager.IsEmpty()) {
        return RemoveService(name);
    }

    /**
     * tell ServiceManager to add service
     */
    std::map<std::string, ServerNodeInfo*>::iterator iter =
            ComoConfig::ServerNameEndpointMap.find(std::string(snServManager.string()));
    std::string strServerEndpoint;
    void *socket;

    bool found;
    if (iter != ComoConfig::ServerNameEndpointMap.end()) {
        strServerEndpoint = iter->second->endpoint;
        found = true;
    }
    else {
        found = false;
        strServerEndpoint = "";
        Logger::E("ServiceManager::GetRemoteService",
                        "Unregistered ServerName: %s", snServManager.string());
    }

    if (found && (nullptr != iter->second->socket)) {
        socket = iter->second->socket;
    }
    else {
        socket = CZMQUtils::CzmqGetSocket(nullptr, snServManager.string(),
                                            strServerEndpoint.c_str(), ZMQ_REQ);
    }

    if (nullptr == socket) {
        Logger::E("ServiceManager::GetRemoteService",
                        "socket is nullptr, ServerManager:%s ServerEndpoint:%s",
                        snServManager.string(), strServerEndpoint.c_str());
        return E_REMOTE_EXCEPTION;
    }

    const char *str = name.string();
    Integer rc = CZMQUtils::CzmqSendBuf(reinterpret_cast<HANDLE>(nullptr),
                                      ZmqFunCode::GetService,
                                      socket, (const void *)str, strlen(str)+1);
    if (rc <= 0) {
        Logger::E("ServiceManager::GetRemoteService",
                                               "CzmqSendBuf error, rc: %d", rc);
        return E_REMOTE_EXCEPTION;
    }

    HANDLE hChannel;
    Integer eventCode;
    zmq_msg_t msg;
    rc = CZMQUtils::CzmqRecvMsg(hChannel, eventCode, socket, msg, 0);
    if (rc <= 0) {
        Logger::E("ServiceManager::GetRemoteService",
                                       "CzmqRecvMsg error, rc: %d, ECode: 0x%X",
                                       rc, eventCode);
        return E_REMOTE_EXCEPTION;
    }

    void *replyData = zmq_msg_data(&msg);
    size_t replySize = zmq_msg_size(&msg);
    ECode ec;

    if (replyData != nullptr) {
        AutoPtr<IParcel> parcel;
        ec = CoCreateParcel(RPCType::Local, parcel);
        if (FAILED(ec)) {
            Logger_E("ServiceManager::GetRemoteService",
                                    "CoCreateParcel failed with ec = 0x%X", ec);
            return ec;
        }

        parcel->SetData(reinterpret_cast<HANDLE>(replyData), replySize);

        AutoPtr<IInterfacePack> ipack;
        ec = CoCreateInterfacePack(RPCType::Remote, ipack);
        if (SUCCEEDED(ec)) {
            ec = IParcelable::Probe(ipack)->ReadFromParcel(parcel);
            if (FAILED(ec)) {
                Logger_E("ServiceManager::GetRemoteService",
                                    "ReadFromParcel failed with ec = 0x%X", ec);
                return ec;
            }

            String str = nullptr;
            ec = ipack->GetServerName(str);
            if ((nullptr == str) || str.IsEmpty()) {
                ec = CoUnmarshalInterface(ipack, RPCType::Local, object);
            }
            else {
                ec = CoUnmarshalInterface(ipack, RPCType::Remote, object);
            }

            if (FAILED(ec)) {
                Logger_E("ServiceManager::GetRemoteService",
                              "CoUnmarshalInterface failed with ec = 0x%X", ec);
                return ec;
            }
        }
        else {
            Logger_E("ServiceManager::GetRemoteService",
                             "CoCreateInterfacePack failed with ec = 0x%X", ec);
            return ec;
        }
    }
    else {
        Logger_E("ServiceManager::GetRemoteService", "Bad reply data");
        return ZMQ_BAD_REPLY_DATA;
    }

    return ec;
}

#else

ECode ServiceManager::RemoveRemoteService(
    /* [in] */ const String& snServManager,
    /* [in] */ const String& name)
{
    return NOERROR;
}

ECode ServiceManager::GetRemoteService(
    /* [in] */ const String& snServManager,
    /* [in] */ const String& name,
    /* [out] */ AutoPtr<IInterface>& object);
{
    return NOERROR;
}

#endif

} // namespace jing
