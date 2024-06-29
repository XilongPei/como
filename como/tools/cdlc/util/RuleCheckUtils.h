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

#ifndef __CDLC_RULECHECKUTILS_H__
#define __CDLC_RULECHECKUTILS_H__

#include <string>

namespace cdlc {

class RuleCheckUtils
{
public:
    static std::string GetFileNameFromPath(const std::string& fullPath);
    static bool CheckModuleName(const char *strComoMoudle, const char *uri);
};

} // namespace cdlc

#endif // __CDLC_RULECHECKUTILS_H__
