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

#include <string.h>
#include "RuleCheckUtils.h"

namespace cdlc {

std::string RuleCheckUtils::GetFileNameFromPath(const std::string& fullPath)
{
    size_t pos = fullPath.find_last_of("/\\");
    if (pos == std::string::npos) {
        return fullPath;
    }
    return fullPath.substr(pos + 1);
}

bool RuleCheckUtils::CheckModuleName(const char *strComoMoudle, const char *uri)
{
    std::string tmpStr = GetFileNameFromPath(uri);

    const char *strFilePath = tmpStr.c_str();
    int lenComoMoudle = strlen(strComoMoudle);
    int lenFilePath = strlen(strFilePath);
    bool stdComoComponentName = true;

    if ((strncmp(strComoMoudle, strFilePath, lenComoMoudle) == 0) &&
                                                (lenFilePath > lenComoMoudle)) {
        if ('.' == strFilePath[lenComoMoudle]) {
            for (int i = lenComoMoudle + 1;  i < lenFilePath;  i++) {
                if (('/' == strFilePath[i]) || ('\\' == strFilePath[i])) {
                    stdComoComponentName = false;
                    break;
                }
            }
        }
        else {
            stdComoComponentName = false;
        }
    }
    else {
        stdComoComponentName = false;
    }

    return stdComoComponentName;
}

} // namespace cdlc
