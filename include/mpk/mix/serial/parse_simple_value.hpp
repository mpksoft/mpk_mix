/** @file
 * @brief Parse a simple scalar from a serialized node
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/meta/enum_type.hpp"
#include "mpk/mix/meta/type.hpp"
#include "mpk/mix/strong/fwd.hpp"
#include "mpk/mix/types/fixed_length_string.hpp"
#include "mpk/mix/util/format_seq.hpp"
#include "mpk/mix/util/throw.hpp"

#include <magic_enum/magic_enum.hpp>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <string_view>
#include <type_traits>

namespace mpk::mix {

auto parse_simple_value(int8_t&, std::string_view) -> void;
auto parse_simple_value(int16_t&, std::string_view) -> void;
auto parse_simple_value(int32_t&, std::string_view) -> void;
auto parse_simple_value(int64_t&, std::string_view) -> void;
auto parse_simple_value(uint8_t&, std::string_view) -> void;
auto parse_simple_value(uint16_t&, std::string_view) -> void;
auto parse_simple_value(uint32_t&, std::string_view) -> void;
auto parse_simple_value(uint64_t&, std::string_view) -> void;
auto parse_simple_value(bool&, std::string_view) -> void;
auto parse_simple_value(float&, std::string_view) -> void;
auto parse_simple_value(double&, std::string_view) -> void;
auto parse_simple_value(std::string&, std::string_view) -> void;
auto parse_simple_value(std::filesystem::path&, std::string_view) -> void;
auto parse_simple_value(std::chrono::nanoseconds&, std::string_view) -> void;
auto parse_simple_value(
    std::chrono::sys_time<std::chrono::nanoseconds>&, std::string_view) -> void;

// Generic chrono duration: parse the underlying Rep and construct the duration.
// E.g. parse_simple_value(std::chrono::seconds{}, "5") → 5s.
template <typename Rep, typename Period>
    requires(!std::same_as<std::chrono::duration<Rep, Period>, std::chrono::nanoseconds>)
auto parse_simple_value(std::chrono::duration<Rep, Period>& value, std::string_view s)
    -> void
{
    Rep count{};
    parse_simple_value(count, s);
    value = std::chrono::duration<Rep, Period>{count};
}

template <EnumType E>
auto parse_simple_value(E& value, std::string_view s) -> void
{
    if (auto e = magic_enum::enum_cast<E>(s))
        value = e.value();
    else
        throw_<std::invalid_argument>(
            "Failed to parse enum value '{}' of type '{}' - "
            "expected one of [{}]",
            s,
            magic_enum::enum_type_name<E>(),
            format_seq(magic_enum::enum_names<E>()));
}

template <size_t N>
auto parse_simple_value(
    FixedLengthString<N>& value, std::string_view s) -> void
{
    if (s.size() > N)
        throw_<std::invalid_argument>(
            "Failed to parse fixed-length string: input is too long: {} > {}",
            s.size(),
            N);
    value = s;
}

template <StrongType T>
auto parse_simple_value(T& value, std::string_view s) -> void
{
    parse_simple_value(value.v, s);
}

template <typename T>
auto parse_simple_value(std::string_view s, Type_Tag<T> = {}) -> T
{
    auto result = T{};
    parse_simple_value(result, s);
    return result;
}

} // namespace mpk::mix
