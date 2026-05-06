/** @file
 * @brief YAML serialization implementation
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/serial/yaml/to_yaml.hpp"

#include <string>

namespace mpk::mix::serial::yaml {

auto format_nanoseconds(std::chrono::nanoseconds value) -> std::string
{
    auto count = value.count();

    constexpr long long ns_per_us   = 1'000LL;
    constexpr long long ns_per_ms   = 1'000'000LL;
    constexpr long long ns_per_s    = 1'000'000'000LL;
    constexpr long long ns_per_min  = 60LL * ns_per_s;
    constexpr long long ns_per_hour = 3'600LL * ns_per_s;
    constexpr long long ns_per_day  = 86'400LL * ns_per_s;

    auto divisible = [&](long long unit) { return count % unit == 0; };

    if (count == 0)                  return "0s";
    if (divisible(ns_per_day))   return std::to_string(count / ns_per_day)  + "days";
    if (divisible(ns_per_hour))  return std::to_string(count / ns_per_hour) + "hours";
    if (divisible(ns_per_min))   return std::to_string(count / ns_per_min)  + "min";
    if (divisible(ns_per_s))     return std::to_string(count / ns_per_s)    + "s";
    if (divisible(ns_per_ms))    return std::to_string(count / ns_per_ms)   + "ms";
    if (divisible(ns_per_us))    return std::to_string(count / ns_per_us)   + "us";
    return std::to_string(count) + "ns";
}

} // namespace mpk::mix::serial::yaml
