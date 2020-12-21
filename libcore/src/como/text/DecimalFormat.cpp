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

#include "como/core/CoreUtils.h"
#include "como/core/CDouble.h"
#include "como/core/CLong.h"
#include "como/core/Math.h"
#include "como/math/CBigDecimal.h"
#include "como/text/CDecimalFormatSymbols.h"
#include "como/text/DecimalFormat.h"
#include "como/util/CLocale.h"
#include "como/util/Currency.h"
#include "como/util/concurrent/CConcurrentHashMap.h"
#include "como.core.IByte.h"
#include "como.core.ICharSequence.h"
#include "como.core.ILong.h"
#include "como.core.INumber.h"
#include "como.core.IShort.h"
#include "como.math.IBigInteger.h"
#include "como.util.IMap.h"
#include "como.util.concurrent.atomic.IAtomicInteger.h"
#include "como.util.concurrent.atomic.IAtomicLong.h"
#include "libcore/icu/LocaleData.h"
#include "libcore/icu/NativeDecimalFormat.h"
#include <comolog.h>

using como::core::CDouble;
using como::core::CLong;
using como::core::CoreUtils;
using como::core::E_NULL_POINTER_EXCEPTION;
using como::core::IByte;
using como::core::ICharSequence;
using como::core::IDouble;
using como::core::IID_IDouble;
using como::core::IID_INumber;
using como::core::ILong;
using como::core::INumber;
using como::core::IShort;
using como::core::Math;
using como::math::CBigDecimal;
using como::math::IBigInteger;
using como::util::CLocale;
using como::util::Currency;
using como::util::ILocale;
using como::util::IMap;
using como::util::concurrent::CConcurrentHashMap;
using como::util::concurrent::IID_IConcurrentMap;
using como::util::concurrent::atomic::IAtomicInteger;
using como::util::concurrent::atomic::IAtomicLong;
using libcore::icu::ILocaleData;
using libcore::icu::LocaleData;
using libcore::icu::NativeDecimalFormat;

