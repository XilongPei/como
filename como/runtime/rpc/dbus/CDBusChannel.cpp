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

#include <vector>
#include <unistd.h>
#include <time.h>
#include "comorpc.h"
#include "CDBusChannel.h"
#include "CDBusParcel.h"
#include "InterfacePack.h"
#include "util/comolog.h"
#include "ComoConfig.h"
#include "registry.h"
#include "ComoConfig.h"

namespace como {

typedef struct tagDBusConnectionContainer {
    DBusConnection* conn;
    void* user_data;
    struct timespec lastAccessTime;
} DBusConnectionContainer;

static std::vector<DBusConnectionContainer*> conns;
static int num_DBUS_DISPATCHER = 0;
static struct timespec lastCheckConnExpireTime = {0,0};

CDBusChannel::ServiceRunnable::ServiceRunnable(
    /* [in] */ CDBusChannel* owner,
    /* [in] */ IStub* target)
    : mOwner(owner)
    , mTarget(target)
    , mRequestToQuit(false)
{}

#define EXIT_dbus_connection_send_with_reply(msg,reply,conn,err)    \
    if (nullptr != msg) {                                           \
        dbus_message_unref(msg);                                    \
    }                                                               \
    if (nullptr != reply) {                                         \
        dbus_message_unref(reply);                                  \
    }                                                               \
    if (nullptr != conn) {                                          \
        dbus_connection_close(conn);                                \
        dbus_connection_unref(conn);                                \
    }                                                               \
    dbus_error_free(&err);


ECode CDBusChannel::ServiceRunnable::Run()
{
    if (conns.size() >= ComoConfig::DBUS_CONNECTION_MAX_NUM) {
        Logger::E("CDBusChannel::ServiceRunnable::Run",
                                                 "Too many D-Bus connections.");
        return E_TOO_MANY_CONNECTION_EXCEPTION;
    }

    DBusError err;
    bool connsNeedCheck = false;

    dbus_error_init(&err);

    DBusConnection* conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        Logger::E("CDBusChannel::ServiceRunnable::Run",
                  "Connect to bus daemon failed, error is \"%s\".", err.message);
        dbus_error_free(&err);
        return E_RUNTIME_EXCEPTION;
    }

    const char* name = dbus_bus_get_unique_name(conn);
    if (nullptr == name) {
        Logger::E("CDBusChannel::ServiceRunnable::Run", "Get unique name failed.");
        if (nullptr != conn) {
            dbus_connection_close(conn);
            dbus_connection_unref(conn);
        }
        dbus_error_free(&err);
        return E_RUNTIME_EXCEPTION;
    }

    mOwner->mName = name;

    DBusObjectPathVTable opVTable;

    opVTable.unregister_function = nullptr;
    opVTable.message_function = CDBusChannel::ServiceRunnable::HandleMessage;

    if (! dbus_connection_register_object_path(conn,
                       STUB_OBJECT_PATH, &opVTable, static_cast<void*>(this))) {

        Logger::E("CDBusChannel::ServiceRunnable::Run",
                                "dbus_connection_register_object_path failed.");
        if (nullptr != conn) {
            dbus_connection_close(conn);
            dbus_connection_unref(conn);
        }
        dbus_error_free(&err);
        return E_RUNTIME_EXCEPTION;
    }

    {
        Mutex::AutoLock lock(mOwner->mLock);
        mOwner->mStarted = true;
    }
    mOwner->mCond.Signal();

    DBusConnectionContainer *conn_;
    Mutex connsLock;
    {
        Mutex::AutoLock lock(connsLock);
        conn_ = (DBusConnectionContainer*)malloc(sizeof(DBusConnectionContainer));
        if (nullptr == conn_) {
            Logger::E("CDBusChannel", "malloc failed.");
            if (nullptr != conn) {
                dbus_connection_close(conn);
                dbus_connection_unref(conn);
            }
            dbus_error_free(&err);
            return E_RUNTIME_EXCEPTION;
        }

        conn_->conn = conn;
        conn_->user_data = static_cast<void*>(this);
        REFCOUNT_ADD(this);
        clock_gettime(CLOCK_REALTIME, &conn_->lastAccessTime);
        conns.push_back(conn_);
    }

