//=========================================================================
// Copyright (C) 2024 The C++ Component Model(COMO) Open Source Project
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

#include "CMethodTester2.h"

#include <cstdio>

namespace como {
namespace test {
namespace reflection {

COMO_INTERFACE_IMPL_1(CMethodTester2, Object, IMethodTest2);
COMO_OBJECT_IMPL(CMethodTester2);

CMethodTester2::CMethodTester2()
{
    printf("==== call CMethodTester2() ====\n");
}

CMethodTester2::~CMethodTester2()
{
    printf("==== call ~CMethodTester2() ====\n");
}

ECode CMethodTester2::TestMethod1(
    /* [in] */ Integer arg,
    /* [out] */ Integer& result)
{
    printf("==== call TestMethod1(arg:%d result addr:%p) ====\n", arg, &result);
    result = arg;
    return NOERROR;
}

ECode CMethodTester2::TestMethod2(
    /* [in] */ Float arg,
    /* [out] */ Float& result)
{
    printf("==== call TestMethod2(arg:%f result addr:%p) ====\n", arg, &result);
    result = arg;
    return NOERROR;
}

ECode CMethodTester2::OnFirstRef(
        /* [in] */ IObject* obj,
        /* [in] */ HANDLE value)
{
    printf("==== call OnFirstRef() ====\n");
    return NOERROR;
}

ECode CMethodTester2::OnLastStrongRef(
        /* [in] */ IObject* obj,
        /* [in] */ HANDLE value)
{
    printf("==== call OnLastStrongRef() ====\n");
    return NOERROR;
}

ECode CMethodTester2::OnLastWeakRef(
        /* [in] */ IObject* obj,
        /* [in] */ HANDLE value)
{
    printf("==== call OnLastWeakRef() ====\n");
    return NOERROR;
}

} // namespace test
} // namespace reflection
} // namespace como
