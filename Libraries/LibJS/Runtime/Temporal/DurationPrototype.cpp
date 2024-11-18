/*
 * Copyright (c) 2021-2023, Linus Groh <linusg@serenityos.org>
 * Copyright (c) 2024, Tim Flynn <trflynn89@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibJS/Runtime/Temporal/Duration.h>
#include <LibJS/Runtime/Temporal/DurationPrototype.h>

namespace JS::Temporal {

GC_DEFINE_ALLOCATOR(DurationPrototype);

// 7.3 Properties of the Temporal.Duration Prototype Object, https://tc39.es/proposal-temporal/#sec-properties-of-the-temporal-duration-prototype-object
DurationPrototype::DurationPrototype(Realm& realm)
    : PrototypeObject(realm.intrinsics().object_prototype())
{
}

void DurationPrototype::initialize(Realm& realm)
{
    Base::initialize(realm);

    auto& vm = this->vm();

    // 7.3.2 Temporal.Duration.prototype[ %Symbol.toStringTag% ], https://tc39.es/proposal-temporal/#sec-temporal.duration.prototype-%symbol.tostringtag%
    define_direct_property(vm.well_known_symbol_to_string_tag(), PrimitiveString::create(vm, "Temporal.Duration"_string), Attribute::Configurable);

#define __JS_ENUMERATE(unit) \
    define_native_accessor(realm, vm.names.unit, unit##_getter, {}, Attribute::Configurable);
    JS_ENUMERATE_DURATION_UNITS
#undef __JS_ENUMERATE

    define_native_accessor(realm, vm.names.sign, sign_getter, {}, Attribute::Configurable);
    define_native_accessor(realm, vm.names.blank, blank_getter, {}, Attribute::Configurable);

    u8 attr = Attribute::Writable | Attribute::Configurable;
    define_native_function(realm, vm.names.with, with, 1, attr);
    define_native_function(realm, vm.names.negated, negated, 0, attr);
    define_native_function(realm, vm.names.abs, abs, 0, attr);
    define_native_function(realm, vm.names.add, add, 1, attr);
    define_native_function(realm, vm.names.subtract, subtract, 1, attr);
    define_native_function(realm, vm.names.round, round, 1, attr);
    define_native_function(realm, vm.names.toString, to_string, 0, attr);
    define_native_function(realm, vm.names.toJSON, to_json, 0, attr);
    define_native_function(realm, vm.names.toLocaleString, to_locale_string, 0, attr);
}

// 7.3.3 get Temporal.Duration.prototype.years, https://tc39.es/proposal-temporal/#sec-get-temporal.duration.prototype.years
// 7.3.4 get Temporal.Duration.prototype.months, https://tc39.es/proposal-temporal/#sec-get-temporal.duration.prototype.months
// 7.3.5 get Temporal.Duration.prototype.weeks, https://tc39.es/proposal-temporal/#sec-get-temporal.duration.prototype.weeks
// 7.3.6 get Temporal.Duration.prototype.days, https://tc39.es/proposal-temporal/#sec-get-temporal.duration.prototype.days
// 7.3.7 get Temporal.Duration.prototype.hours, https://tc39.es/proposal-temporal/#sec-get-temporal.duration.prototype.hours
// 7.3.8 get Temporal.Duration.prototype.minutes, https://tc39.es/proposal-temporal/#sec-get-temporal.duration.prototype.minutes
// 7.3.9 get Temporal.Duration.prototype.seconds, https://tc39.es/proposal-temporal/#sec-get-temporal.duration.prototype.seconds
// 7.3.10 get Temporal.Duration.prototype.milliseconds, https://tc39.es/proposal-temporal/#sec-get-temporal.duration.prototype.milliseconds
// 7.3.11 get Temporal.Duration.prototype.microseconds, https://tc39.es/proposal-temporal/#sec-get-temporal.duration.prototype.microseconds
// 7.3.12 get Temporal.Duration.prototype.nanoseconds, https://tc39.es/proposal-temporal/#sec-get-temporal.duration.prototype.nanoseconds
#define __JS_ENUMERATE(unit)                                                               \
    JS_DEFINE_NATIVE_FUNCTION(DurationPrototype::unit##_getter)                            \
    {                                                                                      \
        /* 1. Let duration be the this value. */                                           \
        /* 2. Perform ? RequireInternalSlot(duration, [[InitializedTemporalDuration]]). */ \
        auto duration = TRY(typed_this_object(vm));                                        \
                                                                                           \
        /* 3. Return 𝔽(duration.[[<unit>]]). */                                         \
        return duration->unit();                                                           \
    }
