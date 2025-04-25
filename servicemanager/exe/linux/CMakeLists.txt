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

project(servicemanager CXX)

set(SM_EXE_SRC_DIR ${SM_SRC_DIR}/exe/linux)
set(SM_EXE_OBJ_DIR ${SM_OBJ_DIR}/exe/linux)

if("$ENV{RPC_OVER_ZeroMQ_SUPPORT_Enable}" STREQUAL "enable")
    # ====== COMO Over ZeroMQ Support ======
    set(COMO_RUNTIME_DIR $ENV{ROOT}/como/runtime)
    set(RPC_ZeroMQ_INC
        ${EXTERNAL_DIR}/libcrc/include
        ${COMO_RUNTIME_DIR}
        ${COMO_RUNTIME_DIR}/rpc
        ${COMO_RUNTIME_DIR}/rpc/ZeroMQ
        ${EXTERNAL_DIR}/libzmq/include
    )
    set(RPC_ZeroMQ_SOURCES
        RpcOverZeroMQ.cpp
        ${COMO_RUNTIME_DIR}/util/mutex.cpp
        ${COMO_RUNTIME_DIR}/util/mistring.cpp
        ${COMO_RUNTIME_DIR}/util/MemPool.cpp
        ${COMO_RUNTIME_DIR}/rpc/ZeroMQ/CZMQParcel.cpp
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
    ${SM_EXE_OBJ_DIR}
    /usr/include/dbus-1.0
    /usr/lib/x86_64-linux-gnu/dbus-1.0/include
    ${RPC_ZeroMQ_INC}
    ${EXTERNAL_DIR}/phxpaxos/export_como
    ${EXTERNAL_DIR}/phxpaxos/include
    ${EXTERNAL_DIR}/phxpaxos/third_party/leveldb/include
)

if("$ENV{RedundantComputing_SUPPORT_Enable}" STREQUAL "enable")
    set(PhxpaxosLib
        ${EXTERNAL_DIR}/phxpaxos/export_como/libPhxPaxosComo.a
        ${EXTERNAL_DIR}/phxpaxos/.lib/libphxpaxos.a
        ${EXTERNAL_DIR}/phxpaxos/third_party/leveldb/libleveldb.a
        ${EXTERNAL_DIR}/phxpaxos/third_party/protobuf/lib/libprotobuf.a
        ${EXTERNAL_DIR}/phxpaxos/third_party/gflags/lib/libgflags.a
    )
    set(PhxpaxosSrc "")

    # <command-line>: warning: "COMO_FUNCTION_SAFETY" redefined
    #set(Phxpaxos_DEFINITIONS -DCOMO_FUNCTION_SAFETY)
    set(Phxpaxos_DEFINITIONS "")
else()
    set(PhxpaxosLib "")
    set(PhxpaxosSrc "")
    set(Phxpaxos_DEFINITIONS "")
endif()

set(SOURCES
    ServiceManager.cpp
    main.cpp
    ServiceManagerOptions.cpp
    ServiceManagerMonitor.cpp
    ${RPC_ZeroMQ_SOURCES}
    ${PhxpaxosSrc}
)

IMPORT_LIBRARY(comort.so)

add_definitions(-DCOMO_WITH_RPC ${RPC_ZeroMQ_DEFINITIONS} ${Phxpaxos_DEFINITIONS})

add_executable(servicemanager
    ${SOURCES})
target_link_libraries(servicemanager
    comort.so
    dbus-1
    pthread
    ${RPC_ZeroMQ_LIBS}
    ${PhxpaxosLib}
)
add_dependencies(servicemanager comort)

COPY(servicemanager ${SM_EXE_OBJ_DIR}/servicemanager ${BIN_DIR})

install(FILES
    ${SM_EXE_OBJ_DIR}/servicemanager
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

