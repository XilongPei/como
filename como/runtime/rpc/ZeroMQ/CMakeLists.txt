#=========================================================================
# Copyright (C) 2021 The C++ Component Model(COMO) Open Source Project
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

set(RPCZeroMQ_SRC_DIR ${RPC_SRC_DIR}/ZeroMQ)

# libzmq is optional part of COMO
if(NOT EXISTS "${EXTERNAL_DIR}/libzmq/include/zmq.h")
    message(FATAL_ERROR "external/libzmq hasn't been built")
endif()

if("$ENV{RPC_OVER_ZeroMQ_SUPPORT_Enable}" STREQUAL "enable")
    add_definitions(-DRPC_OVER_ZeroMQ_SUPPORT)
endif()

if("$ENV{RedundantComputing_SUPPORT_Enable}" STREQUAL "enable")
    if(NOT EXISTS "${EXTERNAL_DIR}/phxpaxos/.lib/libphxpaxos.a")
        message(FATAL_ERROR "external/phxpaxos hasn't been built.  \
                Don't set environment variable RedundantComputing_SUPPORT_Enable to enable")
    endif()

    add_definitions(-DRedundantComputing_SUPPORT)
    set(PhxpaxosLib
        ${EXTERNAL_DIR}/phxpaxos/export_como/libPhxPaxosComo.a
        ${EXTERNAL_DIR}/phxpaxos/.lib/libphxpaxos.a
        ${EXTERNAL_DIR}/phxpaxos/third_party/leveldb/libleveldb.a
        ${EXTERNAL_DIR}/phxpaxos/third_party/protobuf/lib/libprotobuf.a
        ${EXTERNAL_DIR}/phxpaxos/third_party/gflags/lib/libgflags.a
    )
    set(PhxpaxosSrc
        PaxosUtils.cpp
    )
else()
    set(PhxpaxosLib "")
    set(PhxpaxosSrc "")
endif()

include_directories(
    ./
    ../
    ../../
    ../../type
    ../../util
    ../../monitor
    ${INC_DIR}
    ${EXTERNAL_DIR}/libcrc/include
    ${EXTERNAL_DIR}/libzmq/include
    ${EXTERNAL_DIR}/circularbuffer
    ${EXTERNAL_DIR}/lwrb
    ${EXTERNAL_DIR}/phxpaxos/export_como
    ${EXTERNAL_DIR}/phxpaxos/include
    ${EXTERNAL_DIR}/phxpaxos/third_party/leveldb/include
)

set(SOURCES
    CZMQChannel.cpp
    CZMQChannelFactory.cpp
    CZMQParcel.cpp
    CZMQInterfacePack.cpp
    ThreadPoolZmqActor.cpp
    CZMQUtils.cpp
    ${PhxpaxosSrc}
)

add_library(rpc-ZeroMQ STATIC
    ${SOURCES}
    ${PhxpaxosLib}
)
add_dependencies(rpc-ZeroMQ cdl)

COPY(rpc-ZeroMQ ${RPCZeroMQ_SRC_DIR}/CZMQUtils.h ${INC_DIR})

set(DEPLOY_HEADERS
    ${RPCZeroMQ_SRC_DIR}/CZMQUtils.h
)

install(FILES
    ${DEPLOY_HEADERS}
    DESTINATION ${INC_DIR})

