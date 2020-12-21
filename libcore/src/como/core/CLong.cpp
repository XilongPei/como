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

#include "como/core/CLong.h"

using como::io::IID_ISerializable;

namespace como {
namespace core {

COMO_INTERFACE_IMPL_4(CLong, SyncObject, ILong, INumber, ISerializable, IComparable);

COMO_OBJECT_IMPL(CLong);

ECode CLong::Constructor(
    /* [in] */ Long value)
{
    mValue = value;
    return NOERROR;
}

ECode CLong::ByteValue(
    /* [out] */ Byte& value)
{
    value = (Byte)mValue;
    return NOERROR;
}

ECode CLong::ShortValue(
    /* [out] */ Short& value)
{
    value = (Short)mValue;
    return NOERROR;
}

ECode CLong::IntegerValue(
    /* [out] */ Integer& value)
{
    value = (Integer)mValue;
    return NOERROR;
}

ECode CLong::LongValue(
    /* [out] */ Long& value)
{
    value = mValue;
    return NOERROR;
}

ECode CLong::FloatValue(
    /* [out] */ Float& value)
{
    value = (Float)mValue;
    return NOERROR;
}

ECode CLong::DoubleValue(
    /* [out] */ Double& value)
{
    value = (Double)mValue;
    return NOERROR;
}

ECode CLong::GetValue(
    /* [out] */ Long& value)
{
    value = mValue;
    return NOERROR;
}

ECode CLong::CompareTo(
    /* [in] */ IInterface* other,
    /* [out] */ Integer& result)
{
    ILong* l = ILong::Probe(other);
    if (l == nullptr) {
        result = -1;
        return NOERROR;
    }

    Long lv;
    l->GetValue(lv);
    result = mValue == lv ? 0 : (mValue > lv ? 1 : -1);
    return NOERROR;
}

ECode CLong::Equals(
    /* [in] */ IInterface* other,
    /* [out] */ Boolean& result)
{
    ILong* l = ILong::Probe(other);
    if (l == nullptr) {
        result = false;
        return NOERROR;
    }

    Long lv;
    l->GetValue(lv);
    result = mValue == lv;
    return NOERROR;
}

ECode CLong::GetHashCode(
    /* [out] */ Integer& hash)
{
    hash = (Integer)mValue;
    return NOERROR;
}

ECode CLong::ToString(
    /* [out] */ String& str)
{
    str = String::Format("%lld", mValue);
    return NOERROR;
}

}
}