    if (num_DBUS_DISPATCHER < ComoConfig::ThreadPool_MAX_DBUS_DISPATCHER) {
        num_DBUS_DISPATCHER++;

        while (true) {
            DBusDispatchStatus status;
            struct timespec currentTime;
            DBusConnection *conn_dbus;

            clock_gettime(CLOCK_REALTIME, &currentTime);

            {
                Mutex::AutoLock lock(connsLock);

                //@ `Mechanism CHECK_FOR_FREE`

                // check for free time out connection
                // ns accuracy is not required
                if (conns.size() > ComoConfig::DBUS_CONNECTION_MAX_NUM ||
                         connsNeedCheck ||
                         (currentTime.tv_sec - lastCheckConnExpireTime.tv_sec) >
                                    ComoConfig::DBUS_BUS_CHECK_EXPIRES_PERIOD) {
                    clock_gettime(CLOCK_REALTIME, &lastCheckConnExpireTime);

                    // The reason why we iterate over conns here is because we want to release the
                    // member of conns in the loop
                    for (std::vector<DBusConnectionContainer*>::iterator it = conns.begin();
                                                                         it != conns.end(); ) {
                        if (1000000000L * (currentTime.tv_sec - (*it)->lastAccessTime.tv_sec) +
                           /*987654321*/(currentTime.tv_nsec - (*it)->lastAccessTime.tv_nsec) >
                                                        ComoConfig::DBUS_BUS_SESSION_EXPIRES) {

                            IInterface* intf = reinterpret_cast<IInterface*>((*it)->user_data);
                            REFCOUNT_RELEASE(intf);

                            conn_dbus = (*it)->conn;
                            dbus_connection_close(conn_dbus);
                            dbus_connection_unref(conn_dbus);

                            free(*it);
                            conns.erase(it);
                        }
                        else {
                            it++;
                        }
                    }
                }
            }

            for (size_t i = 0;  i < conns.size();  i++) {
                {
                    Mutex::AutoLock lock(connsLock);
                    conn_dbus = conns[i]->conn;
                }

                do {
                    // dbus_connection_read_write_dispatch() return TRUE if the
                    // disconnect message has not been processed
                    if (! dbus_connection_read_write_dispatch(conn_dbus, 0)) {
                        // `Mechanism CHECK_FOR_FREE` will release the DBusConnection
                        conns[i]->lastAccessTime.tv_sec = 0;
                        connsNeedCheck = true;
                    }

                    status = dbus_connection_get_dispatch_status(conn_dbus);
                    if (DBUS_DISPATCH_DATA_REMAINS == status) {
                    // DBUS_DISPATCH_DATA_REMAINS indicates that the message queue may contain messages.
                    // Note, DBUS_DISPATCH_DATA_REMAINS really means that either we have messages in the
                    // queue, or we have raw bytes buffered up that need to be parsed. When these bytes
                    // are parsed, they may not add up to an entire message. Thus, it's possible to see
                    // a status of DBUS_DISPATCH_DATA_REMAINS but not have a message yet.
                        clock_gettime(CLOCK_REALTIME, &(conns[i]->lastAccessTime));
                    }
                    else {
                        break;
                    }
                } while (! mRequestToQuit);

                if (mRequestToQuit) {
                    // `Mechanism CHECK_FOR_FREE` will release the DBusConnection
                    conns[i]->lastAccessTime.tv_sec = 0;
                    connsNeedCheck = true;
                }

                if (status == DBUS_DISPATCH_NEED_MEMORY) {
                // DBUS_DISPATCH_NEED_MEMORY indicates that there could be data,
                // but we can't know for sure without more memory.
                    Logger::E("CDBusChannel", "DBus dispatching needs more memory.");
                    break;
                }
            }

            // usleep - suspend execution for microsecond intervals
            // Even if there is no external request, dbus_connection_read_write_dispatch
            // needs to process it circularly
            usleep(20);
        }
    }

    /* the conn should not be closed!
    dbus_connection_close(conn);
    dbus_connection_unref(conn);
    */

    return NOERROR;
}

