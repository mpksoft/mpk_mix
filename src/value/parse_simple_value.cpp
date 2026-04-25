/** @file
 * @brief Implementation of simple scalar value parser
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/value/parse_simple_value.hpp"

#include "mpk/mix/value/type.hpp"
#include "mpk/mix/value/value.hpp"
#include "mpk/mix/value/value_path.hpp"

#include "mpk/mix/util/throw.hpp"

#include <cassert>
#include <charconv>
#include <type_traits>

using namespace std::string_view_literals;

namespace mpk::mix::value {
namespace {

template <typename T, typename... FromCharsArgs>
    requires std::is_integral_v<T> || std::is_floating_point_v<T>
auto parse_as_decimal(
    mpk::mix::Type_Tag<T>, std::string_view s, FromCharsArgs... from_chars_args)
    -> T
{
    T result{};
    auto r =
        std::from_chars(s.begin(), s.end(), result, from_chars_args...);

    if (r.ec == std::errc::invalid_argument)
        throw_(
            "parse_simple_value: Failed to parse scalar of type {} from "
            "string '{}' - not a number",
            type_of<T>(),
            s);
    else if (r.ec == std::errc::result_out_of_range)
        throw_(
            "parse_simple_value: Failed to parse scalar of type {} from "
            "string '{}' - out of range",
            type_of<T>(),
            s);
    assert(r.ec == std::error_code{});
    if (r.ptr != s.end())
        throw_(
            "parse_simple_value: Failed to parse scalar of type {} from "
            "string '{}' - extra characters remain",
            type_of<T>(),
            s);

    return result;
}

struct ScalarParser final
{
    template <typename T>
        requires std::is_integral_v<T>
    auto operator()(mpk::mix::Type_Tag<T> tag, std::string_view s) const
        -> Value
    {
        if (s.starts_with("0x"sv))
            return parse_as_decimal(tag, s.substr(2), 16);
        else if (s.starts_with("#"sv))
            return parse_as_decimal(tag, s.substr(1), 16);
        else
            return parse_as_decimal(tag, s);
    }

    template <typename T>
        requires std::is_floating_point_v<T>
    auto operator()(mpk::mix::Type_Tag<T> tag, std::string_view s) const
        -> Value
    {
        return parse_as_decimal(tag, s);
    }

    auto operator()(mpk::mix::Type_Tag<bool>, std::string_view s) const -> Value
    {
        if (s == "true")
            return true;
        else if (s == "false")
            return false;
        else
            throw_(
                "parse_simple_value: Failed to parse boolean scalar from "
                "string '{}'",
                s);
    }

    auto operator()(mpk::mix::Type_Tag<std::byte>, std::string_view) const
        -> Value
    {
        throw_("TODO: Parse std::byte");
    }
};

struct SimpleValueParser final
{
    auto operator()(const ArrayT& t, std::string_view) const -> Value
    {
        throw_(
            "parse_simple_value: Failed to parse value of type {} because "
            "array types are not supported",
            t.type());
    }

    auto operator()(const CustomT& t, std::string_view) const -> Value
    {
        throw_(
            "parse_simple_value: Failed to parse value of type {} because "
            "custom types are not supported",
            t.type());
    }

    auto operator()(const EnumT& t, std::string_view text) const -> Value
    {
        auto v = Value::make(t.type());
        // TODO better: pass `text` directly when it is supported
        v.set(ValuePath{"name"sv}, std::string{text});
        return v;
    }

    auto operator()(const PathT&, std::string_view text) const -> Value
    {
        return ValuePath::from_string(text);
    }

    auto operator()(const ScalarT& t, std::string_view text) const -> Value
    {
        return t.visit(ScalarParser{}, text);
    }

    auto operator()(const StringT&, std::string_view text) const -> Value
    {
        return std::string{text};
    }

    auto operator()(const SetT& t, std::string_view) const -> Value
    {
        throw_(
            "parse_simple_value: Failed to parse value of type {} because "
            "set types are not supported",
            t.type());
    }

    auto operator()(const StrongT& t, std::string_view text) const -> Value
    {
        auto weak_value = visit(t.weak_type(), SimpleValueParser{}, text);
        auto result = Value::make(t.type());
        result.set(ValuePath{} / "v"sv, weak_value);
        return result;
    }

    auto operator()(const StructT& t, std::string_view) const -> Value
    {
        throw_(
            "parse_simple_value: Failed to parse value of type {} because "
            "struct types are not supported",
            t.type());
    }

    auto operator()(const TupleT& t, std::string_view) const -> Value
    {
        throw_(
            "parse_simple_value: Failed to parse value of type {} because "
            "tuple types are not supported",
            t.type());
    }

    auto operator()(const VectorT& t, std::string_view) const -> Value
    {
        throw_(
            "parse_simple_value: Failed to parse value of type {} because "
            "vector types are not supported",
            t.type());
    }
};

} // anonymous namespace

auto parse_simple_value(std::string_view text, const Type* type) -> Value
{
    return visit(type, SimpleValueParser{}, text);
}

} // namespace mpk::mix::value
