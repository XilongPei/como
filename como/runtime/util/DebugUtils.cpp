//=========================================================================
// Copyright (C) 2024 The C++ Component Model(COMO) Open Source Project
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

/**
 * Copyright 2018 Dominik Liebler
 * https://gist.github.com/domnikl/af00cc154e3da1c5d965
 */

#include <stdio.h>
#include <string.h>
#include "DebugUtils.h"

namespace como {

void DebugUtils::HexDump(const char *desc, void *addr, int len)
{
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    // Output description if given.
    if (desc != nullptr) {
        printf ("%s:\n", desc);
    }

    // Process every byte in the data.
    for (int i = 0;  i < len;  i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0) {
                printf("  %s\n", buff);
            }

            // Output the offset.
            printf("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf(" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e)) {
            buff[i % 16] = '.';
        }
        else {
            buff[i % 16] = pc[i];
        }

        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        printf("   ");
        i++;
    }

    // And print the final ASCII bit.
    printf("  %s\n", buff);
}

} // namespace como


int main()
{
    const char *str = "// Pad out last line if not exactly 16 characters.// Pad "
    "out last line if not exactly 16 characters.// Pad out last line if not "
    "exactly 16 characters.// Pad out last line if not exactly 16 characters.";
    como::DebugUtils::HexDump("char *desc", (void*)str, strlen(str));
}