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

#ifndef __CDLC_POSTFIXEXPRESSION_H__
#define __CDLC_POSTFIXEXPRESSION_H__

#include "ast/Expression.h"

namespace cdlc {

class PostfixExpression
    : public Expression
{
public:
    inline void SetExpression(
        /* [in] */ Expression* expression);

    inline void SetBooleanValue(
        /* [in] */ bool value);

    inline void SetIntegralValue(
        /* [in] */ long long int value);

    inline void SetFloatingPointValue(
        /* [in] */ double value);

    inline void SetStringValue(
        /* [in] */ const String& value);

    inline void SetEnumeratorName(
        /* [in] */ const String& name);

    bool BooleanValue() override;

    char CharacterValue() override;

    int IntegerValue() override;

    long long int LongValue() override;

    float FloatValue() override;

    double DoubleValue() override;

    String StringValue() override;

    String EnumeratorValue() override;

    bool IsPositiveInfinity() override;

    bool IsNegativeInfinity() override;

    bool IsNaN() override;

private:
    AutoPtr<Expression> mNestedExpression;
    bool                mBooleanValue = false;
    long long int       mIntegralValue = 0;
    double              mFloatingPointValue = 0.0;
    String              mStringValue;
};

void PostfixExpression::SetExpression(
    /* [in] */ Expression* expression)
{
    mNestedExpression = expression;
}

void PostfixExpression::SetBooleanValue(
    /* [in] */ bool value)
{
    mBooleanValue = value;
}

void PostfixExpression::SetIntegralValue(
    /* [in] */ long long int value)
{
    mIntegralValue = value;
}

void PostfixExpression::SetFloatingPointValue(
    /* [in] */ double value)
{
    mFloatingPointValue = value;
}

void PostfixExpression::SetStringValue(
    /* [in] */ const String& value)
{
    mStringValue = value;
}

void PostfixExpression::SetEnumeratorName(
    /* [in] */ const String& name)
{
    mStringValue = name;
}

} // namespace cdlc

#endif // __CDLC_POSTFIXEXPRESSION_H__
