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

#include "como/core/CShort.h"

using como::io::IID_ISerializable;

namespace como {
namespace core {

COMO_INTERFACE_IMPL_4(CShort, SyncObject, IShort, INumber, ISerializable, IComparable);

COMO_OBJECT_IMPL(CShort);

ECode CShort::Constructor(
    /* [in] */ Short value)
{
    mValue = value;
    return NOERROR;
}

ECode CShort::ByteValue(
    /* [out] */ Byte& value)
{
    value = (Byte)mValue;
    return NOERROR;
}

ECode CShort::ShortValue(
    /* [out] */ Short& value)
{
    value = mValue;
    return NOERROR;
}

ECode CShort::IntegerValue(
    /* [out] */ Integer& value)
{
    value = (Integer)mValue;
    return NOERROR;
}

ECode CShort::LongValue(
    /* [out] */ Long& value)
{
    value = (Long)mValue;
    return NOERROR;
}

ECode CShort::FloatValue(
    /* [out] */ Float& value)
{
    value = (Float)mValue;
    return NOERROR;
}

ECode CShort::DoubleValue(
    /* [out] */ Double& value)
{
    value = (Double)mValue;
    return NOERROR;
}

ECode CShort::GetValue(
    /* [out] */ Short& value)
{
    value = mValue;
    return NOERROR;
}

ECode CShort::CompareTo(
    /* [in] */ IInterface* other,
    /* [out] */ Integer& result)
{
    IShort* s = IShort::Probe(other);
    if (s == nullptr) {
        result = -1;
        return NOERROR;
    }

    Short sv;
    s->GetValue(sv);
    result = mValue == sv ? 0 : (mValue > sv ? 1 : -1);
    return NOERROR;
}

ECode CShort::Equals(
    /* [in] */ IInterface* other,
    /* [out] */ Boolean& result)
{
    IShort* s = IShort::Probe(other);
    if (s == nullptr) {
        result = false;
        return NOERROR;
    }

    Short sv;
    s->GetValue(sv);
    result = mValue == sv;
    return NOERROR;
}

ECode CShort::GetHashCode(
    /* [out] */ Integer& hash)
{
    hash = (Integer)mValue;
    return NOERROR;
}

ECode CShort::ToString(
    /* [out] */ String& str)
{
    str = String::Format("%d", mValue);
    return NOERROR;
}

}
}
