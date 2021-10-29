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

//
// when building environment has $ENV{COMO_FUNCTION_SAFETY}, it will define
// -DCOMO_FUNCTION_SAFETY=$ENV{COMO_FUNCTION_SAFETY}

#ifdef COMO_FUNCTION_SAFETY
#endif

#include <time.h>
#include "comoobj.h"
#include "ComoFunctionSafetyObject.h"

namespace como {

ComoFunctionSafetyObject::ComoFunctionSafetyObject()
{
    clock_gettime(CLOCK_REALTIME, &mBirthTime);
}

Long ComoFunctionSafetyObject::GetExpires()
{
    return mExpires;
}

ECode ComoFunctionSafetyObject::SetExpires(
    /* [out] */ Long expires)
{
    mExpires = expires;
    return NOERROR;
}

}