JS_ENUMERATE_DURATION_UNITS
#undef __JS_ENUMERATE

// 7.3.13 get Temporal.Duration.prototype.sign, https://tc39.es/proposal-temporal/#sec-get-temporal.duration.prototype.sign
JS_DEFINE_NATIVE_FUNCTION(DurationPrototype::sign_getter)
{
    // 1. Let duration be the this value.
    // 2. Perform ? RequireInternalSlot(duration, [[InitializedTemporalDuration]]).
    auto duration = TRY(typed_this_object(vm));

    // 3. Return 𝔽(DurationSign(duration)).
    return duration_sign(duration);
}

// 7.3.14 get Temporal.Duration.prototype.blank, https://tc39.es/proposal-temporal/#sec-get-temporal.duration.prototype.blank
JS_DEFINE_NATIVE_FUNCTION(DurationPrototype::blank_getter)
{
    // 1. Let duration be the this value.
    // 2. Perform ? RequireInternalSlot(duration, [[InitializedTemporalDuration]]).
    auto duration = TRY(typed_this_object(vm));

    // 3. If DurationSign(duration) = 0, return true.
    if (duration_sign(duration) == 0)
        return true;

    // 4. Return false.
    return false;
}

// 7.3.15 Temporal.Duration.prototype.with ( temporalDurationLike ), https://tc39.es/proposal-temporal/#sec-temporal.duration.prototype.with
JS_DEFINE_NATIVE_FUNCTION(DurationPrototype::with)
{
    // 1. Let duration be the this value.
    // 2. Perform ? RequireInternalSlot(duration, [[InitializedTemporalDuration]]).
    auto duration = TRY(typed_this_object(vm));

    // 3. Let temporalDurationLike be ? ToTemporalPartialDurationRecord(temporalDurationLike).
    auto temporal_duration_like = TRY(to_temporal_partial_duration_record(vm, vm.argument(0)));

    // 4. If temporalDurationLike.[[Years]] is not undefined, then
    //     a. Let years be temporalDurationLike.[[Years]].
    // 5. Else,
    //     a. Let years be duration.[[Years]].
    auto years = temporal_duration_like.years.value_or(duration->years());

    // 6. If temporalDurationLike.[[Months]] is not undefined, then
    //     a. Let months be temporalDurationLike.[[Months]].
    // 7. Else,
    //     a. Let months be duration.[[Months]].
    auto months = temporal_duration_like.months.value_or(duration->months());

    // 8. If temporalDurationLike.[[Weeks]] is not undefined, then
    //     a. Let weeks be temporalDurationLike.[[Weeks]].
    // 9. Else,
    //     a. Let weeks be duration.[[Weeks]].
    auto weeks = temporal_duration_like.weeks.value_or(duration->weeks());

    // 10. If temporalDurationLike.[[Days]] is not undefined, then
    //     a. Let days be temporalDurationLike.[[Days]].
    // 11. Else,
    //     a. Let days be duration.[[Days]].
    auto days = temporal_duration_like.days.value_or(duration->days());

    // 12. If temporalDurationLike.[[Hours]] is not undefined, then
    //     a. Let hours be temporalDurationLike.[[Hours]].
    // 13. Else,
    //     a. Let hours be duration.[[Hours]].
    auto hours = temporal_duration_like.hours.value_or(duration->hours());

    // 14. If temporalDurationLike.[[Minutes]] is not undefined, then
    //     a. Let minutes be temporalDurationLike.[[Minutes]].
    // 15. Else,
    //     a. Let minutes be duration.[[Minutes]].
    auto minutes = temporal_duration_like.minutes.value_or(duration->minutes());

    // 16. If temporalDurationLike.[[Seconds]] is not undefined, then
    //     a. Let seconds be temporalDurationLike.[[Seconds]].
    // 17. Else,
    //     a. Let seconds be duration.[[Seconds]].
    auto seconds = temporal_duration_like.seconds.value_or(duration->seconds());

    // 18. If temporalDurationLike.[[Milliseconds]] is not undefined, then
    //     a. Let milliseconds be temporalDurationLike.[[Milliseconds]].
    // 19. Else,
    //     a. Let milliseconds be duration.[[Milliseconds]].
    auto milliseconds = temporal_duration_like.milliseconds.value_or(duration->milliseconds());

    // 20. If temporalDurationLike.[[Microseconds]] is not undefined, then
    //     a. Let microseconds be temporalDurationLike.[[Microseconds]].
    // 21. Else,
    //     a. Let microseconds be duration.[[Microseconds]].
    auto microseconds = temporal_duration_like.microseconds.value_or(duration->microseconds());

    // 22. If temporalDurationLike.[[Nanoseconds]] is not undefined, then
    //     a. Let nanoseconds be temporalDurationLike.[[Nanoseconds]].
    // 23. Else,
    //     a. Let nanoseconds be duration.[[Nanoseconds]].
    auto nanoseconds = temporal_duration_like.nanoseconds.value_or(duration->nanoseconds());

    // 24. Return ? CreateTemporalDuration(years, months, weeks, days, hours, minutes, seconds, milliseconds, microseconds, nanoseconds).
    return TRY(create_temporal_duration(vm, years, months, weeks, days, hours, minutes, seconds, milliseconds, microseconds, nanoseconds));
}

