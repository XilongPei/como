#=========================================================================
# Copyright (C) 2018 The C++ Component Model(COMO) Open Source Project
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

set(RPC_SRC_DIR ${RUNTIME_SRC_DIR}/rpc)

#
# use dbus always (?)
#
#if("$ENV{PLATFORM}" STREQUAL "linux")
#    add_subdirectory(dbus)
#elseif("$ENV{PLATFORM}" STREQUAL "android")
#    add_subdirectory(binder)
#endif()
add_subdirectory(dbus)

if("$ENV{RPC_OVER_ZeroMQ_SUPPORT_Enable}" STREQUAL "enable")
    # ====== COMO Over ZeroMQ Support ======
    add_subdirectory(ZeroMQ)
    set(RPC_ZeroMQ_DEFINITIONS -DRPC_OVER_ZeroMQ_SUPPORT)
    add_definitions(${RPC_ZeroMQ_DEFINITIONS})
    set(ZeroMQ_INC_DIR ${EXTERNAL_DIR}/libzmq/include)

    # ^^^^^^ COMO Over ZeroMQ Support ^^^^^^
else()
    set(ZeroMQ_INC_DIR )
endif()

if("$ENV{COMO_FUNCTION_SAFETY}" STREQUAL "enable")
    set(ThreadPoolChannelInvoke ThreadPoolChannelInvoke.cpp)
else()
    set(ThreadPoolChannelInvoke)
endif()

include_directories(
    ./
    ../
    ../type
    ../util
    ../monitor
    ${INC_DIR}
    ${ZeroMQ_INC_DIR}
    ${EXTERNAL_DIR}/circularbuffer
    ${EXTERNAL_DIR}/lwrb
)

set(SOURCES
    CProxy.cpp
    CStub.cpp
    comorpc.cpp
    registry.cpp
    ${ThreadPoolChannelInvoke}
)

add_library(rpc STATIC
    ${SOURCES})
add_dependencies(rpc cdl)

COPY(rpc ${RPC_SRC_DIR}/comorpc.h ${INC_DIR})

set(DEPLOY_HEADERS
    ${RPC_SRC_DIR}/comorpc.h
    ${RPC_SRC_DIR}/ThreadPoolChannelInvoke.h
)

install(FILES
    ${DEPLOY_HEADERS}
    DESTINATION ${INC_DIR})
