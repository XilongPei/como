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

#include "libcore/io/CLibcore.h"
#include "libcore/io/Libcore.h"

namespace libcore {
namespace io {

COMO_INTERFACE_IMPL_1(CLibcore, Object, ILibcore);

COMO_OBJECT_IMPL(CLibcore);

ECode CLibcore::GetRawOs(
    /* [out] */ IOs** os)
{
    VALIDATE_NOT_NULL(os);

    AutoPtr<IOs> o = Libcore::GetRawOs();
    o.MoveTo(os);
    return NOERROR;
}

ECode CLibcore::GetOs(
    /* [out] */ IOs** os)
{
    VALIDATE_NOT_NULL(os);

    AutoPtr<IOs> o = Libcore::GetOs();
    o.MoveTo(os);
    return NOERROR;
}

} // namespace io
} // namespace libcore
