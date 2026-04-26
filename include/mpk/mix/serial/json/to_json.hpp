/** @file
 * @brief Serialize structured data to nlohmann JSON
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/serial/concepts/map_like.hpp"
#include "mpk/mix/serial/detail/type_value_field_names.hpp"
#include "mpk/mix/serial/parse_simple_value.hpp"
#include "mpk/mix/serial/path.hpp"
#include "mpk/mix/serial/type_name.hpp"
#include "mpk/mix/strong/fwd.hpp"
#include "mpk/mix/struct_type.hpp"
#include "mpk/mix/util/time_point.hpp"
#include "mpk/mix/util/tuple_like.hpp"

#include <magic_enum/magic_enum.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <optional>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <variant>
#include <vector>

namespace mpk::mix::serial::json {

namespace detail {

template <typename T>
concept SimpleSerializableNonNumericType =
    requires(T& v, T const& cv, std::string_view sv, std::ostream& s) {
        requires !std::is_arithmetic_v<T>;
        requires !StrongNumericType<T>;
        parse_simple_value(v, sv);
        s << cv;
    };

template <typename T>
    requires std::is_arithmetic_v<T>
auto to_json(T const& value) -> nlohmann::json;

template <StrongNumericType T>
auto to_json(T const& value) -> nlohmann::json;

template <SimpleSerializableNonNumericType T>
auto to_json(T const& value) -> nlohmann::json;

template <typename E>
    requires std::is_enum_v<E>
auto to_json(E const& value) -> nlohmann::json;

template <StructType T>
auto to_json(T const& value) -> nlohmann::json;

template <tuple_like T>
auto to_json(T const& value) -> nlohmann::json;

template <typename T>
auto to_json(std::vector<T> const& value) -> nlohmann::json;

template <typename T>
auto to_json(std::optional<T> const& value) -> nlohmann::json;

template <typename... Ts>
auto to_json(std::variant<Ts...> const& value) -> nlohmann::json;

template <MapLike T>
auto to_json(T const& value) -> nlohmann::json;

// ---

template <typename T>
    requires std::is_arithmetic_v<T>
auto to_json(T const& value) -> nlohmann::json
{
    return value;
}

template <StrongNumericType T>
auto to_json(T const& value) -> nlohmann::json
{
    return value.v;
}

template <SimpleSerializableNonNumericType T>
auto to_json(T const& value) -> nlohmann::json
{
    if constexpr (std::same_as<T, std::filesystem::path>)
        return value.string();
    else if constexpr (ChronoTimepointType<T>)
#ifndef MPKMIX_NO_TZ
        return time_point_to_readable<std::chrono::nanoseconds>(value, "UTC");
#else
        return time_point_to_readable<std::chrono::nanoseconds>(value);
#endif
    else
    {
        std::ostringstream s;
        s << value;
        return s.str();
    }
}

template <typename E>
    requires std::is_enum_v<E>
auto to_json(E const& value) -> nlohmann::json
{
    return magic_enum::enum_name(value);
}

template <StructType T>
auto to_json(T const& value) -> nlohmann::json
{
    using TupleType = typename decltype(tuple_tag_of(Type<T>))::type;
    constexpr auto names = field_names_of(Type<T>);
    auto fields = fields_of(value);
    nlohmann::ordered_json result;
    [&]<size_t... I>(std::index_sequence<I...>)
    {
        ((result[names[I]] = to_json(std::get<I>(fields))), ...);
    }(std::make_index_sequence<std::tuple_size_v<TupleType>>());
    return result;
}

template <tuple_like T>
auto to_json(T const& value) -> nlohmann::json
{
    return [&]<size_t... I>(std::index_sequence<I...>)
    {
        return nlohmann::json::array({to_json(std::get<I>(value))...});
    }(std::make_index_sequence<std::tuple_size_v<T>>());
}

template <typename T>
auto to_json(std::vector<T> const& value) -> nlohmann::json
{
    auto result = nlohmann::json::array();
    for (auto const& element: value)
        result.push_back(to_json(element));
    return result;
}

template <typename T>
auto to_json(std::optional<T> const& value) -> nlohmann::json
{
    if (!value)
        return nlohmann::json::value_t::null;
    return to_json(*value);
}

template <typename... Ts>
auto to_json(std::variant<Ts...> const& value) -> nlohmann::json
{
    return std::visit(
        []<typename T>(T const& concrete_value)
        {
            return nlohmann::json(
                {{serial::detail::type_field_name, type_name(Type<T>)},
                 {serial::detail::value_field_name, to_json(concrete_value)}});
        },
        value);
}

template <MapLike T>
auto to_json(T const& value) -> nlohmann::json
{
    auto result = nlohmann::json{};
    for (auto const& [k, v]: value)
    {
        std::ostringstream s;
        s << k;
        result[s.str()] = to_json(v);
    }
    return result;
}

} // namespace detail

template <typename T>
auto to_json(T const& value) -> nlohmann::json
{
    return detail::to_json(value);
}

} // namespace mpk::mix::serial::json