DBusHandlerResult CDBusChannel::ServiceRunnable::HandleMessage(
    /* [in] */ DBusConnection* conn,
    /* [in] */ DBusMessage* msg,
    /* [in] */ void* arg)
{
    CDBusChannel::ServiceRunnable* thisRunnable = static_cast<CDBusChannel::ServiceRunnable*>(arg);

    if (dbus_message_is_method_call(msg, STUB_INTERFACE_PATH, "GetComponentMetadata")) {
        Logger::D("CDBusChannel", "Handle GetComponentMetadata message");

        DBusMessageIter args;
        DBusMessageIter subArg;
        void* data = nullptr;
        Integer size = 0;

        if (! dbus_message_iter_init(msg, &args)) {
            Logger::E("CDBusChannel", "HandleMessage dbus_message_iter_init() error");
            return DBUS_HANDLER_RESULT_HANDLED;
        }
        if (DBUS_TYPE_ARRAY != dbus_message_iter_get_arg_type(&args)) {
            Logger::E("CDBusChannel", "HandleMessage dbus_message_iter_get_arg_type() error");
            return DBUS_HANDLER_RESULT_HANDLED;
        }
        dbus_message_iter_recurse(&args, &subArg);
        dbus_message_iter_get_fixed_array(&subArg, &data, (int*)&size);

#ifdef COMO_FUNCTION_SAFETY_RTOS
        void *buf = CDBusParcel::MemPoolAlloc(sizeof(CDBusParcel));
        if (nullptr == buf) {
            Logger::E("CDBusChannel", "MemPoolAlloc return nullptr.");
            return DBUS_HANDLER_RESULT_HANDLED;
        }

        CDBusParcel *cdbusParcel = new(buf) CDBusParcel();
        AutoPtr<IParcel> argParcel = cdbusParcel;
        cdbusParcel->SetFunFreeMem(CDBusParcel::MemPoolFree, 0);
#else
        AutoPtr<IParcel> argParcel = new CDBusParcel();
        if (nullptr == argParcel) {
            Logger::E("CDBusChannel", "HandleMessage new CDBusParcel() error");
            return DBUS_HANDLER_RESULT_HANDLED;
        }
#endif

        argParcel->SetData(reinterpret_cast<HANDLE>(data), size);
        CoclassID cid;
        argParcel->ReadCoclassID(cid);
        AutoPtr<IMetaComponent> mc;
        CoGetComponentMetadata(*cid.mCid, nullptr, mc);
        Array<Byte> metadata;
        ECode ec = mc->GetSerializedMetadata(metadata);
        ReleaseCoclassID(cid);

        DBusMessage* reply = dbus_message_new_method_return(msg);
        if (nullptr != reply) {
            dbus_message_iter_init_append(reply, &args);
            dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &ec);

            HANDLE resData;
            Long resSize;
            if (SUCCEEDED(ec)) {
                resData = reinterpret_cast<HANDLE>(metadata.GetPayload());
                resSize = metadata.GetLength();
            }
            else {
                resData = reinterpret_cast<HANDLE>("");
                resSize = 1;
            }

            dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY,
                                             DBUS_TYPE_BYTE_AS_STRING, &subArg);
            dbus_message_iter_append_fixed_array(&subArg, DBUS_TYPE_BYTE,
                                                             &resData, resSize);
            dbus_message_iter_close_container(&args, &subArg);

            dbus_uint32_t serial = 0;
            if (! dbus_connection_send(conn, reply, &serial)) {
                Logger::E("CDBusChannel", "Send reply message failed.");
            }
            dbus_connection_flush(conn);
            dbus_message_unref(reply);
        }
        else {
            dbus_connection_flush(conn);
        }
    }
    else if (dbus_message_is_method_call(msg, STUB_INTERFACE_PATH, "Invoke")) {
        Logger::D("CDBusChannel", "Handle Invoke message.");

        DBusMessageIter args;
        DBusMessageIter subArg;
        void* data = nullptr;
        Integer size = 0;

        if (! dbus_message_iter_init(msg, &args)) {
            Logger::E("CDBusChannel", "Invoke message has no arguments.");
            return DBUS_HANDLER_RESULT_HANDLED;
        }
        if (DBUS_TYPE_ARRAY != dbus_message_iter_get_arg_type(&args)) {
            Logger::E("CDBusChannel", "Invoke message has no ARRAY arguments.");
            return DBUS_HANDLER_RESULT_HANDLED;
        }
        dbus_message_iter_recurse(&args, &subArg);
        dbus_message_iter_get_fixed_array(&subArg, &data, (int*)&size);

#ifdef COMO_FUNCTION_SAFETY_RTOS
        void *buf = CDBusParcel::MemPoolAlloc(sizeof(CDBusParcel));
        if (nullptr == buf) {
            Logger::E("CDBusChannel", "MemPoolAlloc return nullptr.");
            return DBUS_HANDLER_RESULT_HANDLED;
        }

        CDBusParcel *cdbusParcel = new(buf) CDBusParcel();
        AutoPtr<IParcel> argParcel = cdbusParcel;
        cdbusParcel->SetFunFreeMem(CDBusParcel::MemPoolFree, 0);
#else
        AutoPtr<IParcel> argParcel = new CDBusParcel();
        if (argParcel == nullptr) {
            Logger::E("CDBusChannel", "HandleMessage new CDBusParcel() error");
            return DBUS_HANDLER_RESULT_HANDLED;
        }
#endif

        argParcel->SetData(reinterpret_cast<HANDLE>(data), size);

#ifdef COMO_FUNCTION_SAFETY_RTOS
        buf = CDBusParcel::MemPoolAlloc(sizeof(CDBusParcel));
        if (nullptr == buf) {
            Logger::E("CDBusChannel", "MemPoolAlloc return nullptr.");
            return DBUS_HANDLER_RESULT_HANDLED;
        }

        cdbusParcel = new(buf) CDBusParcel();
        AutoPtr<IParcel> resParcel = cdbusParcel;
        cdbusParcel->SetFunFreeMem(CDBusParcel::MemPoolFree, 0);
#else
        AutoPtr<IParcel> resParcel = new CDBusParcel();
        if (resParcel == nullptr) {
            Logger::E("CDBusChannel", "HandleMessage new CDBusParcel() error");
            return DBUS_HANDLER_RESULT_HANDLED;
        }
#endif

        ECode ec = thisRunnable->mTarget->Invoke(argParcel, resParcel);

        DBusMessage* reply = dbus_message_new_method_return(msg);
        if (nullptr != reply) {
            dbus_message_iter_init_append(reply, &args);
            dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &ec);

            HANDLE resData;
            Long resSize;
            dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY,
                                             DBUS_TYPE_BYTE_AS_STRING, &subArg);
            resParcel->GetData(resData);
            resParcel->GetDataSize(resSize);
            dbus_message_iter_append_fixed_array(&subArg, DBUS_TYPE_BYTE,
                                                             &resData, resSize);
            dbus_message_iter_close_container(&args, &subArg);

            dbus_uint32_t serial = 0;
            if (! dbus_connection_send(conn, reply, &serial)) {
                Logger::E("CDBusChannel", "Send reply message failed.");
            }
            dbus_connection_flush(conn);
            dbus_message_unref(reply);
        }
        else {
            dbus_connection_flush(conn);
        }
    }
    else if (dbus_message_is_method_call(msg, STUB_INTERFACE_PATH, "IsPeerAlive")) {
        Logger::D("CDBusChannel", "Handle IsPeerAlive message.");

        DBusMessageIter args;
        Boolean pong;
        RPCType type;
        AutoPtr<IStub> stub;

        ((CStub*)thisRunnable->mTarget)->GetChannel()->GetRPCType(type);
        IObject* obj = ((CStub*)thisRunnable->mTarget)->GetTarget();
        ECode ec = FindExportObject(type, obj, stub);
        if (SUCCEEDED(ec) && (stub == thisRunnable->mTarget)) {
            pong = true;
        }
        else {
            pong = false;
        }

        DBusMessage* reply = dbus_message_new_method_return(msg);
        if (nullptr != reply) {
            ECode ec = NOERROR;
            dbus_bool_t val = pong;

            dbus_message_iter_init_append(reply, &args);
            dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &ec);
            dbus_message_iter_append_basic(&args, DBUS_TYPE_BOOLEAN, &val);

            dbus_uint32_t serial = 0;
            if (! dbus_connection_send(conn, reply, &serial)) {
                Logger::E("CDBusChannel", "Send reply message failed.");
            }
            dbus_connection_flush(conn);
            dbus_message_unref(reply);
        }
        else {
            dbus_connection_flush(conn);
        }
    }
    else if (dbus_message_is_method_call(msg, STUB_INTERFACE_PATH, "Release")) {
        Logger::D("CDBusChannel", "Handle Release message.");

        thisRunnable->mTarget->Release();
        thisRunnable->mRequestToQuit = true;

        DBusMessageIter args;

        DBusMessage* reply = dbus_message_new_method_return(msg);
        if (nullptr != reply) {
            ECode ec = NOERROR;
            dbus_bool_t val = TRUE;

            dbus_message_iter_init_append(reply, &args);
            dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &ec);
            dbus_message_iter_append_basic(&args, DBUS_TYPE_BOOLEAN, &val);

            dbus_uint32_t serial = 0;
            if (! dbus_connection_send(conn, reply, &serial)) {
                Logger::E("CDBusChannel", "Send reply message failed.");
            }
            dbus_connection_flush(conn);
            dbus_message_unref(reply);
        }
        else {
            dbus_connection_flush(conn);
        }
    }
    else if (dbus_message_is_method_call(msg, STUB_INTERFACE_PATH, "ReleaseObject")) {
        DBusMessageIter args;
        DBusMessageIter subArg;
        Long hash;
        ECode ec = NOERROR;

        if (! dbus_message_iter_init(msg, &args)) {
            Logger_E("CDBusChannel", "ReleaseObject message has no arguments.");
            goto ReleaseObjectExit;
        }
        if (DBUS_TYPE_UINT64 != dbus_message_iter_get_arg_type(&args)) {
            Logger_E("CDBusChannel", "ReleaseObject message has no UINT64 argument.");
            goto ReleaseObjectExit;
        }

        dbus_message_iter_get_basic(&args, &hash);

        if (0 != hash) {
            ec = UnregisterExportObjectByHash(RPCType::Remote, hash);
            if (FAILED(ec)) {
                Logger::E("threadHandleMessage",
                                       "Object_Release error, ECode: 0x%X", ec);
            }
        }

    ReleaseObjectExit:
        DBusMessage* reply = dbus_message_new_method_return(msg);
        if (nullptr != reply) {
            dbus_message_iter_init_append(reply, &args);
            dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &ec);
            dbus_uint32_t serial = 0;
            if (! dbus_connection_send(conn, reply, &serial)) {
                Logger_E("CDBusChannel", "Send reply message failed.");
            }
            dbus_connection_flush(conn);
            dbus_message_unref(reply);
        }
        else {
            dbus_connection_flush(conn);
        }
    }
    else if (dbus_message_is_method_call(msg, DBUS_INTERFACE_INTROSPECTABLE, "Introspect")) {
        DBusMessage* reply = dbus_message_new_method_return(msg);
        if (nullptr != reply) {
            const char *introspection_xml =
"<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n"
"\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n"
"<node name=\"/com/example/sample_object0\">\n"
"  <interface name=\"com.example.SampleInterface0\">\n"
"    <method name=\"Frobate\">\n"
"      <arg name=\"foo\" type=\"i\" direction=\"in\"/>\n"
"      <arg name=\"bar\" type=\"s\" direction=\"out\"/>\n"
"      <arg name=\"baz\" type=\"a{us}\" direction=\"out\"/>\n"
"      <annotation name=\"org.freedesktop.DBus.Deprecated\" value=\"true\"/>\n"
"    </method>\n"
"    <method name=\"Bazify\">\n"
"      <arg name=\"bar\" type=\"(iiu)\" direction=\"in\"/>\n"
"      <arg name=\"bar\" type=\"v\" direction=\"out\"/>\n"
"    </method>\n"
"    <method name=\"Mogrify\">\n"
"      <arg name=\"bar\" type=\"(iiav)\" direction=\"in\"/>\n"
"    </method>\n"
"    <signal name=\"Changed\">\n"
"      <arg name=\"new_value\" type=\"b\"/>\n"
"    </signal>\n"
"    <property name=\"Bar\" type=\"y\" access=\"readwrite\"/>\n"
"  </interface>\n"
"  <node name=\"child_of_sample_object\"/>\n"
"  <node name=\"another_child_of_sample_object\"/>\n"
"</node>";
            dbus_message_append_args(reply, DBUS_TYPE_STRING, &introspection_xml,
                                                                DBUS_TYPE_INVALID);

            dbus_uint32_t serial = 0;
            if (! dbus_connection_send(conn, reply, &serial)) {
                Logger::E("CDBusChannel", "Send Introspect reply message failed.");
            }
            dbus_connection_flush(conn);
            dbus_message_unref(reply);
        }
        else {
            dbus_connection_flush(conn);
        }
    }
    else {
        const char* name = dbus_message_get_member(msg);
        if (nullptr != name) {
            Logger::D("CDBusChannel",
                 "The message which name is \"%s\" does not be handled.", name);
        }
    }

    return DBUS_HANDLER_RESULT_HANDLED;
}

