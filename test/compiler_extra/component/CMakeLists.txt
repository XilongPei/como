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

project(CompilerExtraUnit CXX)

set(COMPONENT_SRC_DIR ${TEST_SRC_DIR}/compiler_extra/component)
set(COMPONENT_OBJ_DIR ${TEST_OBJ_DIR}/compiler_extra/component)

include_directories(
    ./
    ${INC_DIR}
    ${COMPONENT_OBJ_DIR}
    ${EXTERNAL_DIR}/libzmq/include
)

set(SOURCES
    CService2.cpp)

set(GENERATED_SOURCES
    ${COMPONENT_OBJ_DIR}/_como_test_rpc_CService2.cpp
    ${COMPONENT_OBJ_DIR}/RPCTestUnit2Pub.cpp
    ${COMPONENT_OBJ_DIR}/MetadataWrapper.cpp)

IMPORT_LIBRARY(comort.so)

add_library(CompilerExtraUnit
    SHARED
    ${SOURCES}
    ${GENERATED_SOURCES})
target_link_libraries(CompilerExtraUnit comort.so)
add_dependencies(CompilerExtraUnit comort)

add_custom_command(
    OUTPUT
        ${GENERATED_SOURCES}
    COMMAND
        "$ENV{CDLC}"
        -gen
        -mode-component
        -d ${COMPONENT_OBJ_DIR}
        -i ${COMPONENT_SRC_DIR}
        -c "${COMPONENT_SRC_DIR}/RPCTestUnit2.cdl")

COPY(CompilerExtraUnit ${COMPONENT_OBJ_DIR}/CompilerExtraUnit.so ${BIN_DIR})

install(FILES
    ${COMPONENT_OBJ_DIR}/CompilerExtraUnit.so
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