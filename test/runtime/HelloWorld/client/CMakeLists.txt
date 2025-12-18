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

project(HelloWorld-client CXX)

set(CLIENT_SRC_DIR ${HelloWorld_SRC_DIR}/client)
set(CLIENT_OBJ_DIR ${HelloWorld_OBJ_DIR}/client)

IMPORT_GTEST()

include_directories(
    ./
    ${INC_DIR}
    ${CLIENT_OBJ_DIR}
)

set(SOURCES
    main.cpp
)

IMPORT_COMO_COMPONENT("${BIN_DIR}/HelloWorld_demo.so" ${HelloWorld_OBJ_DIR})

IMPORT_LIBRARY(comort.so)

add_executable(testHelloWorld
    ${SOURCES}
    ${GENERATED_SOURCES}
)
target_link_libraries(testHelloWorld comort.so ${GTEST_LIBS})
add_dependencies(testHelloWorld HelloWorld_demo gtest gtest_main)

COPY(testHelloWorld ${CLIENT_OBJ_DIR}/testHelloWorld ${BIN_DIR})

install(FILES
    ${CLIENT_OBJ_DIR}/testHelloWorld
    DESTINATION ${BIN_DIR}
    PERMISSIONS
        OWNER_READ
        OWNER_WRITE
        OWNER_EXECUTE
        GROUP_READ
        GROUP_WRITE
        GROUP_EXECUTE
        WORLD_READ
        WORLD_EXECUTE
)