//-------------------------------------------------------------------------------

const CoclassID CID_CDBusChannel =
        {{0x8efc6167,0xe82e,0x4c7d,0x89aa,{0x66,0x8f,0x39,0x7b,0x23,0xcc}}, &CID_COMORuntime};

COMO_INTERFACE_IMPL_1(CDBusChannel, Object, IRPCChannel);

COMO_OBJECT_IMPL(CDBusChannel);

CDBusChannel::CDBusChannel(
    /* [in] */ RPCType type,
    /* [in] */ RPCPeer peer)
    : mType(type)
    , mPeer(peer)
    , mStarted(false)
    , mServerObjectId(0)
    , mCond(mLock)
    , mServerName(nullptr)
    , mPubSocket(0)
{}

CMemPool *CDBusChannel::memPool = new CMemPool(nullptr, ComoConfig::POOL_SIZE_Channel,
                                               sizeof(CDBusChannel));
void *CDBusChannel::MemPoolAlloc(size_t ulSize)
{
    return memPool->Alloc(ulSize, MUST_USE_MEM_POOL);
}

void CDBusChannel::MemPoolFree(Short id, const void *p)
{
    memPool->Free((void *)p);
}

ECode CDBusChannel::Apply(
    /* [in] */ IInterfacePack* ipack)
{
    mName = InterfacePack::From(ipack)->GetDBusName();
    return NOERROR;
}

