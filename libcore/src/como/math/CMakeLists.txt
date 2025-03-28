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

set(MATH_DIR ${LIBCORE_COMO_SRC_DIR}/math)

if($ENV{PLATFORM} STREQUAL "linux")
    include_directories(
        ./
        ../
        ../../../inc
        ${EXTERNAL_DIR}/boringssl/inc
        ${INC_DIR}
        ${LIBCORE_OBJ_DIR})
elseif($ENV{PLATFORM} STREQUAL "openEuler")
    include_directories(
        ./
        ../
        ../../../inc
        ${EXTERNAL_DIR}/boringssl/inc
        ${INC_DIR}
        ${LIBCORE_OBJ_DIR})
elseif($ENV{PLATFORM} STREQUAL "android")
    include_directories(
        ./
        ../
        ../../../inc
        ${INC_DIR}
        ${LIBCORE_OBJ_DIR})
endif()

if("$ENV{COMO_WITH_MATH_BIG_NUMBER}" STREQUAL "bn")
    set(MATH_BIG_NUMBER_SRCS
        BigDecimal.cpp
        BigDecimalFactory.cpp
        BigInt.cpp
        BigInteger.cpp
        BigIntegerFactory.cpp
        math.cpp)
else()
    set(MATH_BIG_NUMBER_SRCS )
endif()

set(SOURCES
    ${MATH_BIG_NUMBER_SRCS}
    BitLevel.cpp
    Conversion.cpp
    Division.cpp
    Logical.cpp
    MathContext.cpp
    Multiplication.cpp
    Primality.cpp)

add_library(como_math STATIC
    ${SOURCES})
add_dependencies(como_math build_libcore)

