#=========================================================================
# Copyright (C) 2022 The C++ Component Model(COMO) Open Source Project
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

project(UtilTest CXX)

set(UTIL_SRC_DIR ${RUNTIME_SRC_DIR}/util)
set(UTIL_OBJ_DIR ${RUNTIME_OBJ_DIR}/util)

include_directories(
    ./
    ${INC_DIR}
    ${UTIL_OBJ_DIR})

set(SOURCES
    main.cpp)

IMPORT_LIBRARY(comort.so)
IMPORT_GTEST()

add_executable(testUtil
    ${SOURCES})
target_link_libraries(testUtil comort.so ${GTEST_LIBS})
add_dependencies(testUtil comort gtest gtest_main)

COPY(testUtil ${UTIL_OBJ_DIR}/testUtil ${BIN_DIR})

install(FILES
    ${UTIL_OBJ_DIR}/testUtil
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