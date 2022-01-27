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

project(ComoContextTest CXX)

set(LIBCORE_SRC_DIR ${TEST_SRC_DIR}/libcore)
set(LIBCORE_OBJ_DIR ${TEST_OBJ_DIR}/libcore)
set(LIBCORE_COMO_SRC_DIR ${LIBCORE_SRC_DIR}/como)
set(LIBCORE_COMO_OBJ_DIR ${LIBCORE_OBJ_DIR}/como)
set(LIBCORE_COMO_MATH_SRC_DIR ${LIBCORE_COMO_SRC_DIR}/math)
set(LIBCORE_COMO_MATH_OBJ_DIR ${LIBCORE_COMO_OBJ_DIR}/math)
set(LIBCORE_COMO_MATH_BIGINTEGER_SRC_DIR ${LIBCORE_COMO_MATH_SRC_DIR}/biginteger)
set(LIBCORE_COMO_MATH_BIGINTEGER_OBJ_DIR ${LIBCORE_COMO_MATH_OBJ_DIR}/biginteger)

#message(FATAL_ERROR "==${LIBCORE_SRC_DIR}==")

set(ComoContext_SRC_DIR ${RUNTIME_SRC_DIR}/ComoContext)
set(ComoContext_OBJ_DIR ${RUNTIME_OBJ_DIR}/ComoContext)

include_directories(
    ./
    ${INC_DIR}
    ${OBJ_DIR}
    ${LIBCORE_COMO_MATH_BIGINTEGER_OBJ_DIR}
)

set(SOURCES
    main.cpp)

IMPORT_LIBRARY(comort.so)
IMPORT_LIBRARY(libcore.so)
IMPORT_GTEST()

add_executable(testComoContext
    ${SOURCES})
target_link_libraries(testComoContext comort.so libcore.so ${GTEST_LIBS})
add_dependencies(testComoContext comort libcore gtest gtest_main)

COPY(testComoContext ${ComoContext_OBJ_DIR}/testComoContext ${BIN_DIR})

install(FILES
    ${ComoContext_OBJ_DIR}/testComoContext
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