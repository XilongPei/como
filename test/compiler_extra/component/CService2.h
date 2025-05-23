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

#ifndef __COMO_TEST_RPC_CSERVICE_H__
#define __COMO_TEST_RPC_CSERVICE_H__

#include <comoapi.h>
#include <comoobj.h>
#include "_como_test_rpc_CService2.h"

namespace como {
namespace test {
namespace rpc {

Coclass(CService2)
    , public Object
    , public IService2
{
public:
    CService2();

    ~CService2();

    COMO_INTERFACE_DECL();

    COMO_OBJECT_DECL();

    ECode TestMethod1(
        /* [in] */ const Array<Integer>& arg1,
        /* [out] */ Array<Integer>* result1) override;

    ECode TestMethod2(
        /* [in] */ Integer arg1,
        /* [out] */ Char* result1,
        /* [out] */ Char* result2,
        /* [out] */ Char* result3,
        /* [out] */ Char* result4) override;

    ECode TestMethod3(
        /* [in] */ TestEnum arg1,
        /* [out] */ String& result1) override;

    ECode TestMethod4(
        /* [in] */ const Triple& arg1,
        /* [out] */ Char* result1,
        /* [out] */ Long& result2,
        /* [out] */ TypeKind& result3) override;
};

} // namespace rpc
} // namespace test
} // namespace como

#endif // __COMO_TEST_RPC_CSERVICE_H__