ECode CDBusChannel::GetRPCType(
    /* [out] */ RPCType& type)
{
    type = mType;
    return NOERROR;
}

ECode CDBusChannel::GetServerName(
    /* [out] */ String& serverName)
{
    serverName = mServerName;
    return NOERROR;
}

ECode CDBusChannel::SetServerName(
    /* [in] */ const String& serverName)
{
    mServerName = serverName;
    return NOERROR;
}

ECode CDBusChannel::SetServerObjectId(
        /* [out] */ Long serverObjectId)
{
    mServerObjectId = serverObjectId;
    return NOERROR;
}

ECode CDBusChannel::GetServerObjectId(
    /* [out] */ Long& serverObjectId)
{
    serverObjectId = mServerObjectId;
    return NOERROR;
}

ECode CDBusChannel::IsPeerAlive(
    /* [in, out] */ Long& lvalue,
    /* [out] */ Boolean& alive)
{
    ECode ec = NOERROR;
    DBusError err;
    DBusConnection* conn = nullptr;
    DBusMessage* msg = nullptr;
    DBusMessage* reply = nullptr;
    DBusMessageIter args;
    alive = false;

    dbus_error_init(&err);

    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);

    if (dbus_error_is_set(&err)) {
        Logger::E("CDBusChannel",
                 "Connect to bus daemon failed, error is \"%s\".", err.message);
        alive = false;
        ec = E_RUNTIME_EXCEPTION;
        dbus_error_free(&err);
        goto Exit;
    }

    msg = dbus_message_new_method_call(mName, STUB_OBJECT_PATH,
                                            STUB_INTERFACE_PATH, "IsPeerAlive");
    if (msg == nullptr) {
        Logger::E("CDBusChannel", "Fail to create dbus message.");
        alive = false;
        ec = E_RUNTIME_EXCEPTION;
        goto Exit;
    }

    Logger::D("CDBusChannel", "Send message.");

    reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);
    if (dbus_error_is_set(&err)) {
        Logger::E("CDBusChannel.IsPeerAlive()",
                          "Fail to send message, error is \"%s\"", err.message);
        alive = false;
        ec = E_REMOTE_EXCEPTION;
        dbus_error_free(&err);
        goto Exit;
    }

    if (! dbus_message_iter_init(reply, &args)) {
        Logger::E("CDBusChannel", "Reply has no results.");
        alive = false;
        ec = E_REMOTE_EXCEPTION;
        goto Exit;
    }

    if (DBUS_TYPE_INT32 != dbus_message_iter_get_arg_type(&args)) {
        Logger::E("CDBusChannel", "The first result is not Integer.");
        alive = false;
        ec = E_REMOTE_EXCEPTION;
        goto Exit;
    }

    dbus_message_iter_get_basic(&args, &ec);

    if (SUCCEEDED(ec)) {
        if (! dbus_message_iter_next(&args)) {
            Logger::E("CDBusChannel", "Reply has no out arguments.");
            alive = false;
            ec = E_REMOTE_EXCEPTION;
            goto Exit;
        }
        if (DBUS_TYPE_BOOLEAN != dbus_message_iter_get_arg_type(&args)) {
            Logger::E("CDBusChannel", "Reply arguments is not BOOLEAN.");
            alive = false;
            ec = E_REMOTE_EXCEPTION;
            goto Exit;
        }

        dbus_bool_t val;
        dbus_message_iter_get_basic(&args, &val);
        alive = ((val == TRUE) ? true : false);
    }
    else {
        Logger::D("CDBusChannel", "Remote call failed with ec = 0x%X.", ec);
        alive = false;
    }

