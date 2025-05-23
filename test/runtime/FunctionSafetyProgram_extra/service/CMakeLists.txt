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

project(FunctionSafetyProgram2-service CXX)

set(SERVICE_SRC_DIR ${FunctionSafetyProgram_EX_SRC_DIR}/service)
set(SERVICE_OBJ_DIR ${FunctionSafetyProgram_EX_OBJ_DIR}/service)

include_directories(
    ./
    ${INC_DIR}
    ${SERVICE_OBJ_DIR})

set(SOURCES
    main.cpp)

set(GENERATED_SOURCES
    ${SERVICE_OBJ_DIR}/ExpiresTestUnit.cpp)

IMPORT_LIBRARY(comort.so)
IMPORT_LIBRARY(libservicemanager.so)

add_executable(testFSP2Srv
    ${SOURCES}
    ${GENERATED_SOURCES})
target_link_libraries(testFSP2Srv comort.so libservicemanager.so)
add_dependencies(testFSP2Srv ExpiresTestUnit libservicemanager)

add_custom_command(
    OUTPUT
        ${GENERATED_SOURCES}
    COMMAND
        "$ENV{CDLC}"
        -gen
        -mode-client
        -d ${SERVICE_OBJ_DIR}
        -metadata-so "${BIN_DIR}/ExpiresTestUnit.so")

COPY(testFSP2Srv ${SERVICE_OBJ_DIR}/testFSP2Srv ${BIN_DIR})

install(FILES
    ${SERVICE_OBJ_DIR}/testFSP2Srv
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