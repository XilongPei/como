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

#include "ExclusiveOrExpression.h"

namespace ccdl {
namespace ast {

int ExclusiveOrExpression::IntegerValue()
{
    if (mLeftOperand != nullptr) {
        return mLeftOperand->IntegerValue() ^
                mRightOperand->IntegerValue();
    }
    else {
        return mRightOperand->IntegerValue();
    }
}

long long int ExclusiveOrExpression::LongValue()
{
    if (mLeftOperand != nullptr) {
        long long int leftValue = mLeftOperand->GetType()->IsIntegerType() ?
                mLeftOperand->IntegerValue() : mLeftOperand->LongValue();
        long long int rightValue = mRightOperand->GetType()->IsIntegerType() ?
                mRightOperand->IntegerValue() : mRightOperand->LongValue();
        return leftValue ^ rightValue;
    }
    else {
        return mRightOperand->LongValue();
    }
}

float ExclusiveOrExpression::FloatValue()
{
    return mRightOperand->FloatValue();
}

double ExclusiveOrExpression::DoubleValue()
{
    return mRightOperand->DoubleValue();
}

char ExclusiveOrExpression::CharacterValue()
{
    return mRightOperand->CharacterValue();
}

bool ExclusiveOrExpression::BooleanValue()
{
    return mRightOperand->BooleanValue();
}

String ExclusiveOrExpression::StringValue()
{
    return mRightOperand->StringValue();
}

String ExclusiveOrExpression::EnumeratorValue()
{
    return mRightOperand->EnumeratorValue();
}

bool ExclusiveOrExpression::IsPositiveInfinity()
{
    if (mLeftOperand != nullptr || mRightOperand == nullptr) {
        return false;
    }
    return mRightOperand->IsPositiveInfinity();
}

bool ExclusiveOrExpression::IsNegativeInfinity()
{
    if (mLeftOperand != nullptr || mRightOperand == nullptr) {
        return false;
    }
    return mRightOperand->IsNegativeInfinity();
}

bool ExclusiveOrExpression::IsNaN()
{
    if (mLeftOperand != nullptr || mRightOperand == nullptr) {
        return false;
    }
    return mRightOperand->IsNaN();
}

Expression* ExclusiveOrExpression::Clone()
{
    ExclusiveOrExpression* newExpr = new ExclusiveOrExpression();
    if (mLeftOperand != nullptr) {
        newExpr->mLeftOperand = mLeftOperand->Clone();
    }
    if (mRightOperand != nullptr) {
        newExpr->mRightOperand = mRightOperand->Clone();
    }
    newExpr->mOperator = mOperator;
    newExpr->mType = mType;
    newExpr->mRadix = mRadix;
    newExpr->mScientificNotation = mScientificNotation;
    return newExpr;
}

}
}
