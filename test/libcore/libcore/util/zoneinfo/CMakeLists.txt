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

project(ZoneInfoTest CXX)

set(LIBCORE_LIBCORE_UTIL_ZONEINFO_SRC_DIR ${LIBCORE_LIBCORE_UTIL_SRC_DIR}/zoneinfo)
set(LIBCORE_LIBCORE_UTIL_ZONEINFO_OBJ_DIR ${LIBCORE_LIBCORE_UTIL_OBJ_DIR}/zoneinfo)

include_directories(
    ./
    ${INC_DIR}
    ${LIBCORE_LIBCORE_UTIL_ZONEINFO_OBJ_DIR})

set(SOURCES
    ZoneInfoTestHelper.cpp
    main.cpp)

set(GENERATED_SOURCES
    ${LIBCORE_LIBCORE_UTIL_ZONEINFO_OBJ_DIR}/libcore.cpp)

IMPORT_LIBRARY(comort.so)
IMPORT_LIBRARY(libcore.so)
IMPORT_GTEST()

add_executable(testZoneInfo
    ${SOURCES}
    ${GENERATED_SOURCES})
target_link_libraries(testZoneInfo comort.so libcore.so ${GTEST_LIBS})
add_dependencies(testZoneInfo libtest)

add_custom_command(
    OUTPUT
        ${GENERATED_SOURCES}
    COMMAND
        "$ENV{CDLC}"
        -gen
        -mode-client
        -split
        -d ${LIBCORE_LIBCORE_UTIL_ZONEINFO_OBJ_DIR}
        -metadata-so "${BIN_DIR}/libcore.so")

COPY(testZoneInfo ${LIBCORE_LIBCORE_UTIL_ZONEINFO_OBJ_DIR}/testZoneInfo ${BIN_DIR})

install(FILES
    ${LIBCORE_LIBCORE_UTIL_ZONEINFO_OBJ_DIR}/testZoneInfo
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