// 7.3.16 Temporal.Duration.prototype.negated ( ), https://tc39.es/proposal-temporal/#sec-temporal.duration.prototype.negated
JS_DEFINE_NATIVE_FUNCTION(DurationPrototype::negated)
{
    // 1. Let duration be the this value.
    // 2. Perform ? RequireInternalSlot(duration, [[InitializedTemporalDuration]]).
    auto duration = TRY(typed_this_object(vm));

    // 3. Return CreateNegatedTemporalDuration(duration).
    return create_negated_temporal_duration(vm, duration);
}

// 7.3.17 Temporal.Duration.prototype.abs ( ), https://tc39.es/proposal-temporal/#sec-temporal.duration.prototype.abs
JS_DEFINE_NATIVE_FUNCTION(DurationPrototype::abs)
{
    // 1. Let duration be the this value.
    // 2. Perform ? RequireInternalSlot(duration, [[InitializedTemporalDuration]]).
    auto duration = TRY(typed_this_object(vm));

    // 3. Return ! CreateTemporalDuration(abs(duration.[[Years]]), abs(duration.[[Months]]), abs(duration.[[Weeks]]), abs(duration.[[Days]]), abs(duration.[[Hours]]), abs(duration.[[Minutes]]), abs(duration.[[Seconds]]), abs(duration.[[Milliseconds]]), abs(duration.[[Microseconds]]), abs(duration.[[Nanoseconds]])).
    return MUST(create_temporal_duration(vm, fabs(duration->years()), fabs(duration->months()), fabs(duration->weeks()), fabs(duration->days()), fabs(duration->hours()), fabs(duration->minutes()), fabs(duration->seconds()), fabs(duration->milliseconds()), fabs(duration->microseconds()), fabs(duration->nanoseconds())));
}

// 7.3.18 Temporal.Duration.prototype.add ( other ), https://tc39.es/proposal-temporal/#sec-temporal.duration.prototype.add
JS_DEFINE_NATIVE_FUNCTION(DurationPrototype::add)
{
    auto other = vm.argument(0);

    // 1. Let duration be the this value.
    // 2. Perform ? RequireInternalSlot(duration, [[InitializedTemporalDuration]]).
    auto duration = TRY(typed_this_object(vm));

    // 3. Return ? AddDurations(ADD, duration, other).
    return TRY(add_durations(vm, ArithmeticOperation::Add, duration, other));
}

// 7.3.19 Temporal.Duration.prototype.subtract ( other ), https://tc39.es/proposal-temporal/#sec-temporal.duration.prototype.subtract
JS_DEFINE_NATIVE_FUNCTION(DurationPrototype::subtract)
{
    auto other = vm.argument(0);

    // 1. Let duration be the this value.
    // 2. Perform ? RequireInternalSlot(duration, [[InitializedTemporalDuration]]).
    auto duration = TRY(typed_this_object(vm));

    // 3. Return ? AddDurations(SUBTRACT, duration, other).
    return TRY(add_durations(vm, ArithmeticOperation::Subtract, duration, other));
}

