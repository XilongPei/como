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

#ifndef __COMO_IO_DELETEONEXITHOOK_H__
#define __COMO_IO_DELETEONEXITHOOK_H__

#include "como/core/SyncObject.h"
#include "como.util.IHashSet.h"
#include <comosp.h>

using como::core::SyncObject;
using como::util::IHashSet;

namespace como {
namespace io {

class DeleteOnExitHook
{
public:
    static ECode Add(
        /* [in] */ const String& file);

    static void RunHooks();

private:
    DeleteOnExitHook();

    static SyncObject& GetClassLock();

    static ECode StaticInitialize();

private:
    static AutoPtr<IHashSet> FILES;
};

} // namespace io
} // namespace como

#endif // __COMO_IO_DELETEONEXITHOOK_H__
