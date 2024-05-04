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

project(libservicemanager CXX)

set(SM_LIB_SRC_DIR ${SM_SRC_DIR}/lib/linux)
set(SM_LIB_OBJ_DIR ${SM_OBJ_DIR}/lib/linux)

set(COMO_RUNTIME_DIR $ENV{ROOT}/como/runtime)
if("$ENV{RPC_OVER_ZeroMQ_SUPPORT_Enable}" STREQUAL "enable")
    # ====== COMO Over ZeroMQ Support ======
    set(RPC_ZeroMQ_INC
        ${EXTERNAL_DIR}/libcrc/include
        ${COMO_RUNTIME_DIR}
        ${COMO_RUNTIME_DIR}/rpc
        ${COMO_RUNTIME_DIR}/rpc/ZeroMQ
        ${EXTERNAL_DIR}/libzmq/include
    )
    set(RPC_ZeroMQ_SOURCES
        ${COMO_RUNTIME_DIR}/util/mutex.cpp
        ${COMO_RUNTIME_DIR}/util/mistring.cpp
        ${COMO_RUNTIME_DIR}/util/MemPool.cpp
        ${COMO_RUNTIME_DIR}/rpc/ZeroMQ/CZMQParcel.cpp
        ${COMO_RUNTIME_DIR}/rpc/ZeroMQ/ThreadPoolZmqActor.cpp
        ${COMO_RUNTIME_DIR}/rpc/ZeroMQ/CZMQUtils.cpp
    )
    set(RPC_ZeroMQ_LIBS
        pthread
        ${EXTERNAL_DIR}/libzmq/build/lib/libzmq.a
    )
    set(RPC_ZeroMQ_DEFINITIONS -DRPC_OVER_ZeroMQ_SUPPORT)
    # ^^^^^^ COMO Over ZeroMQ Support ^^^^^^
else()
    set(RPC_ZeroMQ_INC "")
    set(RPC_ZeroMQ_SOURCES "")
    set(RPC_ZeroMQ_LIBS "")
    set(RPC_ZeroMQ_DEFINITIONS "")
endif()

include_directories(
    ./
    ${INC_DIR}
    ${SM_LIB_OBJ_DIR}
    /usr/include/dbus-1.0
    /usr/lib/x86_64-linux-gnu/dbus-1.0/include
    ${RPC_ZeroMQ_INC}
    ${EXTERNAL_DIR}/debugbreak
    ${EXTERNAL_DIR}/circularbuffer
    ${EXTERNAL_DIR}/lwrb
)

set(SOURCES
    ServiceManager.cpp
    rpcHelpers.cpp
    ${RPC_ZeroMQ_SOURCES}
)

IMPORT_LIBRARY(comort.so)

add_definitions(-DCOMO_WITH_RPC)
add_definitions(${RPC_ZeroMQ_DEFINITIONS})

add_library(libservicemanager
    SHARED
    ${SOURCES})
target_link_libraries(libservicemanager
    comort.so
    dbus-1
    pthread
    ${RPC_ZeroMQ_LIBS}
)
add_dependencies(libservicemanager comort)

COPY(libservicemanager ${SM_LIB_SRC_DIR}/ServiceManager.h ${INC_DIR})
COPY(libservicemanager ${SM_LIB_SRC_DIR}/rpcHelpers.h ${INC_DIR})
COPY(libservicemanager ${SM_LIB_OBJ_DIR}/libservicemanager.so ${BIN_DIR})

install(FILES
    ${SM_LIB_OBJ_DIR}/libservicemanager.so
    DESTINATION ${BIN_DIR}
    PERMISSIONS
        OWNER_READ
        OWNER_WRITE
        OWNER_EXECUTE
        GROUP_READ
        GROUP_WRITE
        GROUP_EXECUTE
        WORLD_READ
        WORLD_EXECUTE)
