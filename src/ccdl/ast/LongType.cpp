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

#include "LongType.h"

namespace ccdl {
namespace ast {

LongType::LongType()
{
    SetName(String("Long"));
}

bool LongType::IsPrimitiveType()
{
    return true;
}

bool LongType::IsNumericType()
{
    return true;
}

bool LongType::IsIntegralType()
{
    return true;
}

bool LongType::IsLongType()
{
    return true;
}

String LongType::Signature()
{
    return String("L");
}

}
}