namespace como {
namespace text {

static AutoPtr<IDouble> Create_NEGATIVE_ZERO_DOUBLE()
{
    AutoPtr<IDouble> db;
    CDouble::New(-0.0, IID_IDouble, (IInterface**)&db);
    return db;
}

static AutoPtr<IDouble> Get_NEGATIVE_ZERO_DOUBLE()
{
    static const AutoPtr<IDouble> NEGATIVE_ZERO_DOUBLE = Create_NEGATIVE_ZERO_DOUBLE();
    return NEGATIVE_ZERO_DOUBLE;
}

static AutoPtr<IConcurrentMap> CreateConcurrentHashMap()
{
    AutoPtr<IConcurrentMap> map;
    CConcurrentHashMap::New(3, IID_IConcurrentMap, (IInterface**)&map);
    return map;
}

AutoPtr<IConcurrentMap> DecimalFormat::GetCachedLocaleData()
{
    static const AutoPtr<IConcurrentMap> sCachedLocaleData =
            CreateConcurrentHashMap();
    return sCachedLocaleData;
}

COMO_INTERFACE_IMPL_1(DecimalFormat, NumberFormat, IDecimalFormat);

DecimalFormat::~DecimalFormat()
{
    if (mNativeDF != 0) {
        delete reinterpret_cast<NativeDecimalFormat*>(mNativeDF);
    }
}

ECode DecimalFormat::Constructor()
{
    AutoPtr<ILocale> def = CLocale::GetDefault(CLocale::Category::GetFORMAT());
    AutoPtr<IInterface> v;
    IMap::Probe(GetCachedLocaleData())->Get(def, v);
    AutoPtr<ICharSequence> pattern = std::move(v);
    String patternStr = CoreUtils::Unbox(pattern);
    if (patternStr.IsNull()) {
        AutoPtr<ILocaleData> data;
        LocaleData::Get(def, &data);
        data->GetNumberPattern(&patternStr);
        IMap::Probe(GetCachedLocaleData())->PutIfAbsent(def, CoreUtils::Box(patternStr));
    }
    CDecimalFormatSymbols::New(def, IID_IDecimalFormatSymbols, (IInterface**)&mSymbols);
    return Init(patternStr);
}

ECode DecimalFormat::Constructor(
    /* [in] */ const String& pattern)
{
    CDecimalFormatSymbols::New(CLocale::GetDefault(
            CLocale::Category::GetFORMAT()), IID_IDecimalFormatSymbols, (IInterface**)&mSymbols);
    return Init(pattern);
}

ECode DecimalFormat::Constructor(
    /* [in] */ const String& pattern,
    /* [in] */ IDecimalFormatSymbols* symbols)
{
    mSymbols = (IDecimalFormatSymbols*)CoreUtils::Clone(symbols, IID_IDecimalFormatSymbols).Get();
    return Init(pattern);
}

ECode DecimalFormat::Init(
    /* [in] */ const String& pattern)
{
    NativeDecimalFormat* ndf = new NativeDecimalFormat();
    ECode ec = ndf->Constructor(pattern, mSymbols);
    if (FAILED(ec)) {
        delete ndf;
        return ec;
    }

    mNativeDF = reinterpret_cast<HANDLE>(ndf);
    NumberFormat::SetMaximumFractionDigits(ndf->GetMaximumFractionDigits());
    NumberFormat::SetMaximumIntegerDigits(ndf->GetMaximumIntegerDigits());
    NumberFormat::SetMinimumFractionDigits(ndf->GetMinimumFractionDigits());
    NumberFormat::SetMinimumIntegerDigits(ndf->GetMinimumIntegerDigits());
    return NOERROR;
}

ECode DecimalFormat::Format(
    /* [in] */ IInterface* number,
    /* [in, out] */ IStringBuffer* toAppendTo,
    /* [in] */ IFieldPosition* pos)
{
    Integer bitLen;
    if (ILong::Probe(number) != nullptr ||
        IInteger::Probe(number) != nullptr ||
        IShort::Probe(number) != nullptr ||
        IByte::Probe(number) != nullptr ||
        IAtomicInteger::Probe(number) != nullptr ||
        IAtomicLong::Probe(number) != nullptr ||
        (IBigInteger::Probe(number) != nullptr &&
        (IBigInteger::Probe(number)->BitLength(bitLen), bitLen < 64))) {
        Long lv;
        INumber::Probe(number)->LongValue(lv);
        return Format(lv, toAppendTo, pos);
    }
    else if (IBigDecimal::Probe(number) != nullptr) {
        return Format(IBigDecimal::Probe(number), toAppendTo, pos);
    }
    else if (IBigInteger::Probe(number) != nullptr) {
        return Format(IBigInteger::Probe(number), toAppendTo, pos);
    }
    else if (INumber::Probe(number)) {
        Double dv;
        INumber::Probe(number)->DoubleValue(dv);
        return Format(dv, toAppendTo, pos);
    }
    else {
        Logger::E("DecimalFormat", "Cannot format given Object as a Number");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
}

ECode DecimalFormat::Format(
    /* [in] */ Double number,
    /* [in, out] */ IStringBuffer* toAppendTo,
    /* [in] */ IFieldPosition* pos)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    Array<Char> result;
    FAIL_RETURN(ndf->FormatDouble(number, pos, &result));
    toAppendTo->Append(result);
    return NOERROR;
}

ECode DecimalFormat::Format(
    /* [in] */ Long number,
    /* [in, out] */ IStringBuffer* toAppendTo,
    /* [in] */ IFieldPosition* pos)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    Array<Char> result;
    FAIL_RETURN(ndf->FormatLong(number, pos, &result));
    toAppendTo->Append(result);
    return NOERROR;
}

ECode DecimalFormat::Format(
    /* [in] */ IBigDecimal* number,
    /* [in, out] */ IStringBuffer* toAppendTo,
    /* [in] */ IFieldPosition* pos)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    Array<Char> result;
    FAIL_RETURN(ndf->FormatBigDecimal(number, pos, &result));
    toAppendTo->Append(result);
    return NOERROR;
}

ECode DecimalFormat::Format(
    /* [in] */ IBigInteger* number,
    /* [in, out] */ IStringBuffer* toAppendTo,
    /* [in] */ IFieldPosition* pos)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    Array<Char> result;
    Integer bitlen;
    if (number->BitLength(bitlen), bitlen < 64) {
        Long lv;
        INumber::Probe(number)->LongValue(lv);
        FAIL_RETURN(ndf->FormatLong(lv, pos, &result));
    }
    else {
        FAIL_RETURN(ndf->FormatBigInteger(number, pos, &result));
    }
    toAppendTo->Append(result);
    return NOERROR;
}

