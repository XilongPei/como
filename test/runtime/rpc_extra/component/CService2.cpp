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

#include <stdio.h>
#include "CService2.h"

namespace como {
namespace test {
namespace rpc {

COMO_INTERFACE_IMPL_1(CService2, Object, IService2);
COMO_OBJECT_IMPL(CService2);

CService2::CService2()
{}

CService2::~CService2()
{}

ECode CService2::TestMethod1(
        /* [in] */ const Array<Integer>& arg1,
        /* [out] */ Array<Integer>* result1)
{

    Long size = arg1.GetLength();
    Long size1 = (*result1).GetLength();
    Logger::D("CService2", "in CService2::TestMethod1 [in]size: %ld %ld, [out]result1: %p",
                                                                    size, size1, result1);
    (*result1).Alloc(size);

    Long size12 = (*result1).GetLength();
    Logger::D("CService2", "$debug$, in CService2::TestMethod1 [in]size: %ld %ld, [out]result1: %p",
                                                                    size, size12, result1);
    for (int i = 0;  i < size - 1;  i++) {
        (*result1)[size - i - 1] = arg1[i];
    }

    Logger::D("CService2", "out CService2::TestMethod1");
    return NOERROR;
}

ECode CService2::TestMethod2(
    /* [in] */ Integer arg1,
    /* [out] */ Char* result1,
    /* [out] */ Char* result2,
    /* [out] */ Char* result3,
    /* [out] */ Char* result4)
{
    *result1 = *(char*)&arg1;
    *result2 = *((char*)&arg1 + 1);
    *result3 = *((char*)&arg1 + 2);
    *result4 = *((char*)&arg1 + 3);
    return NOERROR;
}

ECode CService2::TestMethod3(
        /* [in] */ TestEnum arg1,
        /* [out] */ String& result1)
{
    switch (arg1) {
        case TestEnum::Monday:
            result1 = "Monday";
            break;
        case TestEnum::Tuesday:
            result1 = "Tuesday";
            break;
        case TestEnum::Wednesday:
            result1 = "Wednesday";
            break;
        case TestEnum::Thursday:
            result1 = "Thursday";
            break;
        case TestEnum::Friday:
            result1 = "Friday";
            break;
        case TestEnum::Saturday:
            result1 = "Saturday";
            break;
        case TestEnum::Sunday:
            result1 = "Sunday";
            break;
        default:
            return E_NOT_FOUND_EXCEPTION;
            break;
    }
    return NOERROR;
}

ECode CService2::TestMethod4(
    /* [in] */ const Triple& arg1,
    /* [out] */ Char* result1,
    /* [out] */ Long& result2,
    /* [out] */ TypeKind& result3)
{
    result1 = (Char*)arg1.mData;
    result2 = arg1.mSize;
    result3 = arg1.mType;
    return NOERROR;
}

}
}
}
