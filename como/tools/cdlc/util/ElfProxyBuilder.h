//=========================================================================
// Copyright (C) 2026 The C++ Component Model(COMO) Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//=========================================================================


#ifndef __ELF_PROXY_BUILDER_H__
#define __ELF_PROXY_BUILDER_H__

#include <string>

namespace cdlc {

enum class TargetArch {
    kX86_64,
    kAARCH64,
    kRISCV64,
};

// Build an ELF proxy that forwards all global function symbols from origin_so.
// The output is a position-independent shared object with:
//   - Minimal DT_HASH (3-bucket)
//   - .note.gnu.build-id (SHA1 of origin path + symbol list)
//   - Custom .session section
// Returns true on success.
bool BuildElfProxy(const std::string& origin_so,
                   const std::string& output_so);

}  // namespace cdlc

#endif  // __ELF_PROXY_BUILDER_H__
