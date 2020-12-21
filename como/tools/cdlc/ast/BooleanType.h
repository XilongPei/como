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

#ifndef __CDLC_BOOLEANTYPE_H__
#define __CDLC_BOOLEANTYPE_H__

#include "ast/Type.h"

namespace cdlc {

class BooleanType
    : public Type
{
public:
    inline BooleanType();

    bool IsBooleanType() override;

    bool IsBuildinType() override;

    String GetSignature() override;

    AutoPtr<Node> Clone(
        /* [in] */ Module* module,
        /* [in] */ bool deepCopy) override;
};

BooleanType::BooleanType()
{
    mName = "Boolean";
}

}

#endif // __CDLC_BOOLEANTYPE_H__
