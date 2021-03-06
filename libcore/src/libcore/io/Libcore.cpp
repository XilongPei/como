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

#include "libcore/io/BlockGuardOs.h"
#include "libcore/io/Libcore.h"
#include "libcore/io/Linux.h"

namespace libcore {
namespace io {

AutoPtr<IOs> Libcore::GetRawOs()
{
    static AutoPtr<IOs> sRawOs = new Linux();
    return sRawOs;
}

AutoPtr<IOs> Libcore::GetOs()
{
    static AutoPtr<IOs> sOs = new BlockGuardOs(GetRawOs());
    return sOs;
}

} // namespace io
} // namespace libcore
