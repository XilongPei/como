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

set(RPCDBUS_SRC_DIR ${RPC_SRC_DIR}/dbus)

if($ENV{ARCH} MATCHES "(x64)|(x32)")
    set(USR_LIB_INCLUDE "/usr/lib/x86_64-linux-gnu/dbus-1.0/include")
elseif($ENV{PLATFORM} MATCHES "openEuler")
    #This parameter is not affected by --sysroot=$CMAKE_SYSROOT
    set(USR_LIB_INCLUDE "${CMAKE_SYSROOT}/usr/lib64/dbus-1.0/include")
else()
    set(USR_LIB_INCLUDE "")
endif()

include_directories(
    ./
    ../
    ../../
    ../../type
    ../../util
    /usr/include/dbus-1.0
    ${USR_LIB_INCLUDE}
    ${INC_DIR}
)

set(SOURCES
    CDBusChannel.cpp
    CDBusChannelFactory.cpp
    CDBusParcel.cpp
    InterfacePack.cpp
    ThreadPoolExecutor.cpp
)

add_library(rpc-dbus STATIC
    ${SOURCES})
add_dependencies(rpc-dbus cdl)
