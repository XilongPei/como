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

set(RUNTIME_SRC_DIR ${TEST_SRC_DIR}/runtime)
set(RUNTIME_OBJ_DIR ${TEST_OBJ_DIR}/runtime)

add_subdirectory(array)
add_subdirectory(autoptr)
add_subdirectory(char)
add_subdirectory(comostring)
add_subdirectory(localstaticvariable)
add_subdirectory(macro)
# testMutex tests mutex in comort, so running testMutex needs export
# mutex.h and related methods from comort.so
#add_subdirectory(mutex)
add_subdirectory(nestedinterface)
add_subdirectory(outreferencetype)
add_subdirectory(reflection)

if("$ENV{COMO_WITH_RPC}" STREQUAL "rpc")
    add_subdirectory(rpc)
    add_subdirectory(rpc_extra)
endif()

add_subdirectory(ComoConfig)
add_subdirectory(IDfromName)
add_subdirectory(util)
add_subdirectory(IP_Utils)

add_subdirectory(ObjectObserver)
add_subdirectory(ObjectObserver_extra)

if("$ENV{COMO_FUNCTION_SAFETY}" STREQUAL "enable")
    add_subdirectory(FunctionSafetyProgram)
    add_subdirectory(FunctionSafetyProgram_extra)
endif()
