//=========================================================================
// Copyright (C) 2018 The C++ Component Model(COMO) Open Source Project
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

#ifndef __COMO_SECURITY_ACCESSCONTROLLER_H__
#define __COMO_SECURITY_ACCESSCONTROLLER_H__

#include "como.security.IPrivilegedAction.h"

namespace como {
namespace security {

class AccessController
{
public:
    static ECode DoPrivileged(
        /* [in] */ IPrivilegedAction* action,
        /* [out] */ AutoPtr<IInterface>& result);

private:
    AccessController();
};

}
}

#endif // __COMO_SECURITY_ACCESSCONTROLLER_H__
