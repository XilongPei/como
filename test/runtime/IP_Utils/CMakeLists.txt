#=========================================================================
# Copyright (C) 2025 The C++ Component Model(COMO) Open Source Project
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

project(IP_UtilsTest CXX)

set(IP_Utils_SRC_DIR ${RUNTIME_SRC_DIR}/IP_Utils)
set(IP_Utils_OBJ_DIR ${RUNTIME_OBJ_DIR}/IP_Utils)

include_directories(
    ./
    ${INC_DIR}
    ${OBJ_DIR})

set(SOURCES
    main.cpp)

IMPORT_LIBRARY(comort.so)
IMPORT_GTEST()

add_executable(testIP_Utils
    ${SOURCES})
target_link_libraries(testIP_Utils comort.so ${GTEST_LIBS})
add_dependencies(testIP_Utils comort gtest gtest_main)

COPY(testIP_Utils ${IP_Utils_OBJ_DIR}/testIP_Utils ${BIN_DIR})

install(FILES
    ${IP_Utils_OBJ_DIR}/testIP_Utils
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