// 7.3.20 Temporal.Duration.prototype.round ( roundTo ), https://tc39.es/proposal-temporal/#sec-temporal.duration.prototype.round
JS_DEFINE_NATIVE_FUNCTION(DurationPrototype::round)
{
    auto& realm = *vm.current_realm();

    auto round_to_value = vm.argument(0);

    // 1. Let duration be the this value.
    // 2. Perform ? RequireInternalSlot(duration, [[InitializedTemporalDuration]]).
    auto duration = TRY(typed_this_object(vm));

    // 3. If roundTo is undefined, then
    if (round_to_value.is_undefined()) {
        // a. Throw a TypeError exception.
        return vm.throw_completion<TypeError>(ErrorType::TemporalMissingOptionsObject);
    }

    GC::Ptr<Object> round_to;

    // 4. If roundTo is a String, then
    if (round_to_value.is_string()) {
        // a. Let paramString be roundTo.
        auto param_string = round_to_value;

        // b. Set roundTo to OrdinaryObjectCreate(null).
        round_to = Object::create(realm, nullptr);

        // c. Perform ! CreateDataPropertyOrThrow(roundTo, "smallestUnit", paramString).
        MUST(round_to->create_data_property_or_throw(vm.names.smallestUnit, param_string));
    }
    // 5. Else,
    else {
        // a. Set roundTo to ? GetOptionsObject(roundTo).
        round_to = TRY(get_options_object(vm, round_to_value));
    }

    // 6. Let smallestUnitPresent be true.
    bool smallest_unit_present = true;

    // 7. Let largestUnitPresent be true.
    bool largest_unit_present = true;

    // 8. NOTE: The following steps read options and perform independent validation in alphabetical order
    //    (GetTemporalRelativeToOption reads "relativeTo", GetRoundingIncrementOption reads "roundingIncrement" and
    //    GetRoundingModeOption reads "roundingMode").

    // 9. Let largestUnit be ? GetTemporalUnitValuedOption(roundTo, "largestUnit", DATETIME, UNSET, « auto »).
    auto largest_unit = TRY(get_temporal_unit_valued_option(vm, *round_to, vm.names.largestUnit, UnitGroup::DateTime, Unset {}, { { Auto {} } }));

    // 10. Let relativeToRecord be ? GetTemporalRelativeToOption(roundTo).
    // 11. Let zonedRelativeTo be relativeToRecord.[[ZonedRelativeTo]].
    // 12. Let plainRelativeTo be relativeToRecord.[[PlainRelativeTo]].
    auto [zoned_relative_to, plain_relative_to] = TRY(get_temporal_relative_to_option(vm, *round_to));

    // 13. Let roundingIncrement be ? GetRoundingIncrementOption(roundTo).
    auto rounding_increment = TRY(get_rounding_increment_option(vm, *round_to));

    // 14. Let roundingMode be ? GetRoundingModeOption(roundTo, HALF-EXPAND).
    auto rounding_mode = TRY(get_rounding_mode_option(vm, *round_to, RoundingMode::HalfExpand));

    // 15. Let smallestUnit be ? GetTemporalUnitValuedOption(roundTo, "smallestUnit", DATETIME, UNSET).
    auto smallest_unit = TRY(get_temporal_unit_valued_option(vm, *round_to, vm.names.smallestUnit, UnitGroup::DateTime, Unset {}));

    // 16. If smallestUnit is UNSET, then
    if (smallest_unit.has<Unset>()) {
        // a. Set smallestUnitPresent to false.
        smallest_unit_present = false;

        // b. Set smallestUnit to NANOSECOND.
        smallest_unit = Unit::Nanosecond;
    }

    auto smallest_unit_value = smallest_unit.get<Unit>();

    // 17. Let existingLargestUnit be DefaultTemporalLargestUnit(duration).
    auto existing_largest_unit = default_temporal_largest_unit(duration);

    // 18. Let defaultLargestUnit be LargerOfTwoTemporalUnits(existingLargestUnit, smallestUnit).
    auto default_largest_unit = larger_of_two_temporal_units(existing_largest_unit, smallest_unit_value);

    // 19. If largestUnit is UNSET, then
    if (largest_unit.has<Unset>()) {
        // a. Set largestUnitPresent to false.
        largest_unit_present = false;

        // b. Set largestUnit to defaultLargestUnit.
        largest_unit = default_largest_unit;
    }
    // 20. Else if largestUnit is AUTO, then
    else if (largest_unit.has<Auto>()) {
        // a. Set largestUnit to defaultLargestUnit.
        largest_unit = default_largest_unit;
    }

    // 21. If smallestUnitPresent is false and largestUnitPresent is false, then
    if (!smallest_unit_present && !largest_unit_present) {
        // a. Throw a RangeError exception.
        return vm.throw_completion<RangeError>(ErrorType::TemporalMissingUnits);
    }

    auto largest_unit_value = largest_unit.get<Unit>();

    // 22. If LargerOfTwoTemporalUnits(largestUnit, smallestUnit) is not largestUnit, throw a RangeError exception.
    if (larger_of_two_temporal_units(largest_unit_value, smallest_unit_value) != largest_unit_value)
        return vm.throw_completion<RangeError>(ErrorType::TemporalInvalidUnitRange, temporal_unit_to_string(smallest_unit_value), temporal_unit_to_string(largest_unit_value));

    // 23. Let maximum be MaximumTemporalDurationRoundingIncrement(smallestUnit).
    auto maximum = maximum_temporal_duration_rounding_increment(smallest_unit_value);

    // 24. If maximum is not UNSET, perform ? ValidateTemporalRoundingIncrement(roundingIncrement, maximum, false).
    if (!maximum.has<Unset>())
        TRY(validate_temporal_rounding_increment(vm, rounding_increment, maximum.get<u64>(), false));

    // 25. If roundingIncrement > 1, and largestUnit is not smallestUnit, and TemporalUnitCategory(smallestUnit) is DATE,
    //     throw a RangeError exception.
    if (rounding_increment > 1 && largest_unit_value != smallest_unit_value && temporal_unit_category(smallest_unit_value) == UnitCategory::Date)
        return vm.throw_completion<RangeError>(ErrorType::OptionIsNotValidValue, rounding_increment, "roundingIncrement");

    // 26. If zonedRelativeTo is not undefined, then
    if (zoned_relative_to) {
        // a. Let internalDuration be ToInternalDurationRecord(duration).
        auto internal_duration = to_internal_duration_record(vm, duration);

        // FIXME: b. Let timeZone be zonedRelativeTo.[[TimeZone]].
        // FIXME: c. Let calendar be zonedRelativeTo.[[Calendar]].
        // FIXME: d. Let relativeEpochNs be zonedRelativeTo.[[EpochNanoseconds]].
        // FIXME: e. Let targetEpochNs be ? AddZonedDateTime(relativeEpochNs, timeZone, calendar, internalDuration, constrain).
        // FIXME: f. Set internalDuration to ? DifferenceZonedDateTimeWithRounding(relativeEpochNs, targetEpochNs, timeZone, calendar, largestUnit, roundingIncrement, smallestUnit, roundingMode).

        // g. If TemporalUnitCategory(largestUnit) is date, set largestUnit to hour.
        if (temporal_unit_category(largest_unit_value) == UnitCategory::Date)
            largest_unit_value = Unit::Hour;

        // h. Return ? TemporalDurationFromInternal(internalDuration, largestUnit).
        return TRY(temporal_duration_from_internal(vm, internal_duration, largest_unit_value));
    }

    // 27. If plainRelativeTo is not undefined, then
    if (plain_relative_to) {
        // a. Let internalDuration be ToInternalDurationRecordWith24HourDays(duration).
        auto internal_duration = to_internal_duration_record_with_24_hour_days(vm, duration);

        // FIXME: b. Let targetTime be AddTime(MidnightTimeRecord(), internalDuration.[[Time]]).
        // FIXME: c. Let calendar be plainRelativeTo.[[Calendar]].
        // FIXME: d. Let dateDuration be ! AdjustDateDurationRecord(internalDuration.[[Date]], targetTime.[[Days]]).
        // FIXME: e. Let targetDate be ? CalendarDateAdd(calendar, plainRelativeTo.[[ISODate]], dateDuration, constrain).
        // FIXME: f. Let isoDateTime be CombineISODateAndTimeRecord(plainRelativeTo.[[ISODate]], MidnightTimeRecord()).
        // FIXME: g. Let targetDateTime be CombineISODateAndTimeRecord(targetDate, targetTime).
        // FIXME: h. Set internalDuration to ? DifferencePlainDateTimeWithRounding(isoDateTime, targetDateTime, calendar, largestUnit, roundingIncrement, smallestUnit, roundingMode).

        // i. Return ? TemporalDurationFromInternal(internalDuration, largestUnit).
        return TRY(temporal_duration_from_internal(vm, internal_duration, largest_unit_value));
    }

    // 28. If IsCalendarUnit(existingLargestUnit) is true, or IsCalendarUnit(largestUnit) is true, throw a RangeError exception.
    if (is_calendar_unit(existing_largest_unit))
        return vm.throw_completion<RangeError>(ErrorType::TemporalInvalidLargestUnit, temporal_unit_to_string(existing_largest_unit));
    if (is_calendar_unit(largest_unit_value))
        return vm.throw_completion<RangeError>(ErrorType::TemporalInvalidLargestUnit, temporal_unit_to_string(largest_unit_value));

    // 29. Assert: IsCalendarUnit(smallestUnit) is false.
    VERIFY(!is_calendar_unit(smallest_unit_value));

    // 30. Let internalDuration be ToInternalDurationRecordWith24HourDays(duration).
    auto internal_duration = to_internal_duration_record_with_24_hour_days(vm, duration);

    // 31. If smallestUnit is DAY, then
    if (smallest_unit_value == Unit::Day) {
        // a. Let fractionalDays be TotalTimeDuration(internalDuration.[[Time]], DAY).
        auto fractional_days = total_time_duration(internal_duration.time, Unit::Day);

        // b. Let days be RoundNumberToIncrement(fractionalDays, roundingIncrement, roundingMode).
        auto days = round_number_to_increment(fractional_days, rounding_increment, rounding_mode);

        // c. Let dateDuration be ? CreateDateDurationRecord(0, 0, 0, days).
        auto date_duration = TRY(create_date_duration_record(vm, 0, 0, 0, days));

        // d. Set internalDuration to ! CombineDateAndTimeDuration(dateDuration, 0).
        internal_duration = MUST(combine_date_and_time_duration(vm, date_duration, TimeDuration { 0 }));
    }
    // 32. Else,
    else {
        // a. Let timeDuration be ? RoundTimeDuration(internalDuration.[[Time]], roundingIncrement, smallestUnit, roundingMode).
        auto time_duration = TRY(round_time_duration(vm, internal_duration.time, Crypto::UnsignedBigInteger { rounding_increment }, smallest_unit_value, rounding_mode));

        // b. Set internalDuration to ! CombineDateAndTimeDuration(ZeroDateDuration(), timeDuration).
        internal_duration = MUST(combine_date_and_time_duration(vm, zero_date_duration(vm), move(time_duration)));
    }

    // 33. Return ? TemporalDurationFromInternal(internalDuration, largestUnit).
    return TRY(temporal_duration_from_internal(vm, internal_duration, largest_unit_value));
}

