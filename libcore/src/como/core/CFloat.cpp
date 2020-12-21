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

#include "como/core/CFloat.h"

using como::io::IID_ISerializable;

namespace como {
namespace core {

COMO_INTERFACE_IMPL_4(CFloat, SyncObject, IFloat, INumber, ISerializable, IComparable);

COMO_OBJECT_IMPL(CFloat);

ECode CFloat::Constructor(
    /* [in] */ Float value)
{
    mValue = value;
    return NOERROR;
}

ECode CFloat::ByteValue(
    /* [out] */ Byte& value)
{
    value = (Byte)mValue;
    return NOERROR;
}

ECode CFloat::ShortValue(
    /* [out] */ Short& value)
{
    value = (Short)mValue;
    return NOERROR;
}

ECode CFloat::IntegerValue(
    /* [out] */ Integer& value)
{
    value = (Integer)mValue;
    return NOERROR;
}

ECode CFloat::LongValue(
    /* [out] */ Long& value)
{
    value = (Long)mValue;
    return NOERROR;
}

ECode CFloat::FloatValue(
    /* [out] */ Float& value)
{
    value = mValue;
    return NOERROR;
}

ECode CFloat::DoubleValue(
    /* [out] */ Double& value)
{
    value = (Double)mValue;
    return NOERROR;
}

ECode CFloat::GetValue(
    /* [out] */ Float& value)
{
    value = mValue;
    return NOERROR;
}

ECode CFloat::IsInfinite(
    /* [out] */ Boolean& infinite)
{
    infinite = (mValue == POSITIVE_INFINITY) || (mValue == NEGATIVE_INFINITY);
    return NOERROR;
}

ECode CFloat::IsNaN(
    /* [out] */ Boolean& nan)
{
    nan = mValue != mValue;
    return NOERROR;
}

ECode CFloat::CompareTo(
    /* [in] */ IInterface* other,
    /* [out] */ Integer& result)
{
    IFloat* f = IFloat::Probe(other);
    if (f == nullptr) {
        result = -1;
        return NOERROR;
    }

    Float fv;
    f->GetValue(fv);
    result = mValue == fv ? 0 : (mValue > fv ? 1 : -1);
    return NOERROR;
}

ECode CFloat::Equals(
    /* [in] */ IInterface* other,
    /* [out] */ Boolean& result)
{
    IFloat* f = IFloat::Probe(other);
    if (f == nullptr) {
        result = false;
        return NOERROR;
    }

    Float fv;
    f->GetValue(fv);
    result = mValue == fv;
    return NOERROR;
}

ECode CFloat::GetHashCode(
    /* [out] */ Integer& hash)
{
    hash = (Integer)mValue;
    return NOERROR;
}

ECode CFloat::ToString(
    /* [out] */ String& str)
{
    str = String::Format("%f", mValue);
    return NOERROR;
}

}
}
