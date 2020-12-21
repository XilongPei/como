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

#ifndef __LIBCORE_IO_CLIBCORE_H__
#define __LIBCORE_IO_CLIBCORE_H__

#include "libcore.io.ILibcore.h"
#include "_libcore_io_CLibcore.h"
#include <comoobj.h>

namespace libcore {
namespace io {

Coclass(CLibcore)
    , public Object
    , public ILibcore
{
public:
    COMO_INTERFACE_DECL();

    COMO_OBJECT_DECL();

    ECode GetRawOs(
        /* [out] */ IOs** os) override;

    ECode GetOs(
        /* [out] */ IOs** os) override;
};

}
}

#endif // __LIBCORE_IO_CLIBCORE_H__
