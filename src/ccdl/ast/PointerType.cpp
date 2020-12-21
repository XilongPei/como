//=========================================================================
// Copyright (C) 2018 The C++ Component Model(CCM) Open Source Project
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

#include "PointerType.h"

namespace ccdl {
namespace ast {

PointerType::PointerType()
    : mBaseType(nullptr)
    , mPointerNumber(0)
{}

bool PointerType::IsPointerType()
{
    return true;
}

String PointerType::Signature()
{
    String typeSig = mBaseType->Signature();
    for (int i = 0; i < mPointerNumber; i++) {
        typeSig += "*";
    }
    return typeSig;
}

String PointerType::ToString()
{
    String typeStr = mBaseType->ToString();
    for (int i = 0; i < mPointerNumber; i++) {
        typeStr += "*";
    }
    return typeStr;
}

}
}
