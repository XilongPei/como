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

#include "como/core/CString.h"

using como::io::IID_ISerializable;

namespace como {
namespace core {

COMO_INTERFACE_IMPL_4(CString, SyncObject, ICharSequence, IComparable, ISerializable, IString);
COMO_OBJECT_IMPL(CString);

ECode CString::Constructor(
    /* [in] */ const String& str)
{
    mString = str;
    return NOERROR;
}

ECode CString::GetLength(
    /* [out] */ Integer& number)
{
    number = mString.GetLength();
    return NOERROR;
}

ECode CString::GetCharAt(
    /* [in] */ Integer index,
    /* [out] */ Char& c)
{
    if (index < 0 || index >= mString.GetLength()) {
        return E_STRING_INDEX_OUT_OF_BOUNDS_EXCEPTION;
    }

    c = mString.GetChar(index);
    return NOERROR;
}

ECode CString::SubSequence(
    /* [in] */ Integer start,
    /* [in] */ Integer end,
    /* [out] */ AutoPtr<ICharSequence>& subcsq)
{
    if (start < 0 || start >= end || end > mString.GetLength()) {
        subcsq = nullptr;
        return E_STRING_INDEX_OUT_OF_BOUNDS_EXCEPTION;
    }

    String subStr = mString.Substring(start, end);
    subcsq = nullptr;
    return CString::New(subStr, IID_ICharSequence, (IInterface**)&subcsq);
}

ECode CString::CompareTo(
    /* [in] */ IInterface* other,
    /* [out] */ Integer& result)
{
    ICharSequence* o = ICharSequence::Probe(other);
    if (o == nullptr) {
        result = -1;
        return NOERROR;
    }

    String str;
    o->ToString(str);
    result = mString.Compare(str);
    return NOERROR;
}

ECode CString::Equals(
    /* [in] */ IInterface* obj,
    /* [out] */ Boolean& result)
{
    ICharSequence* o = ICharSequence::Probe(obj);
    if (o == nullptr) {
        result = false;
        return NOERROR;
    }

    String str;
    o->ToString(str);
    result = mString.Equals(str);
    return NOERROR;
}

ECode CString::GetHashCode(
    /* [out] */ Integer& hash)
{
    hash = mString.GetHashCode();
    return NOERROR;
}

ECode CString::ToString(
    /* [out] */ String& str)
{
    str = mString;
    return NOERROR;
}

}
}
