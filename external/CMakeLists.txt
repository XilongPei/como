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

set(EXTERNAL_DIR ${PROJECT_DIR}/external)

if($ENV{PLATFORM} STREQUAL "linux")
add_subdirectory(boringssl)
endif()
add_subdirectory(googletest)
add_subdirectory(fdlibm)
add_subdirectory(EasyLogger/export_como)

# The intermediate files are generated with make and then incorporated into the
# COMO compilation environment
add_custom_target(build_libcrc ALL
    COMMAND make
    WORKING_DIRECTORY ${EXTERNAL_DIR}/libcrc
    COMMENT "libcrc makefile")
add_library(libcrc STATIC IMPORTED)
add_dependencies(libcrc build_libcrc)
add_subdirectory(libcrc/export_como)
