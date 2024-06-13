//=========================================================================
// Copyright (C) 2022 The C++ Component Model(COMO) Open Source Project
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

#ifndef __COMO_TEST_EXPIRES_CEXPIRESTEST_H__
#define __COMO_TEST_EXPIRES_CEXPIRESTEST_H__

#include <comoapi.h>
#include <comoobj.h>
#include "_como_test_expires_CExpiresTest.h"
#include "como.test.expires.IExpiresTest.h"

namespace como {
namespace test {
namespace expires {

Coclass(CExpiresTest)
    , public Object
    , public IExpiresTest
{
public:
    CExpiresTest();

    ~CExpiresTest();

    COMO_INTERFACE_DECL();

    ECode Add(
        /* [in] */ Integer arg1,
        /* [in] */ Integer arg2,
        /* [out] */ Integer& result) override;

    ECode Add_Wait(
        /* [in] */ Integer arg1,
        /* [in] */ Integer arg2,
        /* [in] */ Integer wait_time,
        /* [out] */ Integer& result) override;

    ECode Sub(
        /* [in] */ Integer arg1,
        /* [in] */ Integer arg2,
        /* [out] */ Integer& result) override;

    ECode Sub_Wait(
        /* [in] */ Integer arg1,
        /* [in] */ Integer arg2,
        /* [in] */ Integer wait_time,
        /* [out] */ Integer& result) override;
};

}
}
}

#endif //__COMO_TEST_EXPIRES_CEXPIRESTEST_H__
