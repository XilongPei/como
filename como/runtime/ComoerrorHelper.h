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

#ifndef __COMOERRORHELPER_h__
#define __COMOERRORHELPER_h__

#include "comotypes.h"

namespace como {

typedef struct tagComoEcError {
    ECode ec;
    const char *info;
} ComoEcError;

class COM_PUBLIC ComoerrorHelper {
public:
    static const char *GetEcErrorInfo(ECode ec);
    static int RegisterEcErrors(ComoEcError *ecErrors, int num);
};

} // namespace como

#endif // __COMOERRORHELPER_h__
