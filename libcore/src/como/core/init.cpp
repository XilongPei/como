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

#include "como/core/NativeRuntime.h"
#include <comolog.h>
#include <comotypes.h>

namespace como {
namespace core {

static CONS_PROI_2
void CoreStartup()
{
    if (!NativeRuntime::Create()) {
        Logger::E("CoreStartup", "Creating NativeRuntime failed.");
        return;
    }

    NativeRuntime* runtime = NativeRuntime::Current();
    Boolean started = runtime->Start();
    if (!started) {
        Logger::E("CoreStartup", "Starting NativeRuntime failed.");
    }
}

}
}