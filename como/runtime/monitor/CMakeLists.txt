#=========================================================================
# Copyright (C) 2022 The C++ Component Model(COMO) Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#=========================================================================

set(MONITOR_SRC_DIR ${RUNTIME_SRC_DIR}/monitor)

if("$ENV{RPC_OVER_ZeroMQ_SUPPORT_Enable}" STREQUAL "enable")
    add_definitions(-DRPC_OVER_ZeroMQ_SUPPORT)
endif()

if("$ENV{COMO_WITH_RPC}" STREQUAL "rpc")
    set(MonitorWithDbus_Include
        /usr/include/dbus-1.0
        /usr/lib/x86_64-linux-gnu/dbus-1.0/include
    )
    set(MonitorWithDbus_Sources
        DbusUtils.cpp
    )
else()
    set(MonitorWithDbus_Include)
    set(MonitorWithDbus_Sources)
endif()

include_directories(
    ./
    ../
    ../type
    ../util
    ../reflection
    ../rpc
    ${INC_DIR}
    ${EXTERNAL_DIR}/libcrc/include
    ${EXTERNAL_DIR}/libzmq/include
    ${EXTERNAL_DIR}/circularbuffer
    ${EXTERNAL_DIR}/inih
    ${EXTERNAL_DIR}/lwrb
    ${MonitorWithDbus_Include}
)

set(SOURCES
    RuntimeMonitor.cpp
    CpuCoreUtils.cpp
    MarshalUtils.cpp
    ${MonitorWithDbus_Sources}
    ${EXTERNAL_DIR}/lwrb/lwrb.c
)

add_library(RuntimeMonitor STATIC
    ${SOURCES})
add_dependencies(RuntimeMonitor cdl component)

COPY(RuntimeMonitor ${MONITOR_SRC_DIR}/RuntimeMonitor.h ${INC_DIR})
COPY(RuntimeMonitor ${MONITOR_SRC_DIR}/CpuCoreUtils.h ${INC_DIR})

set(DEPLOY_HEADERS
    ${MONITOR_SRC_DIR}/RuntimeMonitor.h
    ${MONITOR_SRC_DIR}/CpuCoreUtils.h
    DbusUtils.h
)

install(FILES
    ${DEPLOY_HEADERS}
    DESTINATION ${INC_DIR})
