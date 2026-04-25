/** @file
 * @brief ByteSpan<> and ConstByteSpan<> non-owning byte-range views
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/util/any_of.hpp"
#include "mpk/mix/util/castable_to_bytes.hpp"

#include <cstddef>
#include <span>

namespace mpk::mix
{

template <typename>
constexpr inline auto is_byte_span_type_v = false;

template <size_t Extent>
constexpr inline auto is_byte_span_type_v<std::span<std::byte, Extent>> = true;

template <size_t Extent>
constexpr inline auto is_byte_span_type_v<std::span<std::byte const, Extent>> = true;

template <typename T>
concept ByteSpanType = is_byte_span_type_v<T>;

template <size_t Extent = std::dynamic_extent>
using ByteSpan = std::span<std::byte, Extent>;

template <size_t Extent = std::dynamic_extent>
using ConstByteSpan = std::span<std::byte const, Extent>;

template <typename T>
concept ByteOrConstByte = AnyOf<T, std::byte, std::byte const>;

template <typename T>
concept ConvertibleToSpanOfBytes = requires(T&& t) {
    std::span{t};
    requires CastableToBytes<
        typename std::remove_cvref_t<decltype(std::span{t})>::element_type>;
};

template <typename T>
concept WritableSpan = requires(T s) {
    {
        std::as_writable_bytes(s)
    } -> AnyOf<
        std::span<std::byte, T::extent * sizeof(typename T::element_type)>,
        std::span<std::byte, std::dynamic_extent>>;
};

template <ConvertibleToSpanOfBytes T>
auto byte_span(T&& t)
{
    auto new_span = std::span(t);
    if constexpr (WritableSpan<decltype(new_span)>)
        return std::as_writable_bytes(new_span);
    else
        return std::as_bytes(new_span);
}

template <CastableToBytes T, size_t N>
auto byte_span(T (&arr)[N])
{
    return byte_span(std::span(arr));
}

inline auto byte_span(void* data, size_t len) -> ConstByteSpan<>
{
    return ConstByteSpan<>{static_cast<std::byte*>(data), len};
}

template <typename T>
concept PlainCastableToBytes =
    CastableToBytes<std::remove_cvref_t<T>> && !ConvertibleToSpanOfBytes<T>
    && std::is_lvalue_reference_v<T>;

template <PlainCastableToBytes T>
auto byte_span(T&& t)
{
    using V = std::remove_reference_t<T>;
    auto new_span = std::span<V, 1>(&t, 1);
    if constexpr (WritableSpan<decltype(new_span)>)
        return std::as_writable_bytes(new_span);
    else
        return std::as_bytes(new_span);
}

// dyn_byte_span: returns the dynamic-extent version of byte_span's result.
// Preserves const-ness: mutable source -> ByteSpan<>, const source -> ConstByteSpan<>.
template <typename T>
auto dyn_byte_span(T&& t)
{
    auto s = byte_span(std::forward<T>(t));
    using S = decltype(s);
    if constexpr (std::is_same_v<typename S::element_type, std::byte>)
        return ByteSpan<>{s.data(), s.size()};
    else
        return ConstByteSpan<>{s.data(), s.size()};
}

inline auto dyn_byte_span(ByteSpan<> s) -> ByteSpan<> { return s; }
inline auto dyn_byte_span(ConstByteSpan<> s) -> ConstByteSpan<> { return s; }

} // namespace mpk::mix