Exit:
    EXIT_dbus_connection_send_with_reply(msg, reply, conn, err);
    return ec;
}

ECode CDBusChannel::ReleasePeer(
    /* [out] */ Boolean& alive)
{
    ECode ec = NOERROR;
    DBusError err;
    DBusConnection* conn = nullptr;
    DBusMessage* msg = nullptr;
    DBusMessage* reply = nullptr;
    DBusMessageIter args;
    alive = false;

    dbus_error_init(&err);

    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);

    if (dbus_error_is_set(&err)) {
        Logger::E("CDBusChannel",
                 "Connect to bus daemon failed, error is \"%s\".", err.message);
        ec = E_RUNTIME_EXCEPTION;
        goto Exit;
    }

    msg = dbus_message_new_method_call(mName, STUB_OBJECT_PATH,
                                                STUB_INTERFACE_PATH, "Release");
    if (nullptr == msg) {
        Logger::E("CDBusChannel", "Fail to create dbus message.");
        ec = E_RUNTIME_EXCEPTION;
        goto Exit;
    }

    Logger::D("CDBusChannel", "Send message.");

    reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);
    if (dbus_error_is_set(&err)) {
        Logger::E("CDBusChannel.IsPeerAlive()",
                          "Fail to send message, error is \"%s\"", err.message);
        ec = E_REMOTE_EXCEPTION;
        goto Exit;
    }

    if (! dbus_message_iter_init(reply, &args)) {
        Logger::E("CDBusChannel", "Reply has no results.");
        ec = E_REMOTE_EXCEPTION;
        goto Exit;
    }

    if (DBUS_TYPE_INT32 != dbus_message_iter_get_arg_type(&args)) {
        Logger::E("CDBusChannel", "The first result is not Integer.");
        ec = E_REMOTE_EXCEPTION;
        goto Exit;
    }

    dbus_message_iter_get_basic(&args, &ec);

    if (SUCCEEDED(ec)) {
        if (! dbus_message_iter_next(&args)) {
            Logger::E("CDBusChannel", "Reply has no out arguments.");
            ec = E_REMOTE_EXCEPTION;
            goto Exit;
        }
        if (DBUS_TYPE_BOOLEAN != dbus_message_iter_get_arg_type(&args)) {
            Logger::E("CDBusChannel", "Reply arguments is not BOOLEAN.");
            ec = E_REMOTE_EXCEPTION;
            goto Exit;
        }

        dbus_bool_t val;
        dbus_message_iter_get_basic(&args, &val);
        if (TRUE == val) {
            alive = true;
        }
        else {
            alive = false;
        }
    }
    else {
        Logger::D("CDBusChannel", "Remote call failed with ec = 0x%X.", ec);
    }

Exit:
    EXIT_dbus_connection_send_with_reply(msg, reply, conn, err);
    return ec;
}

ECode CDBusChannel::LinkToDeath(
    /* [in] */ IProxy* proxy,
    /* [in] */ IDeathRecipient* recipient,
    /* [in] */ HANDLE cookie,
    /* [in] */ Integer flags)
{
    return E_UNSUPPORTED_OPERATION_EXCEPTION;
}

ECode CDBusChannel::UnlinkToDeath(
    /* [in] */ IProxy* proxy,
    /* [in] */ IDeathRecipient* recipient,
    /* [in] */ HANDLE cookie,
    /* [in] */ Integer flags,
    /* [out] */ AutoPtr<IDeathRecipient>* outRecipient)
{
    return E_UNSUPPORTED_OPERATION_EXCEPTION;
}

