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

#include <stdio.h>
#include <string.h>
#include "comoobj.h"
#include "util/comolog.h"
#include "DebugUtils.h"

namespace como {

int DebugUtils::HexDump(char *bufStr, int bufSize, void *addr, int len)
{
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;
    int  posBuf;
    int  i;

    posBuf = 0;
    bufSize--;          // for tail '\0'

    // Process every byte in the data.
    for (i = 0;  i < len;  i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0) {
                if (posBuf + 18 < bufSize) {
                // buff Maximum 16 characters, and 2 blanks
                    posBuf += sprintf(&bufStr[posBuf], "  %s\n", buff);
                }
                else {
                    break;
                }
            }

            // Output the offset.
            if (posBuf + 7 < bufSize) {
            // %04x, and 3 blanks
                posBuf += sprintf(&bufStr[posBuf], "  %04x ", i);
            }
            else {
                break;
            }
        }

        // Now the hex code for the specific character.
        posBuf += sprintf(&bufStr[posBuf], " %02x", pc[i]);

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
        if (posBuf + 3 < bufSize) {
            posBuf += sprintf(&bufStr[posBuf], "   ");
            i++;
        }
        else {
            break;
        }
    }

    // And print the final ASCII bit.
    if (posBuf + strlen((const char*)buff) + 2 < bufSize) {
        posBuf += sprintf(&bufStr[posBuf], "  %s\n", buff);
    }

    return posBuf;
}

void DebugUtils::AutoPtrInspect(void *autoPtr, void *AuptrMPtr, char *logTAG,
                                const char *file, const char *func, int line)
{
    if (nullptr == logTAG) {
        logTAG = (char*)"";
    }

    Logger::D(logTAG, "AutoPtr:%p, AutoPtr->mPtr:%p, File:%s, Functin:%s, Line:%d",
                   autoPtr, AuptrMPtr, file, func, line);
}

void DebugUtils::AutoPtrRefBaseInspect(void *autoPtr, IInterface *intf,
               const char *logTAG, const char *file, const char *func, int line)
{
    if (nullptr == logTAG) {
        logTAG = (char*)"";
    }

    Logger::D(logTAG, "AutoPtr:%p, RefBase:%p, File:%s, Functin:%s, Line:%d",
                     autoPtr, Object::IntfToRefBasePtr(intf), file, func, line);
}

} // namespace como

#if 0
int main()
{
    const char *str = "// Pad out last line if not exactly 16 characters.// Pad "
    "out last line if not exactly 16 characters.// Pad out last line if not "
    "exactly 16 characters. \t\n\r// Pad out last line if not exactly 16 characters.";

    char bufStr[4096];
    int i = como::DebugUtils::HexDump(bufStr, 4096, (void*)str, strlen(str));
    printf("%d\n%s", i, bufStr);
}
#endif
