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

DBusConnection *DbusUtils::mConn = nullptr;

int DbusUtils::SendSignalWithArray(const char *signalName, const void* data, int size)
{
    DBusMessage    *msg;
    DBusMessageIter args, subArg;

    if ((nullptr == signalName) || (nullptr == data)) {
        return -1;
    }

    if (nullptr == mConn) {
        DBusError err;
        dbus_error_init(&err);

        mConn = dbus_bus_get(DBUS_BUS_SESSION, &err);
        if (dbus_error_is_set(&err)) {
            Logger::E("DbusUtils::SendSignalWithString",
                                            "Connection Error: %s", err.message);
            dbus_error_free(&err);
        }
    }
    if (nullptr == mConn) {
        return -2;
    }

    msg = dbus_message_new_signal(MONITOR_OBJECT_PATH,
                                            MONITOR_INTERFACE_PATH, signalName);
    if (nullptr == msg) {
        Logger::E("DbusUtils::SendSignalWithString", "Message Null");
        return -3;
    }

    dbus_message_iter_init_append(msg, &args);

    int iRet = 0;
    do {
        if (size < 0) {
            if (! dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &data)) {
                Logger::E("DbusUtils::SendSignalWithString", "Out of Memory!");
                iRet = -4;
                break;
            }
        }
        else {
            if (! dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY,
                                           DBUS_TYPE_BYTE_AS_STRING, &subArg)) {
                Logger::E("DbusUtils::SendSignalWithString", "Out of Memory!");
                iRet = -4;
                break;
            }
            if (! dbus_message_iter_append_fixed_array(&subArg,
                                                 DBUS_TYPE_BYTE, &data, size)) {
                Logger::E("DbusUtils::SendSignalWithString", "Out of Memory!");
                iRet = -4;
                break;
            }

            if (! dbus_message_iter_close_container(&args, &subArg)) {
                Logger::E("DbusUtils::SendSignalWithString", "Out of Memory!");
                iRet = -4;
                break;
            }
        }

        if (! dbus_connection_send(mConn, msg, nullptr)) {
            Logger::E("DbusUtils::SendSignalWithString", "Out of Memory!");
            iRet = -5;
            break;
        }
    } while(0);

    dbus_connection_flush(mConn);
    dbus_message_unref(msg);

    return iRet;
}

} // namespace como
