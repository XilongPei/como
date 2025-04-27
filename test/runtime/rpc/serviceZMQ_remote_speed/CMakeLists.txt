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

project(RpcZmqTest CXX)

set(SERVICE_SRC_SPEED_DIR ${RPC_SRC_DIR}/serviceZMQ_remote_speed)
set(SERVICE_OBJ_SPEED_DIR ${RPC_OBJ_DIR}/serviceZMQ_remote_speed)

include_directories(
    ./
    ${INC_DIR}
    ${SERVICE_OBJ_SPEED_DIR}
    ${EXTERNAL_DIR}/libzmq/include
)

set(SOURCES
    main.cpp)

set(GENERATED_SOURCES
    ${SERVICE_OBJ_SPEED_DIR}/RPCTestUnit.cpp)

IMPORT_LIBRARY(comort.so)
IMPORT_LIBRARY(libservicemanager.so)

add_executable(testRPCZmqSrvSpeed
    ${SOURCES}
    ${GENERATED_SOURCES})
target_link_libraries(testRPCZmqSrvSpeed comort.so libservicemanager.so pthread)
add_dependencies(testRPCZmqSrvSpeed RPCTestUnit libservicemanager)

add_custom_command(
    OUTPUT
        ${GENERATED_SOURCES}
    COMMAND
        "$ENV{CDLC}"
        -gen
        -mode-client
        -d ${SERVICE_OBJ_SPEED_DIR}
        -metadata-so "${BIN_DIR}/RPCTestUnit.so")

COPY(testRPCZmqSrvSpeed ${SERVICE_OBJ_SPEED_DIR}/testRPCZmqSrvSpeed ${BIN_DIR})

install(FILES
    ${SERVICE_OBJ_SPEED_DIR}/testRPCZmqSrv
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