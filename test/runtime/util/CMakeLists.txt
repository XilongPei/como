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
    testMempool.cpp
    ${PROJECT_DIR}/como/runtime/util/MemPool.cpp
)

set(SOURCES1
    testMac.cpp
    ${PROJECT_DIR}/como/runtime/util/mac.cpp
)

set(SOURCES2
    testVerifiedU64Pointer.cpp
    ${PROJECT_DIR}/como/runtime/util/VerifiedU64Pointer.cpp
)

set(SOURCES3
    testMistring.cpp
    ${PROJECT_DIR}/como/runtime/util/mistring.cpp
)

set(SOURCES4
    testInt64.cpp
)

set(SOURCES5
    testThreadStack.cpp
    ${PROJECT_DIR}/como/runtime/util/ThreadStack.cpp
)

set(SOURCES6
    testMempool1.cpp
    ${PROJECT_DIR}/como/runtime/util/MemPool.cpp
)

set(SOURCES7
    testMistring1.cpp
    ${PROJECT_DIR}/como/runtime/util/mistring.cpp
)

set(SOURCES8
    testMutex.cpp
    ${PROJECT_DIR}/como/runtime/util/mutex.cpp
)

set(SOURCES9
    testCircleBuffer.cpp
    ${PROJECT_DIR}/como/runtime/util/mutex.cpp
)

set(SOURCES10
    testLamportClock.cpp
)

IMPORT_LIBRARY(comort.so)
IMPORT_GTEST()

add_executable(testUtil
    ${SOURCES})

add_executable(testMac
    ${SOURCES1})

add_executable(testVerifiedU64Pointer
    ${SOURCES2})

add_executable(testMistring
    ${SOURCES3})

add_executable(testInt64
    ${SOURCES4})

add_executable(testThreadStack
    ${SOURCES5})

add_executable(testMempool1
    ${SOURCES6})

add_executable(testMistring1
    ${SOURCES7})

add_executable(testMutex
    ${SOURCES8})

add_executable(testCircleBuffer
    ${SOURCES9})

add_executable(testLamportClock
    ${SOURCES10})

target_link_libraries(testMac comort.so ${GTEST_LIBS})
add_dependencies(testMac comort gtest gtest_main)

target_link_libraries(testUtil comort.so ${GTEST_LIBS})
add_dependencies(testUtil comort gtest gtest_main)

target_link_libraries(testVerifiedU64Pointer comort.so ${GTEST_LIBS})
add_dependencies(testVerifiedU64Pointer comort gtest gtest_main)

target_link_libraries(testMistring comort.so ${GTEST_LIBS})
add_dependencies(testMistring comort gtest gtest_main)

target_link_libraries(testInt64 comort.so ${GTEST_LIBS})
add_dependencies(testInt64 comort gtest gtest_main)

target_link_libraries(testThreadStack comort.so ${GTEST_LIBS})
add_dependencies(testThreadStack comort gtest gtest_main)

target_link_libraries(testMempool1 comort.so ${GTEST_LIBS})
add_dependencies(testMempool1 comort gtest gtest_main)

target_link_libraries(testMistring1 comort.so ${GTEST_LIBS})
add_dependencies(testMistring1 comort gtest gtest_main)

target_link_libraries(testMutex comort.so ${GTEST_LIBS})
add_dependencies(testMutex comort gtest gtest_main)

target_link_libraries(testCircleBuffer comort.so ${GTEST_LIBS})
target_include_directories(testCircleBuffer 
    PRIVATE ${PROJECT_DIR}/como/runtime/monitor
    PRIVATE ${PROJECT_DIR}/como/runtime/util)
add_dependencies(testCircleBuffer comort gtest gtest_main)

target_link_libraries(testLamportClock comort.so ${GTEST_LIBS})
add_dependencies(testLamportClock comort gtest gtest_main)

COPY(testUtil ${UTIL_OBJ_DIR}/testUtil ${BIN_DIR})
COPY(testMac ${UTIL_OBJ_DIR}/testMac ${BIN_DIR})
COPY(testVerifiedU64Pointer ${UTIL_OBJ_DIR}/testVerifiedU64Pointer ${BIN_DIR})
COPY(testMistring ${UTIL_OBJ_DIR}/testMistring ${BIN_DIR})
COPY(testInt64 ${UTIL_OBJ_DIR}/testInt64 ${BIN_DIR})
COPY(testThreadStack ${UTIL_OBJ_DIR}/testThreadStack ${BIN_DIR})
COPY(testMempool1 ${UTIL_OBJ_DIR}/testMempool1 ${BIN_DIR})
COPY(testMistring1 ${UTIL_OBJ_DIR}/testMistring1 ${BIN_DIR})
COPY(testMutex ${UTIL_OBJ_DIR}/testMutex ${BIN_DIR})
COPY(testCircleBuffer ${UTIL_OBJ_DIR}/testCircleBuffer ${BIN_DIR})
COPY(testLamportClock ${UTIL_OBJ_DIR}/testLamportClock ${BIN_DIR})

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