ECode DecimalFormat::FormatToCharacterIterator(
    /* [in] */ IInterface* obj,
    /* [out] */ AutoPtr<IAttributedCharacterIterator>& it)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    if (obj == nullptr) {
        Logger::E("DecimalFormat", "object == null");
        return E_NULL_POINTER_EXCEPTION;
    }
    return ndf->FormatToCharacterIterator(obj, it);
}

ECode DecimalFormat::Parse(
    /* [in] */ const String& source,
    /* [in] */ IParsePosition* pos,
    /* [out] */ AutoPtr<INumber>& number)
{
    Integer index;
    pos->GetIndex(index);
    if (index < 0 || index >= source.GetLength()) {
        number = nullptr;
        return NOERROR;
    }

    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    AutoPtr<INumber> n;
    ndf->Parse(source, pos, &n);
    if (n == nullptr) {
        number = nullptr;
        return NOERROR;
    }
    Boolean isParseBigDecimal;
    if (IsParseBigDecimal(isParseBigDecimal), isParseBigDecimal) {
        if (ILong::Probe(n) != nullptr) {
            Long lv;
            n->LongValue(lv);
            number = nullptr;
            return CBigDecimal::New(lv, IID_INumber, (IInterface**)&number);
        }
        IDouble* db = IDouble::Probe(n);
        Boolean value;
        if (db != nullptr && (db->IsInfinite(value), !value) &&
                (db->IsNaN(value), !value)) {
            String str = Object::ToString(n);
            number = nullptr;
            return CBigDecimal::New(str, IID_INumber, (IInterface**)&number);

        }
        if (IBigInteger::Probe(n) != nullptr) {
            String str = Object::ToString(n);
            number = nullptr;
            return CBigDecimal::New(str, IID_INumber, (IInterface**)&number);
        }
        number = std::move(n);
        return NOERROR;
    }

    if (IBigDecimal::Probe(n) != nullptr || IBigInteger::Probe(n) != nullptr) {
        Double dv;
        n->DoubleValue(dv);
        number = nullptr;
        return CDouble::New(dv, IID_INumber, (IInterface**)&number);
    }

    Boolean isParseIntegerOnly;
    if ((IsParseIntegerOnly(isParseIntegerOnly), isParseIntegerOnly) &&
            Object::Equals(n, Get_NEGATIVE_ZERO_DOUBLE())) {
        number = nullptr;
        return CLong::New(0ll, IID_INumber, (IInterface**)&number);
    }
    number = std::move(n);
    return NOERROR;
}

ECode DecimalFormat::GetDecimalFormatSymbols(
    /* [out] */ AutoPtr<IDecimalFormatSymbols>& symbols)
{
    symbols = (IDecimalFormatSymbols*)CoreUtils::Clone(
            mSymbols, IID_IDecimalFormatSymbols).Get();
    return NOERROR;
}

ECode DecimalFormat::SetDecimalFormatSymbols(
    /* [in] */ IDecimalFormatSymbols* symbols)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    mSymbols = (IDecimalFormatSymbols*)CoreUtils::Clone(
            symbols, IID_IDecimalFormatSymbols).Get();
    ndf->SetDecimalFormatSymbols(mSymbols);
    return NOERROR;
}

ECode DecimalFormat::GetPositivePrefix(
    /* [out] */ String& prefix)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    return ndf->GetPositivePrefix(&prefix);
}

ECode DecimalFormat::SetPositivePrefix(
    /* [in] */ const String& prefix)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    return ndf->SetPositivePrefix(prefix);
}

ECode DecimalFormat::GetNegativePrefix(
    /* [out] */ String& prefix)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    return ndf->GetNegativePrefix(&prefix);
}

ECode DecimalFormat::SetNegativePrefix(
    /* [in] */ const String& prefix)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    return ndf->SetNegativePrefix(prefix);
}

ECode DecimalFormat::GetPositiveSuffix(
    /* [out] */ String& suffix)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    return ndf->GetPositiveSuffix(&suffix);
}