ECode CDBusChannel::GetComponentMetadata(
    /* [in] */ const CoclassID& cid,
    /* [out, callee] */ Array<Byte>& metadata)
{
    DBusError err;
    DBusConnection* conn = nullptr;
    DBusMessage* msg = nullptr;
    DBusMessage* reply = nullptr;
    DBusMessageIter args, subArg;
    HANDLE data;
    Long size;

    AutoPtr<IParcel> parcel;
    ECode ec = CoCreateParcel(RPCType::Local, parcel);
    if (FAILED(ec)) {
        goto Exit;
    }

    ec = parcel->WriteCoclassID(cid);
    if (FAILED(ec)) {
        goto Exit;
    }

    dbus_error_init(&err);

    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);

    if (dbus_error_is_set(&err)) {
        Logger::E("CDBusChannel::GetComponentMetadata",
                  "Connect to bus daemon failed, error is \"%s\".", err.message);
        ec = E_RUNTIME_EXCEPTION;
        goto Exit;
    }

    msg = dbus_message_new_method_call(
            mName, STUB_OBJECT_PATH, STUB_INTERFACE_PATH, "GetComponentMetadata");
    if (nullptr == msg) {
        Logger::E("CDBusChannel", "Fail to create dbus message.");
        ec = E_RUNTIME_EXCEPTION;
        goto Exit;
    }

    dbus_message_iter_init_append(msg, &args);
    dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY,
                                             DBUS_TYPE_BYTE_AS_STRING, &subArg);
    parcel->GetData(data);
    parcel->GetDataSize(size);
    dbus_message_iter_append_fixed_array(&subArg, DBUS_TYPE_BYTE, &data, size);
    dbus_message_iter_close_container(&args, &subArg);

    Logger::D("CDBusChannel", "Send message.");

    reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);
    if (dbus_error_is_set(&err)) {
        Logger::E("CDBusChannel.GetComponentMetadata",
                          "Fail to send message, error is \"%s\"", err.message);
        ec = E_REMOTE_EXCEPTION;
        goto Exit;
    }

    if (! dbus_message_iter_init(reply, &args)) {
        Logger::E("CDBusChannel", "Reply has no results.");
        ec = E_REMOTE_EXCEPTION;
        goto Exit;
    }

    if (DBUS_TYPE_INT32 != dbus_message_iter_get_arg_type(&args)) {
        Logger::E("CDBusChannel", "The first result is not Integer.");
        ec = E_REMOTE_EXCEPTION;
        goto Exit;
    }

    dbus_message_iter_get_basic(&args, &ec);

    if (SUCCEEDED(ec)) {
        if (! dbus_message_iter_next(&args)) {
            Logger::E("CDBusChannel", "Reply has no out arguments.");
            ec = E_REMOTE_EXCEPTION;
            goto Exit;
        }
        if (DBUS_TYPE_ARRAY != dbus_message_iter_get_arg_type(&args)) {
            Logger::E("CDBusChannel", "Reply arguments is not array.");
            ec = E_REMOTE_EXCEPTION;
            goto Exit;
        }

        void* replyData = nullptr;
        Integer replySize;
        dbus_message_iter_recurse(&args, &subArg);
        dbus_message_iter_get_fixed_array(&subArg, &replyData, &replySize);

        metadata = Array<Byte>::Allocate(replySize);
        if (metadata.IsNull()) {
            Logger::E("CDBusChannel", "Malloc %d size metadata failed.", replySize);
            ec = E_OUT_OF_MEMORY_ERROR;
            goto Exit;
        }
        memcpy(metadata.GetPayload(), replyData, replySize);
    }
    else {
        Logger::D("CDBusChannel", "Remote call failed with ec = 0x%X.", ec);
    }

Exit:
    EXIT_dbus_connection_send_with_reply(msg, reply, conn, err);
    return ec;
}

ECode CDBusChannel::Invoke(
    /* [in] */ IMetaMethod* method,
    /* [in] */ IParcel* argParcel,
    /* [out] */ AutoPtr<IParcel>& resParcel)
{
    ECode ec = NOERROR;
    DBusError err;
    DBusConnection* conn = nullptr;
    DBusMessage* msg = nullptr;
    DBusMessage* reply = nullptr;
    DBusMessageIter args, subArg;
    HANDLE data;
    Long size;

    dbus_error_init(&err);

    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);

    if (dbus_error_is_set(&err)) {
        Logger::E("CDBusChannel::Invoke",
                  "Connect to bus daemon failed, error is \"%s\"", err.message);
        ec = E_RUNTIME_EXCEPTION;
        goto Exit;
    }

    msg = dbus_message_new_method_call(mName, STUB_OBJECT_PATH,
                                                 STUB_INTERFACE_PATH, "Invoke");
    if (msg == nullptr) {
        Logger::E("CDBusChannel", "Fail to create dbus message.");
        ec = E_RUNTIME_EXCEPTION;
        goto Exit;
    }

    dbus_message_iter_init_append(msg, &args);
    dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY,
                                             DBUS_TYPE_BYTE_AS_STRING, &subArg);
    argParcel->GetData(data);
    argParcel->GetDataSize(size);
    dbus_message_iter_append_fixed_array(&subArg, DBUS_TYPE_BYTE, &data, size);
    dbus_message_iter_close_container(&args, &subArg);

    Logger::D("CDBusChannel", "Send message.");

    reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);
    if (dbus_error_is_set(&err)) {
        Logger::E("CDBusChannel.Invoke",
                          "Fail to send message, error is \"%s\"", err.message);
        ec = E_REMOTE_EXCEPTION;
        goto Exit;
    }

    if (! dbus_message_iter_init(reply, &args)) {
        Logger::E("CDBusChannel", "Reply has no results.");
        ec = E_REMOTE_EXCEPTION;
        goto Exit;
    }

    if (DBUS_TYPE_INT32 != dbus_message_iter_get_arg_type(&args)) {
        Logger::E("CDBusChannel", "The first result is not Integer.");
        ec = E_REMOTE_EXCEPTION;
        goto Exit;
    }

    dbus_message_iter_get_basic(&args, &ec);

    if (SUCCEEDED(ec)) {
#ifdef COMO_FUNCTION_SAFETY_RTOS
        void *buf = CDBusParcel::MemPoolAlloc(sizeof(CDBusParcel));
        if (nullptr == buf) {
            Logger::E("CDBusChannel", "MemPoolAlloc return nullptr.");
            ec = E_OUT_OF_MEMORY_ERROR;
            goto Exit;
        }

        CDBusParcel *cdbusParcel = new(buf) CDBusParcel();
        resParcel = cdbusParcel;
        cdbusParcel->SetFunFreeMem(CDBusParcel::MemPoolFree, 0);
#else
        resParcel = new CDBusParcel();
        if (nullptr == resParcel) {
            Logger::E("CDBusChannel.Invoke", "Fail to new CDBusParcel()");
            ec = E_OUT_OF_MEMORY_ERROR;
            goto Exit;
        }
#endif

        Integer hasOutArgs;
        method->GetOutArgumentsNumber(hasOutArgs);
        if (0 != hasOutArgs) {
            if (! dbus_message_iter_next(&args)) {
                Logger::E("CDBusChannel", "Reply has no out arguments.");
                ec = E_REMOTE_EXCEPTION;
                goto Exit;
            }
            if (DBUS_TYPE_ARRAY != dbus_message_iter_get_arg_type(&args)) {
                Logger::E("CDBusChannel", "Reply arguments is not ARRAY.");
                ec = E_REMOTE_EXCEPTION;
                goto Exit;
            }

            void* replyData = nullptr;
            Integer replySize;
            dbus_message_iter_recurse(&args, &subArg);
            dbus_message_iter_get_fixed_array(&subArg, &replyData, &replySize);
            if (nullptr != replyData) {
                resParcel->SetData(reinterpret_cast<HANDLE>(replyData), replySize);
            }
        }
    }
    else {
        Logger::D("CDBusChannel", "Remote call failed with ec = 0x%X.", ec);
    }

