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

namespace como {

class DbusUtils {
public:
    static int SendSignalWithArray(const char *signalName, const void* data, int size);
    static DBusMessage *ListenForMsg(const char *signalName, int milliseconds);

private:
    static constexpr const char* MONITOR_OBJECT_PATH = "/como/monitor/CRuntime";
    static constexpr const char* MONITOR_INTERFACE_PATH = "como.monitor.IRuntime";

    static DBusConnection *mConn;
};

} // namespace como
