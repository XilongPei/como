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

#ifndef __CDLC_METHOD_H__
#define __CDLC_METHOD_H__

#include "ast/Node.h"
#include "ast/Parameter.h"
#include "ast/Type.h"
#include "util/AutoPtr.h"
#include "util/String.h"
#include <vector>

namespace cdlc {

class Method
    : public Node
{
public:
    inline String GetName();

    inline void SetName(
        /* [in] */ const String& name);

    inline String GetSignature();

    inline AutoPtr<Type> GetReturnType();

    inline void SetReturnType(
        /* [in] */ Type* type);

    inline void AddParameter(
        /* [in] */ Parameter* param);

    AutoPtr<Parameter> GetParameter(
        /* [in] */ int i);

    inline int GetParameterNumber();

    inline void SetDeleted(
        /* [in] */ bool deleted);

    inline bool IsDeleted();

    String ToString() override;

    String Dump(
        /* [in] */ const String& prefix) override;

    AutoPtr<Node> Clone(
        /* [in] */ Module* module,
        /* [in] */ bool deepCopy) override;

private:
    void BuildSignature();

private:
    String mName;
    String mSignature;
    AutoPtr<Type> mReturnType;
    std::vector<AutoPtr<Parameter>> mParameters;
    bool mDeleted = false;
};

String Method::GetName()
{
    return mName;
}

void Method::SetName(
    /* [in] */ const String& name)
{
    mName = name;
}

String Method::GetSignature()
{
    if (mSignature.IsEmpty()) {
        BuildSignature();
    }
    return mSignature;
}

AutoPtr<Type> Method::GetReturnType()
{
    return mReturnType;
}

void Method::SetReturnType(
    /* [in] */ Type* type)
{
    mReturnType = type;
}

void Method::AddParameter(
    /* [in] */ Parameter* param)
{
    if (param != nullptr) {
        mParameters.push_back(param);
    }
}

int Method::GetParameterNumber()
{
    return mParameters.size();
}

void Method::SetDeleted(
    /* [in] */ bool deleted)
{
    mDeleted = deleted;
}

bool Method::IsDeleted()
{
    return mDeleted;
}

}

#endif // __CDLC_METHOD_H__
