/** @file
 * @brief Magnitude, floor, ceil, and round operations for strong numeric types
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/strong/fwd.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <type_traits>

namespace mpk::mix
{

template <StrongCountType T>
    requires std::is_integral_v<typename T::Weak>
auto magnitude(T v)
{
    return T{std::abs(v.v)};
}

template <StrongCountType T>
    requires std::is_floating_point_v<typename T::Weak>
auto magnitude(T v) -> T
{
    // NOTE: For floating point, rely on fabs, because
    // abs(floating-point) is gone since C++23
    return T{std::fabs(v.v)};
}

template <StrongArithmeticType T>
    requires std::is_floating_point_v<typename T::Weak>
auto floor(T v) -> T
{
    return T{std::floor(v.v)};
}

template <StrongArithmeticType T>
    requires std::is_floating_point_v<typename T::Weak>
auto floor(T v, T unit) -> T
{
    return T{std::floor(v.v / unit.v) * unit.v};
}

template <StrongArithmeticType T>
    requires std::is_floating_point_v<typename T::Weak>
auto ceil(T v) -> T
{
    return T{std::ceil(v.v)};
}

template <StrongArithmeticType T>
    requires std::is_floating_point_v<typename T::Weak>
auto ceil(T v, T unit) -> T
{
    return T{std::ceil(v.v / unit.v) * unit.v};
}

template <StrongArithmeticType T>
    requires std::is_floating_point_v<typename T::Weak>
auto round(T v) -> T
{
    return T{std::round(v.v)};
}

template <StrongArithmeticType T>
    requires std::is_floating_point_v<typename T::Weak>
auto round(T v, T unit) -> T
{
    return T{std::round(v.v / unit.v) * unit.v};
}

template <StrongArithmeticType T>
auto min(T a, T b) -> T
{
    return T{std::min(a.v, b.v)};
}

template <StrongArithmeticType T>
auto max(T a, T b) -> T
{
    return T{std::max(a.v, b.v)};
}

enum class ClampType : uint8_t
{
    None,
    Lower,
    Upper
};

template <StrongArithmeticType T>
struct ClampedResult
{
    T value;
    ClampType type;

    auto operator==(ClampedResult const&) const noexcept -> bool = default;
};

template <StrongArithmeticType T>
auto clamped(T v, T lower, T upper) -> ClampedResult<T>
{
    if (v < lower)
        return {lower, ClampType::Lower};
    if (v > upper)
        return {upper, ClampType::Upper};
    return {v, ClampType::None};
}

template <StrongArithmeticType T>
auto clamp(T& v, T lower, T upper) -> ClampType
{
    auto [clamped_v, type] = clamped(v, lower, upper);
    v = clamped_v;
    return type;
}

} // namespace mpk::mix