// 7.3.22 Temporal.Duration.prototype.toString ( [ options ] ), https://tc39.es/proposal-temporal/#sec-temporal.duration.prototype.tostring
JS_DEFINE_NATIVE_FUNCTION(DurationPrototype::to_string)
{
    // 1. Let duration be the this value.
    // 2. Perform ? RequireInternalSlot(duration, [[InitializedTemporalDuration]]).
    auto duration = TRY(typed_this_object(vm));

    // 3. Let resolvedOptions be ? GetOptionsObject(options).
    auto resolved_options = TRY(get_options_object(vm, vm.argument(0)));

    // 4. NOTE: The following steps read options and perform independent validation in alphabetical order
    //    (GetTemporalFractionalSecondDigitsOption reads "fractionalSecondDigits" and GetRoundingModeOption reads
    //    "roundingMode").

    // 5. Let digits be ? GetTemporalFractionalSecondDigitsOption(resolvedOptions).
    auto digits = TRY(get_temporal_fractional_second_digits_option(vm, resolved_options));

    // 6. Let roundingMode be ? GetRoundingModeOption(resolvedOptions, TRUNC).
    auto rounding_mode = TRY(get_rounding_mode_option(vm, resolved_options, RoundingMode::Trunc));

    // 7. Let smallestUnit be ? GetTemporalUnitValuedOption(resolvedOptions, "smallestUnit", TIME, UNSET).
    auto smallest_unit = TRY(get_temporal_unit_valued_option(vm, resolved_options, vm.names.smallestUnit, UnitGroup::Time, Unset {}));

    // 8. If smallestUnit is HOUR or MINUTE, throw a RangeError exception.
    if (auto const* unit = smallest_unit.get_pointer<Unit>(); unit && (*unit == Unit::Hour || *unit == Unit::Minute))
        return vm.throw_completion<RangeError>(ErrorType::OptionIsNotValidValue, temporal_unit_to_string(*unit), vm.names.smallestUnit);

    // 9. Let precision be ToSecondsStringPrecisionRecord(smallestUnit, digits).
    auto precision = to_seconds_string_precision_record(smallest_unit, digits);

    // 10. If precision.[[Unit]] is NANOSECOND and precision.[[Increment]] = 1, then
    if (precision.unit == Unit::Nanosecond && precision.increment == 1) {
        // a. Return TemporalDurationToString(duration, precision.[[Precision]]).
        return PrimitiveString::create(vm, temporal_duration_to_string(duration, precision.precision.downcast<Auto, u8>()));
    }

    // 11. Let largestUnit be DefaultTemporalLargestUnit(duration).
    auto largest_unit = default_temporal_largest_unit(duration);

    // 12. Let internalDuration be ToInternalDurationRecord(duration).
    auto internal_duration = to_internal_duration_record(vm, duration);

    // 13. Let timeDuration be ? RoundTimeDuration(internalDuration.[[Time]], precision.[[Increment]], precision.[[Unit]], roundingMode).
    auto time_duration = TRY(round_time_duration(vm, internal_duration.time, precision.increment, precision.unit, rounding_mode));

    // 14. Set internalDuration to ! CombineDateAndTimeDuration(internalDuration.[[Date]], timeDuration).
    internal_duration = MUST(combine_date_and_time_duration(vm, internal_duration.date, move(time_duration)));

    // 15. Let roundedLargestUnit be LargerOfTwoTemporalUnits(largestUnit, SECOND).
    auto rounded_largest_unit = larger_of_two_temporal_units(largest_unit, Unit::Second);

    // 16. Let roundedDuration be ? TemporalDurationFromInternal(internalDuration, roundedLargestUnit).
    auto rounded_duration = TRY(temporal_duration_from_internal(vm, internal_duration, rounded_largest_unit));

    // 17. Return TemporalDurationToString(roundedDuration, precision.[[Precision]]).
    return PrimitiveString::create(vm, temporal_duration_to_string(rounded_duration, precision.precision.downcast<Auto, u8>()));
}

