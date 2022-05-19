//=========================================================================
// Copyright (C) 2022 The C++ Component Model(COMO) Open Source Project
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

#ifndef __MAC_H__
#define __MAC_H__

#include "comotypes.h"

namespace como {

class Mac {
public:

    static Long GetMacAddress(Long& lMacAddr);

    static Long GetThisServiceId(Long& lMacAddr, unsigned short port);

    static Long GetUuid64(Long& uuid64);
};

} // namespace como

#endif
