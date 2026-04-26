/** @file
 * @brief Implementation of time-point-to-string conversion utilities
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/util/time_point.hpp"

#ifndef MPKMIX_NO_TZ
#  ifdef __clang__
#    include "date/tz.h"
#  endif
#endif

#include <chrono>
#include <sstream>

namespace mpk::mix {

#ifndef MPKMIX_NO_TZ

#ifdef __clang__
namespace tz = date;
#else
namespace tz = std::chrono;
#endif

namespace {

auto get_time_zone(std::string_view spec) -> tz::time_zone const*
{
    return spec.empty() ? tz::current_zone() : tz::get_tzdb().locate_zone(spec);
}

} // anonymous namespace

#endif  // MPKMIX_NO_TZ

auto time_point_to_unix_nano(
    std::chrono::system_clock::time_point const& tp) -> std::int64_t
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
               tp.time_since_epoch())
        .count();
}

auto utc_midnight(std::chrono::system_clock::time_point const& tp)
    -> std::chrono::system_clock::time_point
{
    using namespace std::chrono;
    return floor<days>(tp);
}

#ifndef MPKMIX_NO_TZ

template <typename Duration>
auto time_point_to_readable(
    std::chrono::system_clock::time_point const& tp,
    std::string_view zone_spec,
    Type_Tag<Duration>) -> std::string
{
    auto const* zone = get_time_zone(zone_spec);
    auto zoned_ts =
        tz::zoned_time{zone, std::chrono::time_point_cast<Duration>(tp)};
    std::ostringstream s;
    s << zoned_ts;
    return s.str();
}

template auto time_point_to_readable(
    std::chrono::system_clock::time_point const&,
    std::string_view,
    Type_Tag<std::chrono::seconds>) -> std::string;
template auto time_point_to_readable(
    std::chrono::system_clock::time_point const&,
    std::string_view,
    Type_Tag<std::chrono::milliseconds>) -> std::string;
template auto time_point_to_readable(
    std::chrono::system_clock::time_point const&,
    std::string_view,
    Type_Tag<std::chrono::microseconds>) -> std::string;
template auto time_point_to_readable(
    std::chrono::system_clock::time_point const&,
    std::string_view,
    Type_Tag<std::chrono::nanoseconds>) -> std::string;

auto zoned_midnight(
    std::chrono::system_clock::time_point const& tp,
    std::string_view zone_spec)
    -> std::chrono::system_clock::time_point
{
    using namespace std::chrono;
    auto const* zone = get_time_zone(zone_spec);
    auto local_tp = tz::zoned_time{zone, tp};
    auto local_day = floor<days>(local_tp.get_local_time());
    return tz::zoned_time{zone, local_day}.get_sys_time();
}

auto from_local_time(
    std::chrono::local_time<
        std::chrono::system_clock::time_point::duration> tp,
    std::string_view zone_spec)
    -> std::chrono::system_clock::time_point
{
    using namespace std::chrono;
    using TzLocalTime =
        tz::local_time<std::chrono::system_clock::time_point::duration>;
    auto tp_tz = TzLocalTime{tp.time_since_epoch()};
    auto const* zone = get_time_zone(zone_spec);
    return tz::zoned_time{zone, tp_tz}.get_sys_time();
}

#endif  // MPKMIX_NO_TZ

} // namespace mpk::mix
