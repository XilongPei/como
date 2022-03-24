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

set(IDfromName_SRC_DIR ${RUNTIME_SRC_DIR}/IDfromName)
set(IDfromName_OBJ_DIR ${RUNTIME_OBJ_DIR}/IDfromName)

IMPORT_GTEST()

include_directories(
    ./
    ${INC_DIR}
    ${IDfromName_OBJ_DIR})

set(SOURCES
    CFoo.cpp
    CFooBar.cpp
    main.cpp)

set(GENERATED_SOURCES
    ${IDfromName_OBJ_DIR}/_CFoo.h
    ${IDfromName_OBJ_DIR}/_CFoo.cpp
    ${IDfromName_OBJ_DIR}/_Namespace1_Namespace2_Namespace3_CFooBar.h
    ${IDfromName_OBJ_DIR}/_Namespace1_Namespace2_Namespace3_CFooBar.cpp
    ${IDfromName_OBJ_DIR}/TestModule.h
    ${IDfromName_OBJ_DIR}/TestModulePub.cpp
    ${IDfromName_OBJ_DIR}/MetadataWrapper.cpp)

add_custom_command(
    OUTPUT
        ${GENERATED_SOURCES}
    COMMAND
        "$ENV{CDLC}"
        -gen
        -mode-component
        -i "${IDfromName_SRC_DIR}"
        -d ${IDfromName_OBJ_DIR}
        -c "${IDfromName_SRC_DIR}/Module.cdl")

IMPORT_LIBRARY(comort.so)

add_executable(testIDfromNamer
    ${SOURCES}
    ${GENERATED_SOURCES})
target_link_libraries(testIDfromNamer comort.so ${GTEST_LIBS})
add_dependencies(testIDfromNamer comort)