ECode DecimalFormat::SetPositiveSuffix(
    /* [in] */ const String& suffix)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    return ndf->SetPositiveSuffix(suffix);
}

ECode DecimalFormat::GetNegativeSuffix(
    /* [out] */ String& suffix)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    return ndf->GetNegativeSuffix(&suffix);
}

ECode DecimalFormat::SetNegativeSuffix(
    /* [in] */ const String& suffix)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    return ndf->SetNegativeSuffix(suffix);
}

ECode DecimalFormat::GetMultiplier(
    /* [out] */ Integer& multiplier)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    multiplier = ndf->GetMultiplier();
    return NOERROR;
}

ECode DecimalFormat::SetMultiplier(
    /* [in] */ Integer multiplier)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    return ndf->SetMultiplier(multiplier);
}

ECode DecimalFormat::GetGroupingSize(
    /* [out] */ Integer& size)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    size = ndf->GetGroupingSize();
    return NOERROR;
}

ECode DecimalFormat::SetGroupingSize(
    /* [in] */ Integer size)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    return ndf->SetGroupingSize(size);
}

ECode DecimalFormat::IsGroupingUsed(
    /* [out] */ Boolean& used)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    used = ndf->IsGroupingUsed();
    return NOERROR;
}

ECode DecimalFormat::SetGroupingUsed(
    /* [in] */ Boolean used)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    return ndf->SetGroupingUsed(used);
}

ECode DecimalFormat::IsDecimalSeparatorAlwaysShown(
    /* [in] */ Boolean& shown)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    shown = ndf->IsDecimalSeparatorAlwaysShown();
    return NOERROR;
}

ECode DecimalFormat::SetDecimalSeparatorAlwaysShown(
    /* [in] */ Boolean shown)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    return ndf->SetDecimalSeparatorAlwaysShown(shown);
}

ECode DecimalFormat::IsParseBigDecimal(
    /* [out] */ Boolean& value)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    value = ndf->IsParseBigDecimal();
    return NOERROR;
}

ECode DecimalFormat::SetParseBigDecimal(
    /* [in] */ Boolean value)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    return ndf->SetParseBigDecimal(value);
}

ECode DecimalFormat::SetParseIntegerOnly(
    /* [in] */ Boolean value)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    NumberFormat::SetParseIntegerOnly(value);
    return ndf->SetParseIntegerOnly(value);
}

ECode DecimalFormat::IsParseIntegerOnly(
    /* [out] */ Boolean& value)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    value = ndf->IsParseIntegerOnly();
    return NOERROR;
}

ECode DecimalFormat::CloneImpl(
    /* [in] */ IDecimalFormat* newObj)
{
    NumberFormat::CloneImpl(INumberFormat::Probe(newObj));

    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    DecimalFormat* df = (DecimalFormat*)newObj;

    df->mMaximumIntegerDigits = mMaximumIntegerDigits;
    df->mMinimumIntegerDigits = mMinimumIntegerDigits;
    df->mMaximumFractionDigits = mMaximumFractionDigits;
    df->mMinimumFractionDigits = mMinimumFractionDigits;

    df->mRoundingMode = mRoundingMode;
    df->mSymbols = (IDecimalFormatSymbols*)CoreUtils::Clone(
            mSymbols, IID_IDecimalFormatSymbols).Get();
    df->mNativeDF = reinterpret_cast<HANDLE>(ndf->Clone());
    return NOERROR;
}

ECode DecimalFormat::Equals(
    /* [in] */ IInterface* obj,
    /* [out] */ Boolean& same)
{
    DecimalFormat* df = (DecimalFormat*)IDecimalFormat::Probe(obj);
    if (df == nullptr) {
        same = false;
        return NOERROR;
    }
    if (this == df) {
        same = true;
        return NOERROR;
    }

    NativeDecimalFormat* thisNdf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);
    NativeDecimalFormat* thatNdf = reinterpret_cast<NativeDecimalFormat*>(df->mNativeDF);

    same = thisNdf->Equals(thatNdf) && Object::Equals(mSymbols, df->mSymbols);
    return NOERROR;
}

