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

project(RPCTestExtra CXX)

set(CLIENT_SRC_DIR ${RPC_EX_SRC_DIR}/client)
set(CLIENT_OBJ_DIR ${RPC_EX_OBJ_DIR}/client)

include_directories(
    ./
    ${INC_DIR}
    ${CLIENT_OBJ_DIR})

set(SOURCES
    main.cpp)

set(GENERATED_SOURCES
    ${CLIENT_OBJ_DIR}/RPCTestUnit2.cpp)

IMPORT_LIBRARY(comort.so)
IMPORT_LIBRARY(libservicemanager.so)
IMPORT_GTEST()

add_executable(testRPCCli2
    ${SOURCES}
    ${GENERATED_SOURCES})
target_link_libraries(testRPCCli2 comort.so libservicemanager.so ${GTEST_LIBS})
add_dependencies(testRPCCli2 RPCTestUnit2 libservicemanager gtest gtest_main)

add_custom_command(
    OUTPUT
        ${GENERATED_SOURCES}
    COMMAND
        "$ENV{CDLC}"
        -gen
        -mode-client
        -d ${CLIENT_OBJ_DIR}
        -metadata-so "${BIN_DIR}/RPCTestUnit2.so")

COPY(testRPCCli2 ${CLIENT_OBJ_DIR}/testRPCCli2 ${BIN_DIR})

install(FILES
    ${CLIENT_OBJ_DIR}/testRPCCli2
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