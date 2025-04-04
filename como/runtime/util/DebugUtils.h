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

#ifndef __DEBUGUTILS_H__
#define __DEBUGUTILS_H__

namespace como {

typedef struct tagLinuxMemInfo {
    long memTotal;
    long memFree;
    long memAvailable;
    long buffers;
    long cached;
    long swapTotal;
    long swapFree;
} LinuxMemInfo;

class COM_PUBLIC DebugUtils
{
public:
    static int HexDump(char *bufStr, int bufSize, void *addr, int len);

    /**
     * usage, for example:
     *     AutoPtr<IClassLoader> CBootClassLoader::sInstance = ......
     *     AutoPtrINSPECT(&CBootClassLoader::sInstance, (char*)"test");
     */
    #define AutoPtrINSPECT(autoPtr, logTAG)                                     \
           DebugUtils::AutoPtrInspect((void*)autoPtr, (void*)*autoPtr, logTAG,  \
                                        __FILE__, __PRETTY_FUNCTION__, __LINE__)

    static void AutoPtrInspect(void *autoPtr, void *AuptrMPtr, char *logTAG,
                                  const char *file, const char *func, int line);

    /**
     * usage, for example:
     *     AutoPtr<IClassLoader> CBootClassLoader::sInstance = ......
     *     AutoPtrRefBaseINSPECT(&CBootClassLoader::sInstance, (char*)"test");
     */
    #define AutoPtrRefBaseINSPECT(autoPtr, logTAG)                              \
           DebugUtils::AutoPtrRefBaseInspect((void*)autoPtr, autoPtr, logTAG,   \
                                        __FILE__, __PRETTY_FUNCTION__, __LINE__)
    static void AutoPtrRefBaseInspect(void *autoPtr, IInterface *intf,
              const char *logTAG, const char *file, const char *func, int line);

    static void GetMemoryInfo(LinuxMemInfo& memInfo);
    static float GetCpuUsage();
    static float GetDiskUsage(const char *disk);

}; // class DebugUtils

} // namespace como

#endif // __DEBUGUTILS_H__
