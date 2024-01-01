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

#include "int64.h"
#include "mistring.h"
#include "ServiceManager.h"

#ifdef COMO_FUNCTION_SAFETY
#include "ComoPhxUtils.h"
#endif

namespace jing {

AutoPtr<ServiceManager> ServiceManager::sInstance = new ServiceManager();

AutoPtr<ServiceManager> ServiceManager::GetInstance()
{
    return sInstance;
}

ECode ServiceManager::AddService(
    /* [in] */ const String& name,
    /* [in] */ InterfacePack& object)
{
    if (name.IsEmpty()) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

#ifndef COMO_FUNCTION_SAFETY
    InterfacePack *ipack = new InterfacePack();
    if (nullptr == ipack) {
        return E_OUT_OF_MEMORY_ERROR;
    }

    // Because it involves the deep replication of objects in members, we can't
    // simply make a memory directly copy here.
    //memcpy(ipack, &object, sizeof(InterfacePack));
    ipack->mServerName = object.mServerName;
    ipack->mDBusName = object.mDBusName;
    ipack->mCid = CloneCoclassID(object.mCid);
    ipack->mIid = CloneInterfaceID(object.mIid);
    ipack->mIsParcelable = object.mIsParcelable;
    ipack->mServerObjectId = object.mServerObjectId;

    Mutex::AutoLock lock(mServicesLock);
    if (mServices.Put(name, ipack) != 0) {
        /*
         * These two pieces of malloc memory will be held by HashMap mServices,
         * if the mServices.Put succeeds.
         */
        ReleaseCoclassID(ipack->mCid);
        ReleaseInterfaceID(ipack->mIid);

        delete ipack;
        return E_OUT_OF_MEMORY_ERROR;
    }
#else
    if ((nullptr != options) && (! options->GetPaxosServer().IsNull())) {
        size_t poolSize = 8192;
        char buf[8192];
        char *s1, *s2, *s3, *s4, *s5, *s6, *s;

        s = MiString::memNewBlockOnce(buf, &poolSize,
                                      &s1, object.mServerName.GetLength(),
                                      &s2, object.mDBusName.GetLength(),
                                      &s3, sizeof(object.mCid),
                                      &s4, sizeof(object.mIid),
                                      &s5, sizeof(object.mIsParcelable),
                                      &s6, sizeof(object.mServerObjectId),
                                      nullptr);
        if (nullptr == s) {
            return E_OUT_OF_MEMORY_ERROR;
        }

        strcpy(s1, object.mServerName.string());
        strcpy(s2, object.mDBusName.string());
        *(CoclassID *)&s3 = object.mCid;
        *(InterfaceID *)&s4 = object.mIid;
        *(Boolean *)&s5 = object.mIsParcelable;
        *(Long *)&s6 = object.mServerObjectId;

        std::string sEchoReqValue;
        std::string sEchoRespValue;
        std::string sname = std::string(name.string());
        std::string ss = std::string(s);
        int ret = oEchoServer->Echo(como::ComoPhxUtils::MsgEncode(como::ComoPhxUtils::LevelDbWrite, sname, ss),
                                    sEchoRespValue);

        ret = como::ComoPhxUtils::SyncStateData(oEchoServer,
                                                como::ComoPhxUtils::LevelDbWrite, sname, ss,
                                                sEchoRespValue);

        if (ret != 0) {
            Logger_D("ServiceManager", "ServiceManager::HandleMessage AddService. Echo fail, ret %d\n", ret);
        }
    }
    else {
        InterfacePack *ipack = new InterfacePack();
        if (nullptr == ipack) {
            return E_OUT_OF_MEMORY_ERROR;
        }

        // Because it involves the deep replication of objects in members, we can't
        // simply make a memory directly copy here.
        //memcpy(ipack, &object, sizeof(InterfacePack));
        ipack->mServerName = object.mServerName;
        ipack->mDBusName = object.mDBusName;
        ipack->mCid = CloneCoclassID(object.mCid);
        ipack->mIid = CloneInterfaceID(object.mIid);
        ipack->mIsParcelable = object.mIsParcelable;
        ipack->mServerObjectId = object.mServerObjectId;

        Mutex::AutoLock lock(mServicesLock);
        if (mServices.Put(name, ipack) != 0) {
            /*
             * These two pieces of malloc memory will be held by HashMap mServices,
             * if the mServices.Put succeeds.
             */
            ReleaseCoclassID(ipack->mCid);
            ReleaseInterfaceID(ipack->mIid);

            delete ipack;
            return E_OUT_OF_MEMORY_ERROR;
        }
    }
#endif

    return NOERROR;
}

ECode ServiceManager::GetService(
    /* [in] */ const String& name,
    /* [out, callee] */ InterfacePack** object)
{
    if (name.IsEmpty()) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

#ifndef COMO_FUNCTION_SAFETY
    Mutex::AutoLock lock(mServicesLock);
    *object = mServices.Get(name);
    if (nullptr == *object) {
        return E_NOT_FOUND_EXCEPTION;
    }
#else
    if ((nullptr != options) && (! options->GetPaxosServer().IsNull())) {
        std::string value;
        if (como::ComoPhxUtils::GetStateData(std::string(name), value).empty()) {
            return E_NOT_FOUND_EXCEPTION;
        }

        InterfacePack *ipack = new InterfacePack();
        if (nullptr == ipack) {
            return E_OUT_OF_MEMORY_ERROR;
        }

        char *s;
        char buf1[4096];
        char buf2[4096];
        size_t memSize;

        memSize = 4096;
        (*(Int64 *)&memSize).i32.i32_high = 1;   // a value not 0

        s = MiString::memGetBlockOnce((char *)value.c_str(), value.length(),
                                      buf1, memSize,
                                      buf2, memSize,
                                      ipack->mCid, sizeof(ipack->mCid),
                                      ipack->mIid, sizeof(ipack->mIid),
                                      ipack->mIsParcelable, sizeof(ipack->mIsParcelable),
                                      ipack->mServerObjectId, sizeof(ipack->mServerObjectId),
                                      nullptr);
        if (nullptr == s) {
            return E_OUT_OF_MEMORY_ERROR;
        }
    }
#endif

    return NOERROR;
}

ECode ServiceManager::RemoveService(
    /* [in] */ const String& name)
{
    if (name.IsEmpty()) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

#ifdef COMO_FUNCTION_SAFETY
    if ((nullptr != options) && (! options->GetPaxosServer().IsNull())) {
        if (! como::ComoPhxUtils::DelStateData(std::string(name))) {
            return E_NOT_FOUND_EXCEPTION;
        }
        return NOERROR;
    }
#endif

    InterfacePack *ipack;
    {
        Mutex::AutoLock lock(mServicesLock);
        ipack = mServices.Get(name);
        mServices.Remove(name);
    }

    ReleaseCoclassID(ipack->mCid);
    ReleaseInterfaceID(ipack->mIid);
    delete ipack;

    return NOERROR;
}

DBusHandlerResult ServiceManager::HandleMessage(
    /* [in] */ DBusConnection* conn,
    /* [in] */ DBusMessage* msg,
    /* [in] */ void* arg)
{
    if (dbus_message_is_method_call(msg, INTERFACE_PATH, "AddService")) {
        DBusMessageIter args;
        DBusMessageIter subArg;
        void*           data = nullptr;
        Integer         size = 0;
        const char*     str;
        AutoPtr<IParcel> parcel;
        InterfacePack   ipack;
        ECode           ec = NOERROR;

        Logger_D("ServiceManager", "ServiceManager::HandleMessage AddService.");

        if (! dbus_message_iter_init(msg, &args)) {
            Logger_E("ServiceManager", "AddService message has no arguments.");
            ec = E_ILLEGAL_ARGUMENT_EXCEPTION;
            goto AddServiceExit;
        }
        if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args)) {
            Logger_E("ServiceManager", "AddService message has no string arguments.");
            ec = E_ILLEGAL_ARGUMENT_EXCEPTION;
            goto AddServiceExit;
        }

