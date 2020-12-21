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

#ifndef __CDLC_UUID_H__
#define __CDLC_UUID_H__

#include "metadata/Metadata.h"
#include "util/AutoPtr.h"
#include "util/LightRefBase.h"
#include "util/String.h"

namespace cdlc {

class UUID
    : public LightRefBase
{
public:
    static bool IsValid(
        /* [in] */ const String& uuidStr);

    static AutoPtr<UUID> Parse(
        /* [in] */ const String& uuidStr);

    static AutoPtr<UUID> Parse(
        /* [in] */ const como::UUID& source);

    como::UUID ToComoUUID();

    String ToString();

    String Dump();

private:
    inline UUID();

    static char ToDigit(
        /* [in] */ char c);

public:
    unsigned int mData1;
    unsigned short mData2;
    unsigned short mData3;
    unsigned short mData4;
    unsigned char mData5[6];
};

UUID::UUID()
{}

}

#endif // __CDLC_UUID_H__
