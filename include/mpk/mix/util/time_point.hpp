/** @file
 * @brief Conversions between system_clock::time_point and human-readable strings
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/detail/format.hpp"
#include "mpk/mix/meta/type.hpp"

#include <chrono>
#include <cstdint>
#include <string>
#include <string_view>

namespace mpk::mix {

template <typename T>
concept ChronoTimepointType =
    requires {
        typename T::clock;
        typename T::duration;
    }
    && std::same_as<
        T,
        std::chrono::time_point<typename T::clock, typename T::duration>>;

auto time_point_to_unix_nano(
    std::chrono::system_clock::time_point const& tp) -> std::int64_t;

template <typename Duration = std::chrono::microseconds>
auto time_point_to_readable(
    std::chrono::system_clock::time_point const& tp, Type_Tag<Duration> = {})
    -> std::string
{
    return mpk::mix::format(
        "{:%Y-%m-%d %H:%M:%S}", std::chrono::time_point_cast<Duration>(tp));
}

auto utc_midnight(std::chrono::system_clock::time_point const& tp)
    -> std::chrono::system_clock::time_point;

#ifndef MPKMIX_NO_TZ

template <typename Duration = std::chrono::microseconds>
auto time_point_to_readable(
    std::chrono::system_clock::time_point const& tp,
    std::string_view zone_spec,
    Type_Tag<Duration> = {}) -> std::string;

auto zoned_midnight(
    std::chrono::system_clock::time_point const& tp,
    std::string_view zone_spec = {})
    -> std::chrono::system_clock::time_point;

auto from_local_time(
    std::chrono::local_time<
        std::chrono::system_clock::time_point::duration> tp,
    std::string_view zone_spec = {})
    -> std::chrono::system_clock::time_point;

#endif  // MPKMIX_NO_TZ

} // namespace mpk::mix
