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

project(test-rpc CXX)

set(RPC_SRC_DIR ${RUNTIME_SRC_DIR}/rpc)
set(RPC_OBJ_DIR ${RUNTIME_OBJ_DIR}/rpc)

add_subdirectory(component)
add_subdirectory(client)
add_subdirectory(service)

if("$ENV{RPC_OVER_ZeroMQ_SUPPORT_Enable}" STREQUAL "enable")
    add_subdirectory(clientZMQ)
    add_subdirectory(serviceZMQ)
    add_subdirectory(testZMQ)
    add_subdirectory(clientMonitor)
    add_subdirectory(RedundantComputing)
endif()
