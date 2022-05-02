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
)

set(SOURCES
    CZMQChannel.cpp
    CZMQChannelFactory.cpp
    CZMQParcel.cpp
    CZMQInterfacePack.cpp
    ThreadPoolZmqActor.cpp
    CZMQUtils.cpp
)

add_library(rpc-ZeroMQ STATIC
    ${SOURCES})
add_dependencies(rpc-ZeroMQ cdl)

COPY(rpc-ZeroMQ ${RPCZeroMQ_SRC_DIR}/CZMQUtils.h ${INC_DIR})

set(DEPLOY_HEADERS
    ${RPCZeroMQ_SRC_DIR}/CZMQUtils.h
)

install(FILES
    ${DEPLOY_HEADERS}
    DESTINATION ${INC_DIR})