ECode DecimalFormat::GetHashCode(
    /* [out] */ Integer& hash)
{
    String prefix;
    GetPositivePrefix(prefix);

    NumberFormat::GetHashCode(hash);
    hash = hash * 37 + prefix.GetHashCode();
    return NOERROR;
}

ECode DecimalFormat::ToPattern(
    /* [out] */ String& pattern)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    return ndf->ToPattern(&pattern);
}

ECode DecimalFormat::ToLocalizedPattern(
    /* [out] */ String& pattern)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    return ndf->ToLocalizedPattern(&pattern);
}

ECode DecimalFormat::ApplyPattern(
    /* [in] */ const String& pattern)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    FAIL_RETURN(ndf->ApplyPattern(pattern));
    UpdateFieldsFromNative();
    return NOERROR;
}

ECode DecimalFormat::ApplyLocalizedPattern(
    /* [in] */ const String& pattern)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    FAIL_RETURN(ndf->ApplyLocalizedPattern(pattern));
    UpdateFieldsFromNative();
    return NOERROR;
}

void DecimalFormat::UpdateFieldsFromNative()
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    mMaximumFractionDigits = ndf->GetMaximumFractionDigits();
    mMaximumIntegerDigits = ndf->GetMaximumIntegerDigits();
    mMinimumFractionDigits = ndf->GetMinimumFractionDigits();
    mMinimumIntegerDigits = ndf->GetMinimumIntegerDigits();
}

ECode DecimalFormat::SetMaximumIntegerDigits(
    /* [in] */ Integer newValue)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    mMaximumIntegerDigits = Math::Min(Math::Max(0, newValue), MAXIMUM_INTEGER_DIGITS);
    NumberFormat::SetMinimumIntegerDigits((mMaximumIntegerDigits > DOUBLE_INTEGER_DIGITS) ?
                DOUBLE_INTEGER_DIGITS : mMaximumIntegerDigits);
    if (mMinimumIntegerDigits > mMaximumIntegerDigits) {
        mMinimumIntegerDigits = mMaximumIntegerDigits;
        NumberFormat::SetMinimumIntegerDigits((mMinimumIntegerDigits > DOUBLE_INTEGER_DIGITS) ?
                DOUBLE_INTEGER_DIGITS : mMinimumIntegerDigits);
    }
    Integer value;
    GetMaximumIntegerDigits(value);
    ndf->SetMaximumIntegerDigits(value);
    return NOERROR;
}

ECode DecimalFormat::SetMinimumIntegerDigits(
    /* [in] */ Integer newValue)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    mMinimumIntegerDigits = Math::Min(Math::Max(0, newValue), MAXIMUM_INTEGER_DIGITS);
    NumberFormat::SetMinimumIntegerDigits((mMinimumIntegerDigits > DOUBLE_INTEGER_DIGITS) ?
            DOUBLE_INTEGER_DIGITS : mMinimumIntegerDigits);
    if (mMinimumIntegerDigits > mMaximumIntegerDigits) {
        mMaximumIntegerDigits = mMinimumIntegerDigits;
        NumberFormat::SetMaximumIntegerDigits((mMaximumIntegerDigits > DOUBLE_INTEGER_DIGITS) ?
                DOUBLE_INTEGER_DIGITS : mMaximumIntegerDigits);
    }
    Integer value;
    GetMinimumIntegerDigits(value);
    ndf->SetMinimumIntegerDigits(value);
    return NOERROR;
}

ECode DecimalFormat::SetMaximumFractionDigits(
    /* [in] */ Integer newValue)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    mMaximumFractionDigits = Math::Min(Math::Max(0, newValue), MAXIMUM_FRACTION_DIGITS);
    NumberFormat::SetMaximumFractionDigits((mMaximumFractionDigits > DOUBLE_FRACTION_DIGITS) ?
            DOUBLE_FRACTION_DIGITS : mMaximumFractionDigits);
    if (mMinimumFractionDigits > mMaximumFractionDigits) {
        mMinimumFractionDigits = mMaximumFractionDigits;
        NumberFormat::SetMinimumFractionDigits((mMinimumFractionDigits > DOUBLE_FRACTION_DIGITS) ?
                DOUBLE_FRACTION_DIGITS : mMinimumFractionDigits);
    }
    Integer value;
    GetMaximumFractionDigits(value);
    ndf->SetMaximumFractionDigits(value);
    SetRoundingMode(mRoundingMode);
    return NOERROR;
}

