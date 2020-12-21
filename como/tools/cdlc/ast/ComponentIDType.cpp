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

#include "ast/ComponentIDType.h"
#include "ast/Module.h"
#include "ast/Namespace.h"

namespace cdlc {

bool ComponentIDType::IsComponentIDType()
{
    return true;
}

bool ComponentIDType::IsBuildinType()
{
    return true;
}

String ComponentIDType::GetSignature()
{
    return "M";
}

AutoPtr<Node> ComponentIDType::Clone(
    /* [in] */ Module* module,
    /* [in] */ bool deepCopy)
{
    AutoPtr<ComponentIDType> clone = new ComponentIDType();
    CloneBase(clone, module);
    return clone;
}

}
