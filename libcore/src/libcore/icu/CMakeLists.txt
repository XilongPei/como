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

set(ICU_DIR ${LIBCORE_LIBCORE_SRC_DIR}/icu)

if("$ENV{PLATFORM}" STREQUAL "openEuler")
    include_directories(
        ${EXTERNAL_DIR}/../prebuilt/icu4c/icu4c/source/common
        ${EXTERNAL_DIR}/../prebuilt/icu4c/icu4c/source/i18n
    )
endif()

include_directories(
    ./
    ../
    ../../../inc
    ${INC_DIR}
    ${LIBCORE_OBJ_DIR})

set(SOURCES
    ICU.cpp
    LocaleData.cpp
    NativeConverter.cpp
    NativeDecimalFormat.cpp
    TimeZoneNames.cpp
    UStringEnumeration.cpp)

add_library(libcore_icu STATIC
    ${SOURCES})
add_dependencies(libcore_icu build_libcore)