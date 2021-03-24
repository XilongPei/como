#=========================================================================
# Copyright (C) 2021 The C++ Component Model(COMO) Open Source Project
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

project(comoPybind CXX)

#
# g++ -O3 -Wall -shared -std=c++11 -fPIC `python3 -m pybind11 --includes` test_me.cpp -o test_me`python3-config --extension-suffix`
#

execute_process(COMMAND python3 -m pybind11 --includes OUTPUT_VARIABLE PYTHON_DEV_includes OUTPUT_STRIP_TRAILING_WHITESPACE)

STRING(REPLACE "-I" "" PYTHON_DEV_incs "${PYTHON_DEV_includes}")
STRING(REGEX REPLACE  "[ \t\r\n]+" ";" PYTHON_DEV_INCLUDE "${PYTHON_DEV_incs}" )
#message(FATAL_ERROR "=1== ${PYTHON_DEV_INCLUDE} ===")

add_compile_options(-fexceptions -frtti -std=c++14 -fPIC -shared)

include_directories(
    ./
    ${INC_DIR}
    ${PYTHON_DEV_INCLUDE}
    )

set(SOURCES
    como_pybind.cpp)


IMPORT_LIBRARY(comort.so)
#IMPORT_GTEST()


execute_process(COMMAND python3-config --extension-suffix OUTPUT_VARIABLE PYTHON_EXT_suffix OUTPUT_STRIP_TRAILING_WHITESPACE)

STRING(REPLACE "-I" "" PYTHON_EXT_suf "${PYTHON_EXT_suffix}")
STRING(REGEX REPLACE  "[ \t\r\n]+" "" PYTHON_EXT_SUFFIX "${PYTHON_EXT_suf}" )
set(COMO_PYBIND "como_pybind${PYTHON_EXT_SUFFIX}")
#message(FATAL_ERROR "=2== ${COMO_PYBIND} ===")

add_library(${COMO_PYBIND}
    SHARED
    ${SOURCES})

target_link_libraries(${COMO_PYBIND} comort.so ${GTEST_LIBS})

add_dependencies(gtest gtest_main)
