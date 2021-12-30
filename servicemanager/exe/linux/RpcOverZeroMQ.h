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


#ifndef __JING_RPCOVERZeroMQ_H__
#define __JING_RPCOVERZeroMQ_H__

#include <comoapi.h>
#include <comosp.h>
#include <comoref.h>

namespace jing {

class RpcOverZeroMQ
    : public LightRefBase
{
public:
    static void startTPZA_Executor();
    static ECode HandleMessage();
};

} // namespace jing

#endif // __JING_RPCOVERZeroMQ_H__