        dbus_message_iter_get_basic(&args, &str);
        dbus_message_iter_next(&args);
        if (DBUS_TYPE_ARRAY != dbus_message_iter_get_arg_type(&args)) {
            Logger_E("ServiceManager", "AddService message has no array arguments.");
            ec = E_ILLEGAL_ARGUMENT_EXCEPTION;
            goto AddServiceExit;
        }
        dbus_message_iter_recurse(&args, &subArg);
        dbus_message_iter_get_fixed_array(&subArg, &data, (int*)&size);

        ec = CoCreateParcel(RPCType::Local, parcel);
        if (SUCCEEDED(ec)) {
            parcel->SetData(reinterpret_cast<HANDLE>(data), size);
            parcel->ReadString(ipack.mDBusName);
            parcel->ReadCoclassID(ipack.mCid);
            parcel->ReadInterfaceID(ipack.mIid);
            parcel->ReadBoolean(ipack.mIsParcelable);
            ec = ServiceManager::GetInstance()->AddService(str, ipack);
        }

    AddServiceExit:
        DBusMessage* reply = dbus_message_new_method_return(msg);
        if (nullptr == reply) {
            Logger_E("ServiceManager::HandleMessage",
                               "dbus_message_new_method_return return nullptr");
            return DBUS_HANDLER_RESULT_HANDLED;
        }

