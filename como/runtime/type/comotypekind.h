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

#ifndef __COMO_TYPEKIND_H__
#define __COMO_TYPEKIND_H__

/**
 * The macro `COM_PUBLIC` is defined in `comodef.h`. To avoid cross-referencing
 * header files, write it directly here.
 */
#ifndef COM_PUBLIC
#define COM_PUBLIC      __attribute__ ((visibility ("default")))
#endif

namespace como {

enum class TypeKind
{
    Unknown,
    Char = 1,       // 1
    Byte,           // 2
    Short,          // 3
    Integer,        // 4
    Long,           // 5
    Float,          // 6
    Double,         // 7
    Boolean,        // 8
    String,         // 9
    CoclassID,      // 10
    ComponentID,    // 11
    InterfaceID,    // 12
    HANDLE,         // 13
    ECode,          // 14
    Enum,           // 15
    Array,          // 16
    Interface,      // 17
    Coclass,        // 18
    Triple,         // 19
    TypeKind,       // 20
};

class COM_PUBLIC TypeKindHelper
{
public:
    static const char *GetTypeName(TypeKind type);
};

} // namespace como

#endif // __COMO_TYPEKIND_H__
