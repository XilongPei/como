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

#include "PostfixExpression.h"

namespace ccdl {
namespace ast {

PostfixExpression::PostfixExpression()
    : mBooleanValue(false)
    , mIntegralValue(0)
    , mFloatingPointValue(0)
    , mExpression(nullptr)
{}

int PostfixExpression::IntegerValue()
{
    return (int)mIntegralValue;
}

long long int PostfixExpression::LongValue()
{
    return mIntegralValue;
}

float PostfixExpression::FloatValue()
{
    return (float)mFloatingPointValue;
}

double PostfixExpression::DoubleValue()
{
    return mFloatingPointValue;
}

char PostfixExpression::CharacterValue()
{
    return (char)mIntegralValue;
}

bool PostfixExpression::BooleanValue()
{
    return mBooleanValue;
}

String PostfixExpression::StringValue()
{
    return mStringValue;
}

String PostfixExpression::EnumeratorValue()
{
    return mEnumeratorName;
}

bool PostfixExpression::IsPositiveInfinity()
{
    return false;
}

bool PostfixExpression::IsNegativeInfinity()
{
    return false;
}

bool PostfixExpression::IsNaN()
{
    return false;
}

Expression* PostfixExpression::Clone()
{
    PostfixExpression* newExpr = new PostfixExpression();
    newExpr->mBooleanValue = mBooleanValue;
    newExpr->mIntegralValue = mIntegralValue;
    newExpr->mFloatingPointValue = mFloatingPointValue;
    newExpr->mStringValue = mStringValue;
    newExpr->mEnumeratorName = mEnumeratorName;
    if (mExpression != nullptr) {
        newExpr->mExpression = mExpression->Clone();
    }
    return newExpr;
}

}
}
