#=========================================================================
# Copyright (C) 2024 The C++ Component Model(COMO) Open Source Project
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

project(RedundantComputingTest CXX)

set(CLIENT_SRC_DIR ${RPC_SRC_DIR}/RedundantComputing)
set(CLIENT_OBJ_DIR ${RPC_OBJ_DIR}/RedundantComputing)

include_directories(
    ./
    ${INC_DIR}
    ${CLIENT_OBJ_DIR}
    ${EXTERNAL_DIR}/circularbuffer
    ${EXTERNAL_DIR}/lwrb
)

set(SOURCES
    main.cpp)

set(GENERATED_SOURCES
    ${CLIENT_OBJ_DIR}/RPCTestUnit.cpp)

IMPORT_LIBRARY(comort.so)
IMPORT_LIBRARY(libservicemanager.so)
IMPORT_GTEST()

add_executable(testRedundantComputing
    ${SOURCES}
    ${GENERATED_SOURCES})
target_link_libraries(testRedundantComputing comort.so libservicemanager.so ${GTEST_LIBS})
add_dependencies(testRedundantComputing RPCTestUnit libservicemanager gtest gtest_main)

add_custom_command(
    OUTPUT
        ${GENERATED_SOURCES}
    COMMAND
        "$ENV{CDLC}"
        -gen
        -mode-client
        -d ${CLIENT_OBJ_DIR}
        -metadata-so "${BIN_DIR}/RPCTestUnit.so")

COPY(testMonitorZmqCli ${CLIENT_OBJ_DIR}/testRedundantComputing ${BIN_DIR})

install(FILES
    ${CLIENT_OBJ_DIR}/testRedundantComputing
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