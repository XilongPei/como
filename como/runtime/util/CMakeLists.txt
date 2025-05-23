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

set(UTIL_SRC_DIR ${RUNTIME_SRC_DIR}/util)

include_directories(
    ./
    ../
    ../type
    ../monitor
    ${INC_DIR}
    ${EXTERNAL_DIR}/EasyLogger/easylogger/inc
    ${EXTERNAL_DIR}/libcrc/include
    ${EXTERNAL_DIR}/cityhash/export_como
    ${EXTERNAL_DIR}/inih
    ${EXTERNAL_DIR}/circularbuffer
    ${EXTERNAL_DIR}/lwrb
)

if("$ENV{COMO_FUNCTION_SAFETY}" STREQUAL "enable")
    set(ComoFunctionSafetyObject
        ComoFunctionSafetyObject.cpp
    )
else()
    set(ComoFunctionSafetyObject)
endif()

if("$ENV{COMO_WITH_RPC}" STREQUAL "rpc")
    set(WITH_CCheckpoint CCheckpoint.cpp)
else()
    set(WITH_CCheckpoint)
endif()

set(SOURCES
    comoclsobj.cpp
    comolog.cpp
    comoobj.cpp
    comolock.cpp
    comoref.cpp
    ${ComoFunctionSafetyObject}
    mutex.cpp
    ${EXTERNAL_DIR}/cityhash/export_como/cityhash.cc
    mistring.cpp
    mac.cpp
    ${WITH_CCheckpoint}
    MemPool.cpp
    ThreadStack.cpp
    ${EXTERNAL_DIR}/inih/ini.c
    StrToX_Hashmap.cpp
    DebugUtils.cpp
    talloc.cpp
    IP_Utils.cpp
)

set(DEPLOY_HEADERS
    ${UTIL_SRC_DIR}/comosp.h
    ${UTIL_SRC_DIR}/comoclsobj.h
    ${UTIL_SRC_DIR}/comolog.h
    ${UTIL_SRC_DIR}/comoobj.h
    ${UTIL_SRC_DIR}/comoref.h
    ${UTIL_SRC_DIR}/comolock.h
    ${UTIL_SRC_DIR}/mutex.h
    ${UTIL_SRC_DIR}/mistring.h
    ${UTIL_SRC_DIR}/MemPool.h
    ${UTIL_SRC_DIR}/mac.h
    ${UTIL_SRC_DIR}/int64.h
    ${UTIL_SRC_DIR}/ThreadStack.h
    ${UTIL_SRC_DIR}/lamport_clock.h
    ${UTIL_SRC_DIR}/hashmap.h
    ${UTIL_SRC_DIR}/hashmapCache.h
    ${UTIL_SRC_DIR}/StrToX_Hashmap.h
    ${UTIL_SRC_DIR}/DebugUtils.h
    ${UTIL_SRC_DIR}/TimespecUtils.h
    ${UTIL_SRC_DIR}/talloc.h
    ${UTIL_SRC_DIR}/IP_Utils.h
)

add_custom_target(rebuild_if_header_changes
    DEPENDS ${DEPLOY_HEADERS}
)

add_library(util STATIC
    ${SOURCES})
add_dependencies(util cdl rebuild_if_header_changes)

target_link_libraries(util
    $ENV{OUT_PATH}/external/EasyLogger/export_como/libEasyLogger.a
    $ENV{OUT_PATH}/external/libcrc/export_como/libLibCRC.a
)

COPY(util ${UTIL_SRC_DIR}/comosp.h ${INC_DIR})
COPY(util ${UTIL_SRC_DIR}/comoclsobj.h ${INC_DIR})
COPY(util ${UTIL_SRC_DIR}/comolog.h ${INC_DIR})
COPY(util ${UTIL_SRC_DIR}/comoobj.h ${INC_DIR})
COPY(util ${UTIL_SRC_DIR}/comoref.h ${INC_DIR})
COPY(util ${UTIL_SRC_DIR}/comolock.h ${INC_DIR})
COPY(util ${UTIL_SRC_DIR}/mutex.h ${INC_DIR})
COPY(util ${UTIL_SRC_DIR}/ComoFunctionSafetyObject.h ${INC_DIR})
COPY(util ${UTIL_SRC_DIR}/mistring.h ${INC_DIR})
COPY(util ${UTIL_SRC_DIR}/VerifiedU64Pointer.h ${INC_DIR})
COPY(util ${UTIL_SRC_DIR}/MemPool.h ${INC_DIR})
COPY(util ${UTIL_SRC_DIR}/mac.h ${INC_DIR})
COPY(util ${UTIL_SRC_DIR}/int64.h ${INC_DIR})
COPY(util ${UTIL_SRC_DIR}/ThreadStack.h ${INC_DIR})
COPY(util ${UTIL_SRC_DIR}/lamport_clock.h ${INC_DIR})
COPY(util ${UTIL_SRC_DIR}/hashmap.h ${INC_DIR})
COPY(util ${UTIL_SRC_DIR}/hashmapCache.h ${INC_DIR})
COPY(util ${UTIL_SRC_DIR}/StrToX_Hashmap.h ${INC_DIR})
COPY(util ${UTIL_SRC_DIR}/DebugUtils.h ${INC_DIR})
COPY(util ${UTIL_SRC_DIR}/TimespecUtils.h ${INC_DIR})
COPY(util ${UTIL_SRC_DIR}/talloc.h ${INC_DIR})
COPY(util ${UTIL_SRC_DIR}/IP_Utils.h ${INC_DIR})

# `install` command will generate installation rules. `make install` will use these rules.
install(FILES
    ${DEPLOY_HEADERS}
    DESTINATION ${INC_DIR})
