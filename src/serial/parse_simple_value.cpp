/** @file
 * @brief Implementation of simple scalar serialization parser
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/serial/parse_simple_value.hpp"

#include "mpk/mix/util/throw.hpp"
#include "mpk/mix/util/time_point.hpp"

#include <algorithm>
#include <cassert>
#include <charconv>
#include <cmath>
#include <cstdio>
#include <regex>
#include <sstream>
#include <type_traits>

using namespace std::string_view_literals;

namespace mpk::mix {

namespace {

template <typename T, typename... FromCharsArgs>
    requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
auto parse_as_decimal(
    T& result, std::string_view s, FromCharsArgs... from_chars_args) -> void
{
    auto from_chars_result =
        std::from_chars(s.begin(), s.end(), result, from_chars_args...);

    if (from_chars_result.ec == std::errc::invalid_argument)
        throw_("Failed to parse a scalar from string '{}' - not a number", s);
    else if (from_chars_result.ec == std::errc::result_out_of_range)
        throw_("Failed to parse a scalar from string '{}' - out of range", s);

    assert(from_chars_result.ec == std::error_code{});
    if (from_chars_result.ptr != s.end())
        throw_(
            "Failed to parse a scalar from string '{}' - "
            "extra characters remain",
            s);
}

template <typename T>
    requires std::is_integral_v<T>
auto parse_int(T& result, std::string_view s) -> void
{
    if (s.starts_with("0x"sv))
        parse_as_decimal(result, s.substr(2), 16);
    else if (s.starts_with("#"sv))
        parse_as_decimal(result, s.substr(1), 16);
    else
        parse_as_decimal(result, s);
}

// Note: when using Clang (as of versions 18, 19), one should also use
// libstdc++ for `std::from_chars` to be supported for floating-point types.
template <typename T>
    requires std::is_floating_point_v<T>
auto parse_float(T& result, std::string_view s) -> void
{
    parse_as_decimal(result, s);
}

auto parse_bool(bool& result, std::string_view s) -> void
{
    if (s == "true")
        result = true;
    else if (s == "false")
        result = false;
    else
        throw_("Failed to parse boolean scalar from string '{}'", s);
}

auto split_by_whitespace(std::string_view s) -> std::vector<std::string_view>
{
    constexpr auto npos = std::string_view::npos;
    constexpr auto whitespace = " \t";
    auto result = std::vector<std::string_view>{};
    while (true)
    {
        auto begin = s.find_first_not_of(whitespace);
        if (begin == npos)
            break;
        auto end = s.find_first_of(whitespace, begin);
        auto size = end == npos ? npos : end - begin;
        result.push_back(s.substr(begin, size));
        s = end == npos ? std::string_view{} : s.substr(end);
    }
    return result;
}

} // anonymous namespace

auto parse_simple_value(int8_t& result, std::string_view s) -> void
{ parse_int(result, s); }
auto parse_simple_value(int16_t& result, std::string_view s) -> void
{ parse_int(result, s); }
auto parse_simple_value(int32_t& result, std::string_view s) -> void
{ parse_int(result, s); }
auto parse_simple_value(int64_t& result, std::string_view s) -> void
{ parse_int(result, s); }
auto parse_simple_value(uint8_t& result, std::string_view s) -> void
{ parse_int(result, s); }
auto parse_simple_value(uint16_t& result, std::string_view s) -> void
{ parse_int(result, s); }
auto parse_simple_value(uint32_t& result, std::string_view s) -> void
{ parse_int(result, s); }
auto parse_simple_value(uint64_t& result, std::string_view s) -> void
{ parse_int(result, s); }
auto parse_simple_value(bool& result, std::string_view s) -> void
{ parse_bool(result, s); }
auto parse_simple_value(float& result, std::string_view s) -> void
{ parse_float(result, s); }
auto parse_simple_value(double& result, std::string_view s) -> void
{ parse_float(result, s); }
auto parse_simple_value(std::string& result, std::string_view s) -> void
{ result = s; }
auto parse_simple_value(std::filesystem::path& result, std::string_view s) -> void
{ result = s; }

auto parse_simple_value(
    std::chrono::nanoseconds& result, std::string_view s) -> void
{
    std::regex pattern(R"(([\d.]+)\s*([a-zA-Z]+))");
    std::match_results<std::string_view::const_iterator> match;

    if (!std::regex_match(s.begin(), s.end(), match, pattern))
        throw_<std::invalid_argument>("Invalid duration specification '{}'", s);

    double value = std::stod(match[1].str());
    std::string unit = match[2].str();
    using Raw = std::chrono::nanoseconds::rep;

    if (unit == "ns")
        result = std::chrono::nanoseconds(static_cast<Raw>(value));
    else if (unit == "us" || unit == "µs")
        result = std::chrono::nanoseconds(static_cast<Raw>(value * 1'000));
    else if (unit == "ms")
        result = std::chrono::nanoseconds(static_cast<Raw>(value * 1'000'000));
    else if (unit == "s")
        result = std::chrono::nanoseconds(
            static_cast<Raw>(value * 1'000'000'000));
    else if (unit == "min")
        result = std::chrono::nanoseconds(
            static_cast<Raw>(value * 60'000'000'000));
    else if (unit == "hours")
        result = std::chrono::nanoseconds(
            static_cast<Raw>(value) * 3600'000'000'000);
    else if (unit == "days")
        result = std::chrono::nanoseconds(
            static_cast<Raw>(value) * 24 * 3600'000'000'000);
    else
        throw_<std::invalid_argument>(
            "Invalid duration specification '{}' - "
            "unsupported duration unit '{}'",
            s,
            unit);
}

auto parse_simple_value(
    std::chrono::sys_time<std::chrono::nanoseconds>& result,
    std::string_view s) -> void
{
    using namespace std::chrono;

    if (s == "now")
    {
        result = system_clock::now();
        return;
    }

    auto err = [&]
    {
        throw_<std::invalid_argument>(
            "Invalid date/time specification '{}'", s);
    };

#ifndef MPKMIX_NO_TZ
    auto eof = [&](std::istream& is)
    {
        if (is.eof())
            return true;
        auto pos = is.tellg();
        std::string str;
        is >> str;
        if (str.empty() && is.eof())
            return true;
        is.seekg(pos);
        return false;
    };
#endif

    auto parts = split_by_whitespace(s);
    if (parts.size() < 1 || parts.size() > 3)
        err();

    enum PartType { Date, Time, TimeZone, Invalid };
    auto part_type = Date;

    constexpr size_t none = ~0ull;
    size_t date_index = none;
    size_t time_index = none;
    [[maybe_unused]] size_t time_zone_index = none;

    constexpr auto npos = std::string_view::npos;

    for (size_t part_index = 0; part_index < parts.size();)
    {
        auto part = parts[part_index];
        switch (part_type)
        {
            case Date:
                if (part == "today" || part.find_first_of('-') != npos)
                    date_index = part_index++;
                part_type = Time;
                break;
            case Time:
                if (part.find_first_of(':') != npos)
                    time_index = part_index++;
                else if (date_index == none)
                    err();
                part_type = TimeZone;
                break;
            case TimeZone:
                time_zone_index = part_index++;
                part_type = Invalid;
                break;
            case Invalid: err();
        }
    }

#ifndef MPKMIX_NO_TZ
    auto zone_spec = time_zone_index == none ? ""sv : parts[time_zone_index];

    if (date_index == none)
        result = zoned_midnight(system_clock::now(), zone_spec);
    else
    {
        auto date_str = parts[date_index];
        if (date_str == "today")
            result = zoned_midnight(system_clock::now(), zone_spec);
        else
        {
            year_month_day date{};
            std::istringstream iss(std::string{date_str});
            iss >> std::chrono::parse("%Y-%m-%d", date);
            if (iss.fail() || !eof(iss))
                err();
            result = from_local_time(local_days{date}, zone_spec);
        }
    }
#else
    // Timezone support disabled: interpret all dates as UTC.
    if (date_index == none)
        result = utc_midnight(system_clock::now());
    else
    {
        auto date_str = parts[date_index];
        if (date_str == "today")
            result = utc_midnight(system_clock::now());
        else
        {
            year_month_day date{};
            int y, m, d;
            if (std::sscanf(std::string(date_str).c_str(), "%d-%d-%d", &y, &m, &d) != 3)
                err();
            date = year_month_day{
                year{y}, month{static_cast<unsigned>(m)}, day{static_cast<unsigned>(d)}};
            result = sys_days{date};
        }
    }
#endif  // MPKMIX_NO_TZ

    if (time_index != none)
    {
        // Split on '.' first so we can parse HH:MM:SS and fractional separately.
        auto time_str = std::string{parts[time_index]};
        auto dot_pos = time_str.find('.');
        auto hms_str = dot_pos != std::string::npos
                           ? time_str.substr(0, dot_pos)
                           : time_str;

        int h = 0, min_v = 0, sec_v = 0;
        if (std::sscanf(hms_str.c_str(), "%d:%d:%d", &h, &min_v, &sec_v) < 2)
            err();

        result += hours{h} + minutes{min_v} + seconds{sec_v};

        if (dot_pos == std::string::npos)
            return;

        auto fractional_part_str = time_str.substr(dot_pos + 1);
        if (fractional_part_str.empty()
            || fractional_part_str.length() > 9
            || !std::ranges::all_of(fractional_part_str, ::isdigit))
        {
            err();
        }

        using Raw = nanoseconds::rep;
        Raw fractional_nanos = 0;
        parse_simple_value(fractional_nanos, fractional_part_str);
        fractional_nanos *=
            static_cast<Raw>(std::pow(10, 9 - fractional_part_str.length()));
        result += nanoseconds(fractional_nanos);
    }
}

} // namespace mpk::mix
