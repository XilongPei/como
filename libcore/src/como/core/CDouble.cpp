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

#include "como/core/CDouble.h"

using como::io::IID_ISerializable;

namespace como {
namespace core {

COMO_INTERFACE_IMPL_4(CDouble, SyncObject, IDouble, INumber, ISerializable, IComparable);

COMO_OBJECT_IMPL(CDouble);

ECode CDouble::Constructor(
    /* [in] */ Double value)
{
    mValue = value;
    return NOERROR;
}

ECode CDouble::ByteValue(
    /* [out] */ Byte& value)
{
    value = (Byte)mValue;
    return NOERROR;
}

ECode CDouble::ShortValue(
    /* [out] */ Short& value)
{
    value = mValue;
    return NOERROR;
}

ECode CDouble::IntegerValue(
    /* [out] */ Integer& value)
{
    value = (Integer)mValue;
    return NOERROR;
}

ECode CDouble::LongValue(
    /* [out] */ Long& value)
{
    value = (Long)mValue;
    return NOERROR;
}

ECode CDouble::FloatValue(
    /* [out] */ Float& value)
{
    value = (Float)mValue;
    return NOERROR;
}

ECode CDouble::DoubleValue(
    /* [out] */ Double& value)
{
    value = mValue;
    return NOERROR;
}

ECode CDouble::GetValue(
    /* [out] */ Double& value)
{
    value = mValue;
    return NOERROR;
}

ECode CDouble::IsInfinite(
    /* [out] */ Boolean& infinite)
{
    infinite = (mValue == POSITIVE_INFINITY) || (mValue == NEGATIVE_INFINITY);
    return NOERROR;
}

ECode CDouble::IsNaN(
    /* [out] */ Boolean& nan)
{
    nan = mValue != mValue;
    return NOERROR;
}

ECode CDouble::CompareTo(
    /* [in] */ IInterface* other,
    /* [out] */ Integer& result)
{
    IDouble* d = IDouble::Probe(other);
    if (d == nullptr) {
        result = -1;
        return NOERROR;
    }

    Double dv;
    d->GetValue(dv);
    result = mValue == dv ? 0 : (mValue > dv ? 1 : -1);
    return NOERROR;
}

ECode CDouble::Equals(
    /* [in] */ IInterface* other,
    /* [out] */ Boolean& result)
{
    IDouble* d = IDouble::Probe(other);
    if (d == nullptr) {
        result = false;
        return NOERROR;
    }

    Double dv;
    d->GetValue(dv);
    result = mValue == dv;
    return NOERROR;
}

ECode CDouble::GetHashCode(
    /* [out] */ Integer& hash)
{
    hash = (Integer)mValue;
    return NOERROR;
}

ECode CDouble::ToString(
    /* [out] */ String& str)
{
    str = String::Format("%f", mValue);
    return NOERROR;
}

}
}
