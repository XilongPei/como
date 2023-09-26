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

#include "ServiceManager.h"
#include <comolog.h>
#include <dbus/dbus.h>
#include <cstdio>
#include "mistring.h"

#ifdef RPC_OVER_ZeroMQ_SUPPORT
#include "RpcOverZeroMQ.h"
#endif

#ifdef COMO_FUNCTION_SAFETY
#include "ComoPhxUtils.h"
#endif

using como::Logger;
using jing::ServiceManager;

jing::SmOptions *ServiceManager::options = nullptr;
#ifdef COMO_FUNCTION_SAFETY
como::PhxEchoServer *ServiceManager::oEchoServer = nullptr;
#endif

int main(int argc, char** argv)
{
    ServiceManager::options = new jing::SmOptions(argc, argv);
    if (nullptr == ServiceManager::options) {
        Logger_E("ServiceManager", "No memory, new jing::SmOptions()");
        return 1;
    }

    if (ServiceManager::options->DoShowUsage()) {
        ServiceManager::options->ShowUsage();
        return 0;
    }

    if (ServiceManager::options->HasErrors()) {
        ServiceManager::options->ShowErrors();
        return 0;
    }

#ifdef COMO_FUNCTION_SAFETY
    if ((nullptr != ServiceManager::options) &&
                                (! ServiceManager::options->GetPaxosServer().IsNull())) {
        int num = 2;
        char *word[3];
        char buf[4096];
        como::MiString::strZcpy(buf, ServiceManager::options->GetPaxosServer().string(), 4096);
        char *str = como::MiString::WordBreak(buf, num, word, (char*)";");
        if (num < 2) {
            Logger_E("ServiceManager", "PaxosServer error");
            return 2;
        }

        NodeInfo oMyNode;
        NodeInfoList vecNodeInfoList;

        if (como::ComoPhxUtils::RunPaxos(word[0], oMyNode, word[1], vecNodeInfoList,
                                                    &ServiceManager::oEchoServer) != 0) {
            Logger_E("ServiceManager", "RunPaxos fail\n");
            return 3;
        }

        Logger_D("ServiceManager echo server start, ip %s port %d\n", oMyNode.GetIP().c_str(), oMyNode.GetPort());

    }
#endif

    DBusError err;

    Logger_D("ServiceManager", "starting ...");

#ifdef RPC_OVER_ZeroMQ_SUPPORT
    // TPZA : ThreadPoolZmqActor
    jing::RpcOverZeroMQ::startTPZA_Executor();
#endif

    dbus_error_init(&err);

    DBusConnection* conn = dbus_bus_get_private(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        Logger_E("servicemanager",
                 "Connect to dbus daemon failed, error is \"%s\".",
                 err.message);
        dbus_error_free(&err);
        return -1;
    }

    dbus_bus_request_name(conn, ServiceManager::DBUS_NAME,
                                        DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
    if (dbus_error_is_set(&err)) {
        Logger_E("servicemanager",
                 "Request servicemanager dbus name failed, error is \"%s\".",
                 err.message);
        dbus_error_free(&err);
        dbus_connection_close(conn);
        dbus_connection_unref(conn);
        return -1;
    }

    DBusObjectPathVTable opVTable;

    opVTable.unregister_function = nullptr;
    opVTable.message_function = ServiceManager::HandleMessage;

    dbus_connection_register_object_path(conn,
            ServiceManager::OBJECT_PATH, &opVTable, nullptr);

    while (true) {
        DBusDispatchStatus status;

        do {
            dbus_connection_read_write_dispatch(conn, -1);

            Logger_D("ServiceManager", "dbus_connection_read_write_dispatch");

        } while ((status = dbus_connection_get_dispatch_status(conn))
                == DBUS_DISPATCH_DATA_REMAINS);

        if (status == DBUS_DISPATCH_NEED_MEMORY) {
            Logger_E("CDBusChannel", "DBus dispatching needs more memory.");
            break;
        }
    }

    dbus_connection_close(conn);
    dbus_connection_unref(conn);

    return 0;
}
