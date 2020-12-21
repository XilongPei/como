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

#ifndef __COMO_UTIL_ARRAYS_H__
#define __COMO_UTIL_ARRAYS_H__

#include "como.util.IComparator.h"
#include "como.util.IList.h"

namespace como {
namespace util {

class Arrays
{
public:
    static ECode CheckOffsetAndCount(
        /* [in] */ Integer arrayLength,
        /* [in] */ Integer offset,
        /* [in] */ Integer count);

    static Integer BinarySearch(
        /* [in] */ const Array<Long>& a,
        /* [in] */ Long key);

    static Integer BinarySearch(
        /* [in] */ const Array<String>& a,
        /* [in] */ const String& key);

    static Integer BinarySearch(
        /* [in] */ const Array<IInterface*>& a,
        /* [in] */ IInterface* key,
        /* [in] */ IComparator* c);

    static Boolean Equals(
        /* [in] */ const Array<Long>& a,
        /* [in] */ const Array<Long>& a2);

    static Boolean Equals(
        /* [in] */ const Array<Integer>& a,
        /* [in] */ const Array<Integer>& a2);

    static Boolean Equals(
        /* [in] */ const Array<Short>& a,
        /* [in] */ const Array<Short>& a2);

    static Boolean Equals(
        /* [in] */ const Array<Char>& a,
        /* [in] */ const Array<Char>& a2);

    static Boolean Equals(
        /* [in] */ const Array<Byte>& a,
        /* [in] */ const Array<Byte>& a2);

    static Boolean Equals(
        /* [in] */ const Array<Double>& a,
        /* [in] */ const Array<Double>& a2);

    static Boolean Equals(
        /* [in] */ const Array<IInterface*>& a,
        /* [in] */ const Array<IInterface*>& a2);

    static Boolean Equals(
        /* [in] */ const Array<String>& a,
        /* [in] */ const Array<String>& a2);

    static ECode Fill(
        /* [out] */ Array<Integer>& a,
        /* [in] */ Integer fromIndex,
        /* [in] */ Integer toIndex,
        /* [in] */ Integer val);

    static ECode Fill(
        /* [out] */ Array<Char>& a,
        /* [in] */ Char value);

    static ECode Fill(
        /* [out] */ Array<Char>& a,
        /* [in] */ Integer fromIndex,
        /* [in] */ Integer toIndex,
        /* [in] */ Char value);

    static ECode Fill(
        /* [out] */ Array<Boolean>& a,
        /* [in] */ Boolean value);

    static ECode CopyOf(
        /* [in] */ const Array<Byte>& original,
        /* [in] */ Integer newLength,
        /* [out, callee] */ Array<Byte>* newArray);

    static ECode CopyOf(
        /* [in] */ const Array<Integer>& original,
        /* [in] */ Integer newLength,
        /* [out, callee] */ Array<Integer>* newArray);

    static ECode CopyOf(
        /* [in] */ const Array<Double>& original,
        /* [in] */ Integer newLength,
        /* [out, callee] */ Array<Double>* newArray);

    static ECode CopyOf(
        /* [in] */ const Array<String>& original,
        /* [in] */ Integer newLength,
        /* [out, callee] */ Array<String>* newArray);

    static ECode CopyOf(
        /* [in] */ const Array<IInterface*>& original,
        /* [in] */ Integer newLength,
        /* [out, callee] */ Array<IInterface*>* newArray);

    static ECode AsList(
        /* [in] */ const Array<IInterface*>& a,
        /* [out] */ IList** list);

    static Integer GetHashCode(
        /* [in] */ const Array<Long>& a);

    static Integer GetHashCode(
        /* [in] */ const Array<Integer>& a);

    static Integer GetHashCode(
        /* [in] */ const Array<Short>& a);

    static Integer GetHashCode(
        /* [in] */ const Array<Char>& a);

    static Integer GetHashCode(
        /* [in] */ const Array<Byte>& a);

    static Integer GetHashCode(
        /* [in] */ const Array<Boolean>& a);

    static Integer GetHashCode(
        /* [in] */ const Array<Float>& a);

    static Integer GetHashCode(
        /* [in] */ const Array<Double>& a);

    static Integer GetHashCode(
        /* [in] */ const Array<IInterface*>& a);

    static Boolean DeepEquals(
        /* [in] */ const Array<Array<String>>& a,
        /* [in] */ const Array<Array<String>>& a2);

private:
    Arrays()
    {}

    static ECode RangeCheck(
        /* [in] */ Integer arrayLength,
        /* [in] */ Integer fromIndex,
        /* [in] */ Integer toIndex);

    static Integer BinarySearch0(
        /* [in] */ const Array<Long>& a,
        /* [in] */ Integer fromIndex,
        /* [in] */ Integer toIndex,
        /* [in] */ Long key);

    static Integer BinarySearch0(
        /* [in] */ const Array<String>& a,
        /* [in] */ Integer fromIndex,
        /* [in] */ Integer toIndex,
        /* [in] */ String key);

    static Integer BinarySearch0(
        /* [in] */ const Array<IInterface*>& a,
        /* [in] */ Integer fromIndex,
        /* [in] */ Integer toIndex,
        /* [in] */ IInterface* key);

    static Integer BinarySearch0(
        /* [in] */ const Array<IInterface*>& a,
        /* [in] */ Integer fromIndex,
        /* [in] */ Integer toIndex,
        /* [in] */ IInterface* key,
        /* [in] */ IComparator* c);
};

}
}

#endif // __COMO_UTIL_ARRAYS_H__
