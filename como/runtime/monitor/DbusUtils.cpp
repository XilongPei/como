//=========================================================================
// Copyright (C) 2024 The C++ Component Model(COMO) Open Source Project
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

#include <dbus/dbus.h>
#include "util/comolog.h"
#include "DbusUtils.h"

namespace como {

int DbusUtils::SendSignalWithArray(const char *signelName, const void* data, int size)
{
    DBusConnection *conn;
    DBusError       err;
    DBusMessage    *msg;
    DBusMessageIter args, subArg;

    if ((nullptr == signelName) || (nullptr == data)) {
        return -1;
    }

    dbus_error_init(&err);

    conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        Logger::E("DbusUtils::SendSignalWithString",
                                           "Connection Error: %s", err.message);
        dbus_error_free(&err);
    }
    if (nullptr == conn) {
        return -2;
    }

    msg = dbus_message_new_signal(MONITOR_OBJECT_PATH,
                                            MONITOR_INTERFACE_PATH, signelName);
    if (nullptr == msg) {
        Logger::E("DbusUtils::SendSignalWithString", "Message Null");
        return -3;
    }

    dbus_message_iter_init_append(msg, &args);

    if (size < 0) {
        if (! dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &data)) {
            Logger::E("DbusUtils::SendSignalWithString", "Out of Memory!");
            return -4;
        }
    }
    else {
        if (! dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY,
                                           DBUS_TYPE_BYTE_AS_STRING, &subArg)) {
            Logger::E("DbusUtils::SendSignalWithString", "Out of Memory!");
            return -4;
        }
        if (! dbus_message_iter_append_fixed_array(&subArg,
                                                 DBUS_TYPE_BYTE, &data, size)) {
            Logger::E("DbusUtils::SendSignalWithString", "Out of Memory!");
            return -4;
        }

        if (! dbus_message_iter_close_container(&args, &subArg)) {
            Logger::E("DbusUtils::SendSignalWithString", "Out of Memory!");
            return -4;
        }
    }

    if (! dbus_connection_send(conn, msg, nullptr)) {
        Logger::E("DbusUtils::SendSignalWithString", "Out of Memory!");
        return -5;
    }

    dbus_connection_flush(conn);
    dbus_message_unref(msg);

    return 0;
}

} // namespace como
