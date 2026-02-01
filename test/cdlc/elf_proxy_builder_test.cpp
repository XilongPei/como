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

#include "../../como/tools/cdlc/util/ElfProxyBuilder.h"
#include <cstring>
#include <cstdio>

int main(
    /* [in] */ int argc,
    /* [in] */ char* argv[])
{
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <origin.so> <output.so> [session_data]\n", argv[0]);
        return 1;
    }

    const char* origin_so = argv[1];
    const char* output_so = argv[2];

    // Session data
    const char* session_msg = "COMO proxy session";
    const void* session_data = session_msg;
    size_t session_size = strlen(session_msg) + 1;

    if (argc >= 4) {
        session_data = argv[3];
        session_size = strlen(argv[3]) + 1;
    }

    printf("Building ELF proxy:\n");
    printf("  Origin: %s\n", origin_so);
    printf("  Output: %s\n", output_so);
    printf("  Session: %s\n", static_cast<const char*>(session_data));

    bool result = cdlc::BuildElfProxy(origin_so, output_so, session_data, session_size);

    if (result) {
        printf("Success: ELF proxy built successfully\n");
        return 0;
    }
    else {
        fprintf(stderr, "Error: Failed to build ELF proxy\n");
        return 1;
    }
}
