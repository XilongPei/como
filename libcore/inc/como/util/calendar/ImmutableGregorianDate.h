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

#ifndef __COMO_UTIL_CALENDAR_IMMUTABLEGREGORIANDATE_H__
#define __COMO_UTIL_CALENDAR_IMMUTABLEGREGORIANDATE_H__

#include "como/util/calendar/BaseCalendar.h"
#include "como.util.calendar.IBaseCalendarDate.h"

namespace como {
namespace util {
namespace calendar {

class ImmutableGregorianDate
    : public BaseCalendar::Date
{
public:
    IInterface* Probe(
        /* [in] */ const InterfaceID& iid) override;

    ECode Constructor(
        /* [in] */ IBaseCalendarDate* date);

    ECode GetEra(
        /* [out] */ AutoPtr<IEra>& era) override;

    ECode SetEra(
        /* [in] */ IEra* era) override;

    ECode GetYear(
        /* [out] */ Integer& year) override;

    ECode SetYear(
        /* [in] */ Integer year) override;

    ECode AddYear(
        /* [in] */ Integer n) override;

    ECode IsLeapYear(
        /* [out] */ Boolean& leapYear) override;

    ECode SetLeapYear(
        /* [in] */ Boolean leapYear) override;

    ECode GetMonth(
        /* [out] */ Integer& month) override;

    ECode SetMonth(
        /* [in] */ Integer month) override;

    ECode AddMonth(
        /* [in] */ Integer n) override;

    ECode GetDayOfMonth(
        /* [out] */ Integer& date) override;

    ECode SetDayOfMonth(
        /* [in] */ Integer date) override;

    ECode AddDayOfMonth(
        /* [in] */ Integer n) override;

    ECode GetDayOfWeek(
        /* [out] */ Integer& date) override;

    ECode GetHours(
        /* [out] */ Integer& hours) override;

    ECode SetHours(
        /* [in] */ Integer hours) override;

    ECode AddHours(
        /* [in] */ Integer n) override;

    ECode GetMinutes(
        /* [out] */ Integer& minutes) override;

    ECode SetMinutes(
        /* [in] */ Integer minutes) override;

    ECode AddMinutes(
        /* [in] */ Integer n) override;

    ECode GetSeconds(
        /* [out] */ Integer& seconds) override;

    ECode SetSeconds(
        /* [in] */ Integer seconds) override;

    ECode AddSeconds(
        /* [in] */ Integer n) override;

    ECode GetMillis(
        /* [out] */ Integer& millis) override;

    ECode SetMillis(
        /* [in] */ Integer millis) override;

    ECode AddMillis(
        /* [in] */ Integer n) override;

    ECode GetTimeOfDay(
        /* [out] */ Long& date) override;

    ECode SetDate(
        /* [in] */ Integer year,
        /* [in] */ Integer month,
        /* [in] */ Integer dayOfMonth) override;

    ECode AddDate(
        /* [in] */ Integer year,
        /* [in] */ Integer month,
        /* [in] */ Integer dayOfMonth) override;

    ECode SetTimeOfDay(
        /* [in] */ Integer hours,
        /* [in] */ Integer minutes,
        /* [in] */ Integer seconds,
        /* [in] */ Integer millis) override;

    ECode AddTimeOfDay(
        /* [in] */ Integer hours,
        /* [in] */ Integer minutes,
        /* [in] */ Integer seconds,
        /* [in] */ Integer millis) override;

    ECode SetTimeOfDay(
        /* [in] */ Long fraction) override;

    ECode IsNormalized(
        /* [out] */ Boolean& normalized) override;

    ECode IsStandardTime(
        /* [out] */ Boolean& standardTime) override;

    ECode SetStandardTime(
        /* [in] */ Boolean standardTime) override;

    ECode IsDaylightTime(
        /* [out] */ Boolean& daylightTime) override;

    ECode GetZone(
        /* [out] */ AutoPtr<ITimeZone>& zone) override;

    ECode SetZone(
        /* [in] */ ITimeZone* zoneinfo) override;

    ECode IsSameDate(
        /* [in] */ ICalendarDate* date,
        /* [out] */ Boolean& same) override;

    ECode Equals(
        /* [in] */ IInterface* obj,
        /* [out] */ Boolean& same) override;

    ECode GetHashCode(
        /* [out] */ Integer& hash) override;

    ECode Clone(
        /* [in] */ const InterfaceID& iid,
        /* [out] */ IInterface** obj) override;

    ECode ToString(
        /* [out] */ String& desc) override;

    ECode GetZoneOffset(
        /* [out] */ Integer& offset) override;

    ECode GetDaylightSaving(
        /* [out] */ Integer& ds) override;

    ECode GetNormalizedYear(
        /* [out] */ Integer& normalizedYear) override;

    ECode SetNormalizedYear(
        /* [in] */ Integer normalizedYear) override;

protected:
    ECode SetLocale(
        /* [in] */ ILocale* loc) override;

    ECode SetDayOfWeek(
        /* [in] */ Integer dayOfWeek) override;

    ECode SetNormalized(
        /* [in] */ Boolean normalized) override;

    ECode SetZoneOffset(
        /* [in] */ Integer offset) override;

    ECode SetDaylightSaving(
        /* [in] */ Integer daylightSaving) override;

private:
    AutoPtr<BaseCalendar::Date> mDate;
};

}
}
}

#endif // __COMO_UTIL_CALENDAR_IMMUTABLEGREGORIANDATE_H__
