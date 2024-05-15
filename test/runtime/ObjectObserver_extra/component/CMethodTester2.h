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

#ifndef __COMO_TEST_REFLECTION_CMETHODTESTER_H__
#define __COMO_TEST_REFLECTION_CMETHODTESTER_H__

#include <comoapi.h>
#include <comoobj.h>
#include "_como_test_reflection_CMethodTester2.h"

namespace como {
namespace test {
namespace reflection {

Coclass(CMethodTester2)
    , public Object
    , public IMethodTest2
    , public IObjectObserver
{
public:
    CMethodTester2();

    ~CMethodTester2();

    COMO_INTERFACE_DECL();

    COMO_OBJECT_DECL();

    ECode TestMethod1(
        /* [in] */ Integer arg,
        /* [out] */ Integer& result) override;

    ECode TestMethod2(
        /* [in] */ Float arg,
        /* [out] */ Float& result) override;

    ECode OnFirstRef(
            /* [in] */ IObject* obj,
            /* [in] */ HANDLE value);

    ECode OnLastStrongRef(
            /* [in] */ IObject* obj,
            /* [in] */ HANDLE value);

    ECode OnLastWeakRef(
            /* [in] */ IObject* obj,
            /* [in] */ HANDLE value);
};

}
}
}

#endif // __COMO_TEST_REFLECTION_CMETHODTESTER_H__