// 7.3.23 Temporal.Duration.prototype.toJSON ( ), https://tc39.es/proposal-temporal/#sec-temporal.duration.prototype.tojson
JS_DEFINE_NATIVE_FUNCTION(DurationPrototype::to_json)
{
    // 1. Let duration be the this value.
    // 2. Perform ? RequireInternalSlot(duration, [[InitializedTemporalDuration]]).
    auto duration = TRY(typed_this_object(vm));

    // 3. Return TemporalDurationToString(duration, AUTO).
    return PrimitiveString::create(vm, temporal_duration_to_string(duration, Auto {}));
}

// 7.3.24 Temporal.Duration.prototype.toLocaleString ( [ locales [ , options ] ] ), https://tc39.es/proposal-temporal/#sec-temporal.duration.prototype.tolocalestring
// NOTE: This is the minimum toLocaleString implementation for engines without ECMA-402.
JS_DEFINE_NATIVE_FUNCTION(DurationPrototype::to_locale_string)
{
    // 1. Let duration be the this value.
    // 2. Perform ? RequireInternalSlot(duration, [[InitializedTemporalDuration]]).
    auto duration = TRY(typed_this_object(vm));

    // 3. Return TemporalDurationToString(duration, AUTO).
    return PrimitiveString::create(vm, temporal_duration_to_string(duration, Auto {}));
}

}
