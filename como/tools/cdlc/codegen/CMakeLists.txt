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

set(CODEGEN_SRC_DIR ${CDLC_SRC_DIR}/codegen)

include_directories(
    ${CDLC_SRC_DIR}
    ${COMO_SRC_DIR})

set(SOURCES
    CodeGenerator.cpp)

add_library(cdlc_codegen STATIC
    ${SOURCES})
