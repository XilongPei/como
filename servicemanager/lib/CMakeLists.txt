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

if("$ENV{PLATFORM}" STREQUAL "linux")
    add_subdirectory(linux)
elseif("$ENV{PLATFORM}" STREQUAL "android")
    #
    # use dbus(not Android's IBinder) always (?)
    #
    #add_subdirectory(android)
    if("$ENV{COMO_WITH_RPC}" STREQUAL "rpc")
        add_subdirectory(linux)
    endif()
elseif("$ENV{PLATFORM}" STREQUAL "openEuler")
    if("$ENV{COMO_WITH_RPC}" STREQUAL "rpc")
        add_subdirectory(linux)
    endif()
endif()
