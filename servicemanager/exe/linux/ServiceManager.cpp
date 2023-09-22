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

#include "mistring.h"
#include "ServiceManager.h"

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

    InterfacePack* ipack = new InterfacePack();
    if (nullptr == ipack)
        return E_OUT_OF_MEMORY_ERROR;

    // Because it involves the deep replication of objects in members, we can't
    // simply make a memory directly copy here.
    //memcpy(ipack, &object, sizeof(InterfacePack));
    ipack->mServerName = object.mServerName;
    ipack->mDBusName = object.mDBusName;
    ipack->mCid = object.mCid;
    ipack->mCid.mCid = nullptr;
    ipack->mIid = object.mIid;
    ipack->mIid.mCid = nullptr;
    ipack->mIsParcelable = object.mIsParcelable;
    ipack->mServerObjectId = object.mServerObjectId;

    Mutex::AutoLock lock(mServicesLock);
    if (mServices.Put(name, ipack) != 0) {
        delete ipack;
        return E_OUT_OF_MEMORY_ERROR;
    }

#if 0
// temp code
    size_t poolSize;
    char *s1, *s2, *s3, *s4, *s5, *s6, *s;
    s = MiString::memNewBlockOnce(nullptr, &poolSize,
                                  &s1, ipack->mServerName.GetLength(),
                                  &s2, ipack->mDBusName.GetLength(),
                                  &s3, sizeof(ipack->mCid),
                                  &s4, sizeof(ipack->mIid),
                                  &s5, sizeof(ipack->mIsParcelable),
                                  &s6, sizeof(ipack->mServerObjectId),
                                  nullptr);
    strcpy(s1, ipack->mServerName.string());
    strcpy(s2, ipack->mDBusName.string());
    *(CoclassID *)&s3 = ipack->mCid;
    *(InterfaceID *)&s4 = ipack->mIid;
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

    Mutex::AutoLock lock(mServicesLock);
    *object = mServices.Get(name);
    if (nullptr == *object)
        return E_NOT_FOUND_EXCEPTION;

    return NOERROR;
}

ECode ServiceManager::RemoveService(
    /* [in] */ const String& name)
{
    if (name.IsEmpty()) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    InterfacePack* ipack;
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
        void* data = nullptr;
        Integer size = 0;
        const char* str;
        AutoPtr<IParcel> parcel;
        InterfacePack ipack;
        ECode ec = NOERROR;

        Logger_D("ServiceManager", "ServiceManager::HandleMessage AddService.");

        if (!dbus_message_iter_init(msg, &args)) {
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
        if (!dbus_connection_send(conn, reply, &serial)) {
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

        if (!dbus_message_iter_init(msg, &args)) {
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

        if (FAILED(ec))
            goto GetServiceExit_2;

        if (ipack != nullptr) {
#ifdef RPC_OVER_ZeroMQ_SUPPORT
            if (ipack->mServerName.IsEmpty()) {
#endif
                ec = CoCreateParcel(RPCType::Local, parcel);
                if (FAILED(ec))
                    goto GetServiceExit_2;

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
                if (FAILED(ec))
                    goto GetServiceExit_2;

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
        if (!dbus_connection_send(conn, reply, &serial)) {
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

        if (!dbus_message_iter_init(msg, &args)) {
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
        if (!dbus_connection_send(conn, reply, &serial)) {
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
}

}
