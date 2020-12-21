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

#include "como/core/StringUtils.h"
#include "como/util/concurrent/atomic/AtomicLong.h"

using como::core::IID_INumber;
using como::core::StringUtils;
using como::io::IID_ISerializable;

namespace como {
namespace util {
namespace concurrent {
namespace atomic {

COMO_INTERFACE_IMPL_3(AtomicLong, SyncObject, IAtomicLong, INumber, ISerializable);

ECode AtomicLong::Constructor(
    /* [in] */ Long initialValue)
{
    mValue.StoreRelaxed(initialValue);
    return NOERROR;
}

ECode AtomicLong::Constructor()
{
    return NOERROR;
}

ECode AtomicLong::Get(
    /* [out] */ Long& value)
{
    value = mValue.LoadAcquire();
    return NOERROR;
}

ECode AtomicLong::Set(
    /* [in] */ Long value)
{
    mValue.StoreSequentiallyConsistent(value);
    return NOERROR;
}

ECode AtomicLong::LzaySet(
    /* [in] */ Long value)
{
    mValue.StoreRelease(value);
    return NOERROR;
}

ECode AtomicLong::GetAndSet(
    /* [in] */ Long newValue,
    /* [out] */ Long& prevValue)
{
    Long v;
    do {
        v = mValue.LoadSequentiallyConsistent();
    } while (!mValue.CompareExchangeStrongSequentiallyConsistent(v, newValue));
    prevValue = v;
    return NOERROR;
}

ECode AtomicLong::CompareAndSet(
    /* [in] */ Long expect,
    /* [in] */ Long update,
    /* [out] */ Boolean* succeeded)
{
    Boolean result = mValue.CompareExchangeStrongSequentiallyConsistent(
            expect, update);
    if (succeeded != nullptr) {
        *succeeded = result;
    }
    return NOERROR;
}

ECode AtomicLong::WeakCompareAndSet(
    /* [in] */ Long expect,
    /* [in] */ Long update,
    /* [out] */ Boolean* succeeded)
{
    Boolean result = mValue.CompareExchangeStrongSequentiallyConsistent(
            expect, update);
    if (succeeded != nullptr) {
        *succeeded = result;
    }
    return NOERROR;
}

ECode AtomicLong::GetAndIncrement(
    /* [out] */ Long& prevValue)
{
    Long v;
    do {
        v = mValue.LoadSequentiallyConsistent();
    } while (!mValue.CompareExchangeStrongSequentiallyConsistent(v, v + 1));
    prevValue = v;
    return NOERROR;
}

ECode AtomicLong::GetAndDecrement(
    /* [out] */ Long& prevValue)
{
    Long v;
    do {
        v = mValue.LoadSequentiallyConsistent();
    } while (!mValue.CompareExchangeStrongSequentiallyConsistent(v, v - 1));
    prevValue = v;
    return NOERROR;
}

ECode AtomicLong::GetAndAdd(
    /* [in] */ Long delta,
    /* [out] */ Long& prevValue)
{
    Long v;
    do {
        v = mValue.LoadSequentiallyConsistent();
    } while (!mValue.CompareExchangeStrongSequentiallyConsistent(v, v + delta));
    prevValue = v;
    return NOERROR;
}

ECode AtomicLong::IncrementAndGet(
    /* [out] */ Long& value)
{
    Long v;
    do {
        v = mValue.LoadSequentiallyConsistent();
    } while (!mValue.CompareExchangeStrongSequentiallyConsistent(v, v + 1));
    value = v + 1;
    return NOERROR;
}

ECode AtomicLong::DecrementAndGet(
    /* [out] */ Long& value)
{
    Long v;
    do {
        v = mValue.LoadSequentiallyConsistent();
    } while (!mValue.CompareExchangeStrongSequentiallyConsistent(v, v - 1));
    value = v - 1;
    return NOERROR;
}

ECode AtomicLong::AddAndGet(
    /* [in] */ Long delta,
    /* [out] */ Long& value)
{
    Long v;
    do {
        v = mValue.LoadSequentiallyConsistent();
    } while (!mValue.CompareExchangeStrongSequentiallyConsistent(v, v + delta));
    value = v + delta;
    return NOERROR;
}

ECode AtomicLong::ToString(
    /* [out] */ String& desc)
{
    desc = StringUtils::ToString((Long)mValue.LoadAcquire());
    return NOERROR;
}

ECode AtomicLong::IntegerValue(
    /* [out] */ Integer& value)
{
    Long v;
    Get(v);
    value = (Integer)v;
    return NOERROR;
}

ECode AtomicLong::LongValue(
    /* [out] */ Long& value)
{
    return Get(value);
}

ECode AtomicLong::FloatValue(
    /* [out] */ Float& value)
{
    Long v;
    Get(v);
    value = (Float)v;
    return NOERROR;
}

ECode AtomicLong::DoubleValue(
    /* [out] */ Double& value)
{
    Long v;
    Get(v);
    value = (Double)v;
    return NOERROR;
}

ECode AtomicLong::ByteValue(
    /* [out] */ Byte& value)
{
    Long v;
    Get(v);
    value = (Byte)v;
    return NOERROR;
}

ECode AtomicLong::ShortValue(
    /* [out] */ Short& value)
{
    Long v;
    Get(v);
    value = (Short)v;
    return NOERROR;
}

}
}
}
}