Exit:
    EXIT_dbus_connection_send_with_reply(msg, reply, conn, err);
    return ec;
}

ECode CDBusChannel::StartListening(
    /* [in] */ IStub* stub)
{
    int ret = 0;

    if (mPeer == RPCPeer::Stub) {
        AutoPtr<ThreadPoolExecutor::Runnable> runable = new ServiceRunnable(this, stub);
        if (nullptr == runable) {
            Logger::E("CDBusChannel::StartListening", "new ServiceRunnable error");
            return E_OUT_OF_MEMORY_ERROR;
        }

        AutoPtr<ThreadPoolExecutor> instance = ThreadPoolExecutor::GetInstance();
        if (nullptr == instance) {
            Logger::E("CDBusChannel::StartListening", "GetInstance error");
            delete runable;
            runable = nullptr;
            return E_OUT_OF_MEMORY_ERROR;
        }

        /**
         * Although the argument to RunTask is bare pointer (variable Runnable* task),
         * but it (variable instance->) remembers the task through a smart pointer
         * (AutoPtr) before returning back.
         */
        ret = instance->RunTask(runable);
    }

    if (0 == ret) {
        Mutex::AutoLock lock(mLock);
        while (! mStarted) {
            mCond.Wait();
        }
    }
    return NOERROR;
}

ECode CDBusChannel::Match(
    /* [in] */ IInterfacePack* ipack,
    /* [out] */ Boolean& matched)
{
    IDBusInterfacePack* idpack = IDBusInterfacePack::Probe(ipack);
    if (idpack != nullptr) {
        InterfacePack* pack = (InterfacePack*)idpack;
        if (pack->GetDBusName().Equals(mName)) {
            matched = true;
            return NOERROR;
        }
    }
    matched = false;
    return NOERROR;
}

ECode CDBusChannel::ReleaseObject(
    /* [in] */ Long objectId)
{
    ECode ec = NOERROR;
    DBusError err;
    DBusConnection* conn = nullptr;
    DBusMessage* msg = nullptr;
    DBusMessage* reply = nullptr;
    DBusMessageIter args;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        Logger_E("CDBusChannel::ReleaseObject",
                 "Connect to bus daemon failed, error is \"%s\".", err.message);
        ec = E_RUNTIME_EXCEPTION;
        goto Exit;
    }

    msg = dbus_message_new_method_call(mName, STUB_OBJECT_PATH,
                                           STUB_INTERFACE_PATH, "ReleaseObject");
    if (nullptr == msg) {
        Logger_E("CDBusChannel::ReleaseObject", "Fail to create dbus message.");
        ec = E_RUNTIME_EXCEPTION;
        goto Exit;
    }

    dbus_message_iter_init_append(msg, &args);

    dbus_message_iter_append_basic(&args, DBUS_TYPE_UINT64, &objectId);

    reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);
    if (dbus_error_is_set(&err)) {
        Logger_E("CDBusChannel::ReleaseObject",
                          "Fail to send message, error is \"%s\"", err.message);
        ec = E_REMOTE_EXCEPTION;
        goto Exit;
    }

    if (! dbus_message_iter_init(reply, &args)) {
        Logger_E("CDBusChannel::ReleaseObject", "Reply has no results.");
        ec = E_REMOTE_EXCEPTION;
        goto Exit;
    }

    if (DBUS_TYPE_INT32 != dbus_message_iter_get_arg_type(&args)) {
        Logger_E("CDBusChannel::ReleaseObject",
                                              "The first result is not INT32.");
        ec = E_REMOTE_EXCEPTION;
        goto Exit;
    }

    dbus_message_iter_get_basic(&args, &ec);

    if (FAILED(ec)) {
        Logger_E("CDBusChannel::ReleaseObject",
                                       "Remote call failed with ec = 0x%X", ec);
    }

Exit:
    EXIT_dbus_connection_send_with_reply(msg, reply, conn, err);
    return ec;
}

ECode CDBusChannel::MonitorRuntime(
    /* [in] */ const Array<Byte>& request,
    /* [out] */ Array<Byte>& response)
{
    return NOERROR;
}

ECode CDBusChannel::SetPubSocket(
    /* [in] */ HANDLE pubSocket)
{
    mPubSocket = pubSocket;
    return NOERROR;
}

} // namespace como
