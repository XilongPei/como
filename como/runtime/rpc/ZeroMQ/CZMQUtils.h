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

#ifndef __CZMQUTILS_H__
#define __CZMQUTILS_H__

#include "CProxy.h"
#include "CStub.h"
#include "ThreadPoolExecutor.h"
#include "util/comoobj.h"
#include "util/mutex.h"
#include <dbus/dbus.h>

namespace como {

class CZMQUtils {

    void *zmq_getContext();

    void *zmq_getSocket(void *context, char *serverName, int type_);

    ECode zmq_SendWithReplyAndBlock();
};

} // namespace como

#endif // __CZMQUTILS_H__
