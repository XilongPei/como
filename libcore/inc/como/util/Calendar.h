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

#ifndef __COMO_UTIL_CALENDAR_H__
#define __COMO_UTIL_CALENDAR_H__

#include "como/core/SyncObject.h"
#include "como.core.ICloneable.h"
#include "como.core.IComparable.h"
#include "como.core.IStringBuilder.h"
#include "como.io.ISerializable.h"
#include "como.text.IDateFormatSymbols.h"
#include "como.util.ICalendar.h"
#include "como.util.IDate.h"
#include "como.util.ILocale.h"
#include "como.util.IMap.h"
#include "como.util.ITimeZone.h"
#include "como.util.concurrent.IConcurrentMap.h"

using como::core::ICloneable;
using como::core::IComparable;
using como::core::IStringBuilder;
using como::core::SyncObject;
using como::io::ISerializable;
using como::text::IDateFormatSymbols;
using como::util::IMap;
using como::util::concurrent::IConcurrentMap;

namespace como {
namespace util {

class Calendar
    : public SyncObject
    , public ICalendar
    , public ISerializable
    , public ICloneable
    , public IComparable
{
public:
    COMO_INTERFACE_DECL();

    ECode Constructor();

    ECode Constructor(
        /* [in] */ ITimeZone* zone,
        /* [in] */ ILocale* locale);

    static AutoPtr<ICalendar> GetInstance();

    static AutoPtr<ICalendar> GetInstance(
        /* [in] */ ITimeZone* zone);

    static AutoPtr<ICalendar> GetInstance(
        /* [in] */ ILocale* aLocale);

    static AutoPtr<ICalendar> GetInstance(
        /* [in] */ ITimeZone* zone,
        /* [in] */ ILocale* aLocale);

    static AutoPtr<ICalendar> GetJapaneseImperialInstance(
        /* [in] */ ITimeZone* zone,
        /* [in] */ ILocale* aLocale);

    static Array<ILocale*> GetAvailableLocales();

    ECode GetTime(
        /* [out] */ AutoPtr<IDate>& date) override final;

    ECode SetTime(
        /* [in] */ IDate* date) override final;

    ECode GetTimeInMillis(
        /* [out] */ Long& time) override;

    ECode SetTimeInMillis(
        /* [in] */ Long millis) override;

    ECode Get(
        /* [in] */ Integer field,
        /* [out] */ Integer& value) override;

    ECode Set(
        /* [in] */ Integer field,
        /* [in] */ Integer value) override;

    ECode Set(
        /* [in] */ Integer year,
        /* [in] */ Integer month,
        /* [in] */ Integer date) override final;

    ECode Set(
        /* [in] */ Integer year,
        /* [in] */ Integer month,
        /* [in] */ Integer date,
        /* [in] */ Integer hourOfDay,
        /* [in] */ Integer minute) override final;

    ECode Set(
        /* [in] */ Integer year,
        /* [in] */ Integer month,
        /* [in] */ Integer date,
        /* [in] */ Integer hourOfDay,
        /* [in] */ Integer minute,
        /* [in] */ Integer second) override final;

    ECode Clear() override final;

    ECode Clear(
        /* [in] */ Integer field) override final;

    ECode IsSet(
        /* [in] */ Integer field,
        /* [out] */ Boolean& result) override final;

    ECode GetDisplayName(
        /* [in] */ Integer field,
        /* [in] */ Integer style,
        /* [in] */ ILocale* locale,
        /* [out] */ String& name) override;

    ECode GetDisplayNames(
        /* [in] */ Integer field,
        /* [in] */ Integer style,
        /* [in] */ ILocale* locale,
        /* [out] */ AutoPtr<IMap>& names) override;

    ECode CheckDisplayNameParams(
        /* [in] */ Integer field,
        /* [in] */ Integer style,
        /* [in] */ Integer minStyle,
        /* [in] */ Integer maxStyle,
        /* [in] */ ILocale* locale,
        /* [in] */ Integer fieldMask,
        /* [out] */ Boolean* result);

    Boolean IsExternallySet(
        /* [in] */ Integer field);

    Integer GetSetStateFields();

    void SetFieldsComputed(
        /* [in] */ Integer fieldMask);

    void SetFieldsNormalized(
        /* [in] */ Integer fieldMask);

    Boolean IsPartiallyNormalized();

    Boolean IsFullyNormalized();

    void SetUnnormalized();

    static Boolean IsFieldSet(
        /* [in] */ Integer fieldMask,
        /* [in] */ Integer field);

    Integer SelectFields();

    Integer GetBaseStyle(
        /* [in] */ Integer style);

    ECode GetCalendarType(
        /* [out] */ String& type) override;

    ECode Equals(
        /* [in] */ IInterface* obj,
        /* [out] */ Boolean& same) override;

    ECode GetHashCode(
        /* [out] */ Integer& hash) override;

    ECode Before(
        /* [in] */ IInterface* when,
        /* [out] */ Boolean& before) override;

    ECode After(
        /* [in] */ IInterface* when,
        /* [out] */ Boolean& after) override;

    ECode CompareTo(
        /* [in] */ IInterface* another,
        /* [out] */ Integer& result) override;

    ECode Roll(
        /* [in] */ Integer field,
        /* [in] */ Integer amount) override;

    ECode SetTimeZone(
        /* [in] */ ITimeZone* value) override;

    ECode GetTimeZone(
        /* [out] */ AutoPtr<ITimeZone>& zone) override;

    virtual AutoPtr<ITimeZone> GetZone();

    void SetZoneShared(
        /* [in] */ Boolean shared);

    ECode SetLenient(
        /* [in] */ Boolean lenient) override;

    ECode IsLenient(
        /* [out] */ Boolean& lenient) override;

    ECode SetFirstDayOfWeek(
        /* [in] */ Integer value) override;

    ECode GetFirstDayOfWeek(
        /* [out] */ Integer& value) override;

    ECode SetMinimalDaysInFirstWeek(
        /* [in] */ Integer value) override;

    ECode GetMinimalDaysInFirstWeek(
        /* [out] */ Integer& value) override;

    ECode IsWeekDateSupported(
        /* [out] */ Boolean& supported) override;

    ECode GetWeekYear(
        /* [out] */ Integer& weekYear) override;

    ECode SetWeekDate(
        /* [in] */ Integer weekYear,
        /* [in] */ Integer weekOfYear,
        /* [in] */ Integer dayOfWeek) override;

    ECode GetWeeksInWeekYear(
        /* [out] */ Integer& weeks) override;

    ECode GetActualMinimum(
        /* [in] */ Integer field,
        /* [out] */ Integer& value) override;

    ECode GetActualMaximum(
        /* [in] */ Integer field,
        /* [out] */ Integer& value) override;

    static String GetFieldName(
        /* [in] */ Integer field);

    ECode ToString(
        /* [out] */ String& desc) override;

protected:
    virtual ECode ComputeTime() = 0;

    virtual ECode ComputeFields() = 0;

    Integer InternalGet(
        /* [in] */ Integer field);

    void InternalSet(
        /* [in] */ Integer field,
        /* [in] */ Integer value);

    virtual ECode Complete();

    ECode CloneImpl(
        /* [in] */ ICalendar* newObj);

private:
    static AutoPtr<ICalendar> CreateCalendar(
        /* [in] */ ITimeZone* zone,
        /* [in] */ ILocale* aLocale);

    AutoPtr<IMap> GetDisplayNamesImpl(
        /* [in] */ Integer field,
        /* [in] */ Integer style,
        /* [in] */ ILocale* locale);

    Array<String> GetFieldStrings(
        /* [in] */ Integer field,
        /* [in] */ Integer style,
        /* [in] */ IDateFormatSymbols* symbols);

    Integer ToStandaloneStyle(
        /* [in] */ Integer style);

    Boolean IsStandaloneStyle(
        /* [in] */ Integer style);

    Boolean IsNarrowStyle(
        /* [in]*/ Integer style);

    Boolean IsNarrowFormatStyle(
        /* [in] */ Integer style);

    static Integer AggregateStamp(
        /* [in] */ Integer stamp_a,
        /* [in] */ Integer stamp_b);

    static Array<String>& GetFIELD_NAME();

    static void AppendValue(
        /* [in] */ IStringBuilder* sb,
        /* [in] */ const String& item,
        /* [in] */ Boolean valid,
        /* [in] */ Long value);

    void SetWeekCountData(
        /* [in] */ ILocale* desiredLocale);

    ECode UpdateTime();

    Integer CompareTo(
        /* [in] */ Long t);

    static Long GetMillisOf(
        /* [in] */ Calendar* calendar);

    void AdjustStamp();

    void InvalidateWeekFields();

    static AutoPtr<IConcurrentMap> GetCachedLocaleData();

    static SyncObject& GetClassLock();

public:
    static constexpr Integer ALL_FIELDS = (1 << FIELD_COUNT) - 1;

    static constexpr Integer ERA_MASK = (1 << ERA);
    static constexpr Integer YEAR_MASK = (1 << YEAR);
    static constexpr Integer MONTH_MASK = (1 << MONTH);
    static constexpr Integer WEEK_OF_YEAR_MASK = (1 << WEEK_OF_YEAR);
    static constexpr Integer WEEK_OF_MONTH_MASK = (1 << WEEK_OF_MONTH);
    static constexpr Integer DAY_OF_MONTH_MASK = (1 << DAY_OF_MONTH);
    static constexpr Integer DATE_MASK = DAY_OF_MONTH_MASK;
    static constexpr Integer DAY_OF_YEAR_MASK = (1 << DAY_OF_YEAR);
    static constexpr Integer DAY_OF_WEEK_MASK = (1 << DAY_OF_WEEK);
    static constexpr Integer DAY_OF_WEEK_IN_MONTH_MASK = (1 << DAY_OF_WEEK_IN_MONTH);
    static constexpr Integer AM_PM_MASK = (1 << AM_PM);
    static constexpr Integer HOUR_MASK = (1 << HOUR);
    static constexpr Integer HOUR_OF_DAY_MASK = (1 << HOUR_OF_DAY);
    static constexpr Integer MINUTE_MASK = (1 << MINUTE);
    static constexpr Integer SECOND_MASK = (1 << SECOND);
    static constexpr Integer MILLISECOND_MASK = (1 << MILLISECOND);
    static constexpr Integer ZONE_OFFSET_MASK = (1 << ZONE_OFFSET);
    static constexpr Integer DST_OFFSET_MASK = (1 << DST_OFFSET);

protected:
    Array<Integer> mFields;

    Array<Boolean> mIsSet;

    Long mTime = 0;

    Boolean mIsTimeSet = false;

    Boolean mAreFieldsSet = false;

    Boolean mAreAllFieldsSet = false;

private:
    Array<Integer> mStamp;

    Boolean mLenient = true;

    AutoPtr<ITimeZone> mZone;

    Boolean mSharedZone = false;

    Integer mFirstDayOfWeek = 0;

    Integer mMinimalDaysInFirstWeek = 0;

    static constexpr Integer UNSET = 0;

    static constexpr Integer COMPUTED = 1;

    static constexpr Integer MINIMUM_USER_STAMP = 2;

    Integer mNextStamp = MINIMUM_USER_STAMP;
};

inline Integer Calendar::InternalGet(
    /* [in] */ Integer field)
{
    return mFields[field];
}

inline void Calendar::InternalSet(
    /* [in] */ Integer field,
    /* [in] */ Integer value)
{
    mFields[field] = value;
}

inline Boolean Calendar::IsExternallySet(
    /* [in] */ Integer field)
{
    return mStamp[field] >= MINIMUM_USER_STAMP;
}

inline Boolean Calendar::IsPartiallyNormalized()
{
    return mAreFieldsSet && !mAreAllFieldsSet;
}

inline Boolean Calendar::IsFullyNormalized()
{
    return mAreFieldsSet && mAreAllFieldsSet;
}

inline void Calendar::SetUnnormalized()
{
    mAreFieldsSet = mAreAllFieldsSet = false;
}

inline Boolean Calendar::IsFieldSet(
    /* [in] */ Integer fieldMask,
    /* [in] */ Integer field)
{
    return (fieldMask & (1 << field)) != 0;
}

inline Integer Calendar::GetBaseStyle(
    /* [in] */ Integer style)
{
    return style & ~STANDALONE_MASK;
}

inline Integer Calendar::ToStandaloneStyle(
    /* [in] */ Integer style)
{
    return style | STANDALONE_MASK;
}

inline Boolean Calendar::IsStandaloneStyle(
    /* [in] */ Integer style)
{
    return (style & STANDALONE_MASK) != 0;
}

inline Boolean Calendar::IsNarrowStyle(
    /* [in]*/ Integer style)
{
    return style == NARROW_FORMAT || style == NARROW_STANDALONE;
}

inline Boolean Calendar::IsNarrowFormatStyle(
    /* [in] */ Integer style)
{
    return style == NARROW_FORMAT;
}

inline Integer Calendar::AggregateStamp(
    /* [in] */ Integer stamp_a,
    /* [in] */ Integer stamp_b)
{
    if (stamp_a == UNSET || stamp_b == UNSET) {
        return UNSET;
    }
    return (stamp_a > stamp_b) ? stamp_a : stamp_b;
}

inline void Calendar::SetZoneShared(
    /* [in] */ Boolean shared)
{
    mSharedZone = shared;
}

}
}

#endif // __COMO_UTIL_CALENDAR_H__
