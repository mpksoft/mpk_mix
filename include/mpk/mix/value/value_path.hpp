/** @file
 * @brief ValuePath, ValuePathItem, ValuePathView — path into a structured Value.
 *
 * Copyright (C) 2024-2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/util/any_of.hpp"
#include "mpk/mix/util/format_streamable.hpp"

#include <ostream>
#include <span>
#include <string_view>
#include <variant>
#include <vector>


namespace mpk::mix::value {

struct ValuePathItem final
{
public:
    ValuePathItem();  // Creates zero index

    /* implicit */ ValuePathItem(size_t index);
    /* implicit */ ValuePathItem(std::string_view name);
    /* implicit */ ValuePathItem(std::string name);

    auto is_index() const noexcept
        -> bool;

    auto is_name() const noexcept
        -> bool;

    auto index() const
        -> size_t;

    auto name() const
        -> std::string_view;

    friend auto operator<<(std::ostream& s, const ValuePathItem& item)
        -> std::ostream&;

    auto operator==(const ValuePathItem&) const noexcept -> bool;

    static auto from_string(std::string_view s)
        -> ValuePathItem;

private:
    using Storage =
        std::variant<size_t, std::string_view, std::string>;

    Storage storage_;
};

using ValuePathView = std::span<const ValuePathItem>;

auto operator<<(std::ostream& s, ValuePathView path)
    -> std::ostream&;


class ValuePath;

template <typename T>
concept ValuePathLikeType = AnyOf<T, ValuePathView, ValuePath>;

class ValuePath :
    public std::vector<ValuePathItem>
{
public:
    using Base = std::vector<ValuePathItem>;

    using Base::Base;

    inline auto operator/=(const ValuePathItem& i2)
        -> ValuePath&;

    template <ValuePathLikeType ValuePathLike>
    inline auto operator/=(const ValuePathLike& p2)
        -> ValuePath&;

    friend auto operator<<(std::ostream& s, const ValuePath& path)
        -> std::ostream&;

    static auto from_string(std::string_view s)
        -> ValuePath;
};

inline auto operator/(ValuePathItem i1, ValuePathItem i2)
    -> ValuePath;

template <ValuePathLikeType ValuePathLike>
inline auto operator/(const ValuePathLike& p1, ValuePathItem i2)
    -> ValuePath;

template <ValuePathLikeType ValuePathLike>
inline auto operator/(const ValuePathItem& i1, const ValuePath& p2)
    -> ValuePath;

template <ValuePathLikeType ValuePathLike1, ValuePathLikeType ValuePathLike2>
inline auto operator/(const ValuePathLike1& p1, const ValuePathLike2& p2)
    -> ValuePath;

auto operator/(ValuePathItem i1, ValuePathItem i2)
    -> ValuePath
{ return ValuePath{ i1, i2 }; }

template <ValuePathLikeType ValuePathLike>
inline auto operator/(const ValuePathLike& p1, ValuePathItem i2)
    -> ValuePath
{
    auto result = ValuePath{p1};
    result.push_back( i2 );
    return result;
}

template <ValuePathLikeType ValuePathLike>
inline auto operator/(const ValuePathItem& i1, const ValuePathLike& p2)
    -> ValuePath
{
    auto result = ValuePath{i1};
    result.insert(result.end(), p2.begin(), p2.end());
    return result;
}

template <ValuePathLikeType ValuePathLike1, ValuePathLikeType ValuePathLike2>
inline auto operator/(const ValuePathLike1& p1, const ValuePathLike2& p2)
    -> ValuePath
{
    auto result = ValuePath{p1};
    result.insert(result.end(), p2.begin(), p2.end());
    return result;
}

auto ValuePath::operator/=(const ValuePathItem& i2)
    -> ValuePath&
{ return *this = *this / i2; }

template <ValuePathLikeType ValuePathLike>
auto ValuePath::operator/=(const ValuePathLike& p2)
    -> ValuePath&
{ return *this = *this / p2; }

} // namespace mpk::mix::value

template <> struct std::formatter<mpk::mix::value::ValuePathView>
    : mpk::mix::OstreamFormatter<mpk::mix::value::ValuePathView> {};
template <> struct std::formatter<mpk::mix::value::ValuePath>
    : mpk::mix::OstreamFormatter<mpk::mix::value::ValuePath> {};
