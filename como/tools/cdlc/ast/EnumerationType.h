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

#ifndef __CDLC_ENUMERATIONTYPE_H__
#define __CDLC_ENUMERATIONTYPE_H__

#include "ast/Type.h"
#include <vector>

namespace cdlc {

class EnumerationType
    : public Type
{
public:
    class Enumerator
        : public LightRefBase
    {
    public:
        Enumerator(
            /* [in] */ const String& name,
            /* [in] */ int value)
            : mName(name)
            , mValue(value)
        {}

    public:
        String mName;
        int mValue;
    };

public:
    void AddEnumerator(
        /* [in] */ const String& name,
        /* [in] */ int value);

    bool Contains(
        /* [in] */ const String& name);

    AutoPtr<Enumerator> GetEnumerator(
        /* [in] */ int i);

    inline int GetEnumeratorNumber();

    bool IsEnumerationType() override;

    bool IsBuildinType() override;

    String GetSignature() override;

    String ToString() override;

    String Dump(
        /* [in] */ const String& prefix) override;

    AutoPtr<Node> Clone(
        /* [in] */ Module* module,
        /* [in] */ bool deepCopy) override;

    inline static AutoPtr<EnumerationType> CastFrom(
        /* [in] */ Type* type);

private:
    std::vector<AutoPtr<Enumerator>> mEnumerators;
};

int EnumerationType::GetEnumeratorNumber()
{
    return mEnumerators.size();
}

AutoPtr<EnumerationType> EnumerationType::CastFrom(
    /* [in] */ Type* type)
{
    return static_cast<EnumerationType*>(type);
}

}

#endif // __CDLC_ENUMERATIONTYPE_H__