ECode DecimalFormat::SetMinimumFractionDigits(
    /* [in] */ Integer newValue)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    mMinimumFractionDigits = Math::Min(Math::Max(0, newValue), MAXIMUM_FRACTION_DIGITS);
    NumberFormat::SetMinimumFractionDigits((mMinimumFractionDigits > DOUBLE_FRACTION_DIGITS) ?
            DOUBLE_FRACTION_DIGITS : mMinimumFractionDigits);
    if (mMinimumFractionDigits > mMaximumFractionDigits) {
        mMaximumFractionDigits = mMinimumFractionDigits;
        NumberFormat::SetMaximumFractionDigits((mMaximumFractionDigits > DOUBLE_FRACTION_DIGITS) ?
                DOUBLE_FRACTION_DIGITS : mMaximumFractionDigits);
    }
    Integer value;
    GetMinimumFractionDigits(value);
    ndf->SetMinimumFractionDigits(value);
    return NOERROR;
}

ECode DecimalFormat::GetMaximumIntegerDigits(
    /* [out] */ Integer& value)
{
    value = mMaximumIntegerDigits;
    return NOERROR;
}

ECode DecimalFormat::GetMinimumIntegerDigits(
    /* [out] */ Integer& value)
{
    value = mMinimumIntegerDigits;
    return NOERROR;
}

ECode DecimalFormat::GetMaximumFractionDigits(
    /* [out] */ Integer& value)
{
    value = mMaximumFractionDigits;
    return NOERROR;
}

ECode DecimalFormat::GetMinimumFractionDigits(
    /* [out] */ Integer& value)
{
    value = mMinimumFractionDigits;
    return NOERROR;
}

ECode DecimalFormat::GetCurrency(
    /* [out] */ AutoPtr<ICurrency>& currency)
{
    return mSymbols->GetCurrency(currency);
}

ECode DecimalFormat::SetCurrency(
    /* [in] */ ICurrency* currency)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    String currencyCode;
    currency->GetCurrencyCode(currencyCode);
    AutoPtr<ICurrency> instance = Currency::GetInstance(currencyCode);
    mSymbols->SetCurrency(instance);

    String currencySymbol;
    mSymbols->GetCurrencySymbol(currencySymbol);
    return ndf->SetCurrency(currencySymbol, currencyCode);
}

ECode DecimalFormat::GetRoundingMode(
    /* [out] */ RoundingMode& mode)
{
    mode = mRoundingMode;
    return NOERROR;
}

ECode DecimalFormat::SetRoundingMode(
    /* [in] */ RoundingMode mode)
{
    NativeDecimalFormat* ndf = reinterpret_cast<NativeDecimalFormat*>(mNativeDF);

    mRoundingMode = mode;

    // DecimalFormat does not allow specification of a rounding increment.  If anything other
    // than 0.0 is used here the resulting DecimalFormat cannot be deserialized because the
    // serialization format does not include rounding increment information.
    return ndf->SetRoundingMode(mRoundingMode, 0.0);
}

void DecimalFormat::AdjustForCurrencyDefaultFractionDigits()
{
    AutoPtr<ICurrency> currency;
    mSymbols->GetCurrency(currency);
    if (currency == nullptr) {
        String currencySymbol;
        mSymbols->GetInternationalCurrencySymbol(currencySymbol);
        currency = Currency::GetInstance(currencySymbol);
    }
    if (currency != nullptr) {
        Integer digits;
        currency->GetDefaultFractionDigits(digits);
        if (digits != -1) {
            Integer oldMinDigits;
            GetMinimumFractionDigits(oldMinDigits);
            // Common patterns are "#.##", "#.00", "#".
            // Try to adjust all of them in a reasonable way.
            Integer oldMaxDigits;
            GetMaximumFractionDigits(oldMaxDigits);
            if (oldMinDigits == oldMaxDigits) {
                SetMinimumFractionDigits(digits);
                SetMaximumFractionDigits(digits);
            }
            else {
                SetMinimumFractionDigits(Math::Min(digits, oldMinDigits));
                SetMaximumFractionDigits(digits);
            }
        }
    }
}

}
}
