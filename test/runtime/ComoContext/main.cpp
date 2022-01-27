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

#include "como/math/BigIntegerFactory.h"
#include "como.core.ILong.h"
#include "como.core.INumber.h"
#include "como.math.CBigInteger.h"
#include "como.math.IBigInteger.h"
#include "como.util.CRandom.h"
#include "como.util.IRandom.h"
#include <comoobj.h>
#include <comosp.h>
#include <gtest/gtest.h>
#include "ComoContext.h"

using como::core::ILong;
using como::core::INumber;
using como::math::BigIntegerFactory;
using como::math::CBigInteger;
using como::math::IBigInteger;
using como::math::IID_IBigInteger;
using como::util::CRandom;
using como::util::IRandom;
using como::util::IID_IRandom;

/*
TEST(BigIntegerTest, HashCodeTest)
{
    AutoPtr<IBigInteger> firstBig;
    CBigInteger::New(String("3000354366789831885"), IID_IBigInteger, (IInterface**)&firstBig);
    AutoPtr<IBigInteger> secondBig;
    CBigInteger::New(String("3298535022597"), IID_IBigInteger, (IInterface**)&secondBig);
    AutoPtr<IBigInteger> andedBigs;
    firstBig->Add(secondBig, andedBigs);
    Long lv;
    INumber::Probe(andedBigs)->LongValue(lv);
    AutoPtr<IBigInteger> toCompareBig;
    BigIntegerFactory::ValueOf(lv, toCompareBig);
    EXPECT_TRUE(Object::Equals(andedBigs, toCompareBig));
}
*/
TEST(BigIntegerTest, ValueOfTest)
{
/*    for (Integer i = -1024; i <= 1024; ++i) {
        AutoPtr<IBigInteger> bi;
        BigIntegerFactory::ValueOf(i, bi);
*/
        BEGIN_USE_MY_MEM_AREA;

        CBigInteger *biTest;
        ((RefBase*)biTest)->SetFunFreeMem(ComoContext::gComoContext->freeMemInArea, 0);

        END_USE_MY_MEM_AREA;

/*        Integer iv;
        INumber::Probe(bi)->IntegerValue(iv);
        EXPECT_EQ(i, iv);
    }
*/
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
