//=========================================================================
// Copyright (C) 2024 The C++ Component Model(COMO) Open Source Project
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

#include "comotypekind.h"

namespace como {

static const char *TypeKindStr[] = {
    "Unknown,0",         // 0
    "Char,1",            // 1
    "Byte,2",            // 2
    "Short,3",           // 3
    "Integer,4",         // 4
    "Long,5",            // 5
    "Float,6",           // 6
    "Double,7",          // 7
    "Boolean,8",         // 8
    "String,9",          // 9
    "CoclassID,10",      // 10
    "ComponentID,11",    // 11
    "InterfaceID,12",    // 12
    "HANDLE,13",         // 13
    "ECode,14",          // 14
    "Enum,15",           // 15
    "Array,16",          // 16
    "Interface,17",      // 17
    "Coclass,18",        // 18
    "Triple,19",         // 19
    "TypeKind,20",       // 20
};


const char *TypeKindHelper::GetTypeName(TypeKind type)
{
    if ((type < TypeKind::Unknown) || (type > TypeKind::TypeKind)) {
        return "<bad TypeKind>";
    }

    return TypeKindStr[(int)type];
}

} // namespace como
