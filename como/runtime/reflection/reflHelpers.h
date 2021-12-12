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

#ifndef __REFLHELPERS_H__
#define __REFLHELPERS_H__

#include "component/comoobjapi.h"
#include "reflection/CMetaInterface.h"
#include "reflection/CMetaType.h"

namespace como {

class reflHelpers {
public:
    static ECode constantGetLong(AutoPtr<IMetaConstant> constt, String constName, Long& lvalue);
    static ECode intfGetConstantLong(AutoPtr<IMetaInterface> intf, String constName, Long& lvalue);
};

} // namespace como

#endif // __REFLHELPERS_H__
