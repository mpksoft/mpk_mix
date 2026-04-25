/** @file
 * @brief std::hash specializations and hash() helpers for strong types and structs
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/strong/fwd.hpp"
#include "mpk/mix/struct_type.hpp"
#include "mpk/mix/types/fixed_length_string.hpp"

#include <concepts>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace mpk::mix::detail
{

template <typename>
constexpr inline auto new_hashed_type = false;

template <std::integral T>
auto hash(T const& v) noexcept -> size_t
{
    return std::hash<T>{}(v);
}

template <std::floating_point T>
auto hash(T const& v) noexcept -> size_t
{
    return std::hash<T>{}(v);
}

template <typename CharT>
auto hash(std::basic_string_view<CharT> const& v) noexcept -> size_t
{
    return std::hash<std::basic_string_view<CharT>>{}(v);
}

inline auto hash(std::string const& v) noexcept -> size_t
{
    return std::hash<std::string>{}(v);
}

// Forward declarations so hash_impl can find these via unqualified lookup
template <StrongType T>
auto hash(T const& v) noexcept -> size_t;

template <size_t N>
auto hash(FixedLengthString<N> const& v) noexcept -> size_t;

template <StructType T>
auto hash(T const& v) noexcept -> size_t;

template <typename T>
auto hash(std::vector<T> const& v) noexcept -> size_t;

template <size_t... I, typename... Ts>
auto hash_impl(std::index_sequence<I...>, Ts const&... vs) noexcept -> size_t
{
    return ((hash(vs) << I) ^ ...);
}

template <size_t... I, typename... Ts>
auto tuple_hash_impl(
    std::index_sequence<I...> index_seq, std::tuple<Ts...> const& v) noexcept
    -> size_t
{
    return hash_impl(index_seq, std::get<I>(v)...);
}

template <typename T0, typename T1, typename... Ts>
auto hash(T0 const& v0, T1 const& v1, Ts const&... vs) noexcept -> size_t
{
    return hash_impl(std::index_sequence_for<T0, T1, Ts...>(), v0, v1, vs...);
}

template <typename... Ts>
auto hash(std::tuple<Ts...> const& v) noexcept -> size_t
{
    return tuple_hash_impl(std::index_sequence_for<Ts...>(), v);
}

template <typename T1, typename T2>
auto hash(std::pair<T1, T2> const& v) noexcept -> size_t
{
    return hash(v.first, v.second);
}

template <StructType T>
auto hash(T const& v) noexcept -> size_t
{
    return hash(fields_of(v));
}

template <StructType T>
constexpr inline auto new_hashed_type<T> = true;

template <typename T>
auto hash(std::vector<T> const& v) noexcept -> size_t
{
    auto result = size_t{};
    for (auto const& element: v)
        result = (result << 1) ^ hash(element);
    return result;
}

template <typename T>
constexpr inline auto new_hashed_type<std::vector<T>> = true;

template <size_t N>
auto hash(FixedLengthString<N> const& v) noexcept -> size_t
{
    return hash(v.view());
}

template <size_t N>
constexpr inline auto new_hashed_type<FixedLengthString<N>> = true;

template <StrongType T>
auto hash(T const& v) noexcept -> size_t
{
    return hash(v.v);
}

template <StrongType T>
constexpr inline auto new_hashed_type<T> = true;

struct Hash final
{
    template <typename T>
    auto operator()(T const& v) const noexcept -> size_t
    {
        return hash(v);
    }
};

template <typename T>
concept NewHashedType = new_hashed_type<T>;

template <typename T>
concept Hashable = requires(T const& v) { Hash{}(v) -> size_t; };

} // namespace mpk::mix::detail

namespace std
{

template <::mpk::mix::detail::NewHashedType T>
struct hash<T>
{
    auto operator()(T const& v) const noexcept -> size_t
    {
        return ::mpk::mix::detail::hash(v);
    }
};

} // namespace std

namespace mpk::mix::detail
{

template <typename T>
concept StdHashable = requires(T const& v) {
    { ::std::hash<T>{}(v) } -> std::same_as<size_t>;
};

struct StringHash
{
    using is_transparent = void;

    auto operator()(std::string_view sv) const noexcept -> size_t
    {
        return std::hash<std::string_view>{}(sv);
    }

    auto operator()(std::string const& s) const noexcept -> size_t
    {
        return std::hash<std::string>{}(s);
    }

    auto operator()(char const* s) const noexcept -> size_t
    {
        return std::hash<std::string_view>{}(s);
    }
};

} // namespace mpk::mix::detail
