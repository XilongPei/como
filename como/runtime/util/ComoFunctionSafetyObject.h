//=========================================================================
// Copyright (C) 2021 The C++ Component Model(COMO) Open Source Project
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

#ifndef __ComoFunctionSafetyObject_H__
#define __ComoFunctionSafetyObject_H__

#include <time.h>
#include "comotypes.h"

namespace como {

class COM_PUBLIC ComoFunctionSafetyObject
{
public:
    ComoFunctionSafetyObject();

    Long GetExpires();

    ECode SetExpires(
        /* [in] */ Long expires);

private:
    Long mChecksum;
    Long mExpires;
    struct timespec mBirthTime;         // CLOCK_REALTIME
};

} // namespace como

#endif // __ComoFunctionSafetyObject_H__