        dbus_message_iter_init_append(reply, &args);
        dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &ec);
        dbus_uint32_t serial = 0;
        if (! dbus_connection_send(conn, reply, &serial)) {
            Logger_E("ServiceManager", "Send reply message failed.");
        }
        dbus_connection_flush(conn);
        dbus_message_unref(reply);
    }
    else if (dbus_message_is_method_call(msg, INTERFACE_PATH, "GetService")) {
        DBusMessageIter args;
        DBusMessageIter subArg;
        const char* str;
        InterfacePack* ipack = nullptr;
        ECode ec = NOERROR;
        HANDLE resData = reinterpret_cast<HANDLE>("");
        Long resSize = 0;
        AutoPtr<IParcel> parcel;

        if (! dbus_message_iter_init(msg, &args)) {
            Logger_E("ServiceManager", "GetService message has no arguments.");
            ec = E_ILLEGAL_ARGUMENT_EXCEPTION;
            goto GetServiceExit;
        }
        if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args)) {
            Logger_E("ServiceManager", "GetService message has no string arguments.");
            ec = E_ILLEGAL_ARGUMENT_EXCEPTION;
            goto GetServiceExit;
        }

        dbus_message_iter_get_basic(&args, &str);

        ec = ServiceManager::GetInstance()->GetService(str, &ipack);

    GetServiceExit:
        DBusMessage* reply = dbus_message_new_method_return(msg);
        if (nullptr == reply) {
            Logger_E("ServiceManager::HandleMessage",
                               "dbus_message_new_method_return return nullptr");
            return DBUS_HANDLER_RESULT_HANDLED;
        }

        dbus_message_iter_init_append(reply, &args);
        dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &ec);

        if (FAILED(ec)) {
            goto GetServiceExit_2;
        }

        if (ipack != nullptr) {
#ifdef RPC_OVER_ZeroMQ_SUPPORT
            if (ipack->mServerName.IsEmpty()) {
#endif
                ec = CoCreateParcel(RPCType::Local, parcel);
                if (FAILED(ec)) {
                    goto GetServiceExit_2;
                }

                // Consistent with InterfacePack::WriteToParcel() write order
                parcel->WriteString(ipack->mDBusName);
                parcel->WriteCoclassID(ipack->mCid);
                parcel->WriteInterfaceID(ipack->mIid);
                parcel->WriteBoolean(ipack->mIsParcelable);
                parcel->WriteLong(ipack->mServerObjectId);
#ifdef RPC_OVER_ZeroMQ_SUPPORT
            }
            else {
                // Keep the same order with InterfacePack::ReadFromParcel() in
                // como/runtime/rpc/ZeroMQ/InterfacePack.cpp
                ec = CoCreateParcel(RPCType::Remote, parcel);
                if (FAILED(ec)) {
                    goto GetServiceExit_2;
                }

                parcel->WriteCoclassID(ipack->mCid);
                parcel->WriteInterfaceID(ipack->mIid);
                parcel->WriteBoolean(ipack->mIsParcelable);
                parcel->WriteLong(ipack->mServerObjectId);
                parcel->WriteString(ipack->mServerName);
            }
#endif
            parcel->GetData(resData);
            parcel->GetDataSize(resSize);

    GetServiceExit_2:
            dbus_message_iter_open_container(&args,
                            DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE_AS_STRING, &subArg);
            dbus_message_iter_append_fixed_array(&subArg,
                                             DBUS_TYPE_BYTE, &resData, resSize);
            dbus_message_iter_close_container(&args, &subArg);
        }

        dbus_uint32_t serial = 0;
        if (! dbus_connection_send(conn, reply, &serial)) {
            Logger_E("ServiceManager", "Send reply message failed.");
        }
        dbus_connection_flush(conn);
        dbus_message_unref(reply);
    }
    else if (dbus_message_is_method_call(msg, INTERFACE_PATH, "RemoveService")) {
        DBusMessageIter args;
        DBusMessageIter subArg;
        const char* str;
        ECode ec = NOERROR;

        if (! dbus_message_iter_init(msg, &args)) {
            Logger_E("ServiceManager", "RemoveService message has no arguments.");
            goto RemoveServiceExit;
        }
        if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args)) {
            Logger_E("ServiceManager", "RemoveService message has no string arguments.");
            goto RemoveServiceExit;
        }

        dbus_message_iter_get_basic(&args, &str);

        ec = ServiceManager::GetInstance()->RemoveService(str);

    RemoveServiceExit:
        DBusMessage* reply = dbus_message_new_method_return(msg);
        if (nullptr == reply) {
            Logger_E("ServiceManager::HandleMessage",
                               "dbus_message_new_method_return return nullptr");
            return DBUS_HANDLER_RESULT_HANDLED;
        }

        dbus_message_iter_init_append(reply, &args);
        dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &ec);
        dbus_uint32_t serial = 0;
        if (! dbus_connection_send(conn, reply, &serial)) {
            Logger_E("ServiceManager", "Send reply message failed.");
        }
        dbus_connection_flush(conn);
        dbus_message_unref(reply);
    }
    else {
        const char* name = dbus_message_get_member(msg);
        if (name != nullptr) {
            Logger_D("servicemanager",
                  "The message which name is \"%s\" does not be handled.", name);
        }
    }

    return DBUS_HANDLER_RESULT_HANDLED;
} // ServiceManager::HandleMessage()

} // namespace jing

