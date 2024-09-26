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
#include <stdlib.h>
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

    Logger::D(logTAG, "AutoPtr: %p , AutoPtr->mPtr: %p , File: %s , "
                      "Function: %s , Line: %d",
                   autoPtr, AuptrMPtr, file, func, line);
}

void DebugUtils::AutoPtrRefBaseInspect(void *autoPtr, IInterface *intf,
               const char *logTAG, const char *file, const char *func, int line)
{
    if (nullptr == logTAG) {
        logTAG = (char*)"";
    }

    Logger::D(logTAG, "AutoPtr: %p , RefBase: %p , File: %s , "
                      "Function: %s , Line: %d",
                     autoPtr, Object::IntfToRefBasePtr(intf), file, func, line);
}

static void ParseLine(char *line, LinuxMemInfo *memInfo)
{
    char key[32];
    long value;

    // read a "key: value kB" style line
    sscanf(line, "%31s %ld", key, &value);

    // remove ':'
    key[strlen(key) - 1] = '\0';

    if (strcmp(key, "MemTotal") == 0) {
        memInfo->MemTotal = value;
    }
    else if (strcmp(key, "MemFree") == 0) {
        memInfo->MemFree = value;
    }
    else if (strcmp(key, "MemAvailable") == 0) {
        memInfo->MemAvailable = value;
    }
    else if (strcmp(key, "Buffers") == 0) {
        memInfo->Buffers = value;
    }
    else if (strcmp(key, "Cached") == 0) {
        memInfo->Cached = value;
    }
    else if (strcmp(key, "SwapTotal") == 0) {
        memInfo->SwapTotal = value;
    }
    else if (strcmp(key, "SwapFree") == 0) {
        memInfo->SwapFree = value;
    }
}

/**
 * GetMemoryInfo
 */
void DebugUtils::GetMemoryInfo(LinuxMemInfo& memInfo)
{
    FILE* file = fopen("/proc/meminfo", "r");

    if (nullptr == file) {
        memInfo.MemTotal = 0;
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        ParseLine(line, &memInfo);
    }

    fclose(file);
}

} // namespace como

#if 0
int main() {
    como::LinuxMemInfo memInfo;
    como::DebugUtils::GetMemoryInfo(memInfo);

    printf("MemTotal:    %ld kB\n", memInfo.MemTotal);
    printf("MemFree:     %ld kB\n", memInfo.MemFree);
    printf("MemAvailable: %ld kB\n", memInfo.MemAvailable);
    printf("Buffers:     %ld kB\n", memInfo.Buffers);
    printf("Cached:      %ld kB\n", memInfo.Cached);
    printf("SwapTotal:   %ld kB\n", memInfo.SwapTotal);
    printf("SwapFree:    %ld kB\n", memInfo.SwapFree);

    return 0;
}
#endif

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
