/** @file
 * @brief Serialize structured data to YAML
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/serial/concepts/map_like.hpp"
#include "mpk/mix/serial/detail/type_value_field_names.hpp"
#include "mpk/mix/serial/parse_simple_value.hpp"
#include "mpk/mix/serial/type_name.hpp"
#include "mpk/mix/strong/fwd.hpp"
#include "mpk/mix/struct_type.hpp"
#include "mpk/mix/util/tuple_like.hpp"

#include <magic_enum/magic_enum.hpp>
#include <yaml-cpp/yaml.h>

#include <chrono>
#include <filesystem>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

namespace mpk::mix::serial::yaml {

// Defined in to_yaml.cpp — picks the coarsest unit that divides evenly.
auto format_nanoseconds(std::chrono::nanoseconds value) -> std::string;

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
auto to_yaml(T const& value) -> YAML::Node;

template <StrongNumericType T>
auto to_yaml(T const& value) -> YAML::Node;

template <SimpleSerializableNonNumericType T>
auto to_yaml(T const& value) -> YAML::Node;

template <typename E>
    requires std::is_enum_v<E>
auto to_yaml(E const& value) -> YAML::Node;

auto to_yaml(std::chrono::nanoseconds const& value) -> YAML::Node;

template <typename Rep, typename Period>
    requires(!std::same_as<std::chrono::duration<Rep, Period>, std::chrono::nanoseconds>)
auto to_yaml(std::chrono::duration<Rep, Period> const& value) -> YAML::Node;

template <StructType T>
auto to_yaml(T const& value) -> YAML::Node;

template <tuple_like T>
auto to_yaml(T const& value) -> YAML::Node;

template <typename T>
auto to_yaml(std::vector<T> const& value) -> YAML::Node;

template <typename T>
auto to_yaml(std::optional<T> const& value) -> YAML::Node;

template <typename... Ts>
auto to_yaml(std::variant<Ts...> const& value) -> YAML::Node;

template <MapLike T>
auto to_yaml(T const& value) -> YAML::Node;

// ---

template <typename T>
    requires std::is_arithmetic_v<T>
auto to_yaml(T const& value) -> YAML::Node
{
    return YAML::Node{value};
}

template <StrongNumericType T>
auto to_yaml(T const& value) -> YAML::Node
{
    return YAML::Node{value.v};
}

template <SimpleSerializableNonNumericType T>
auto to_yaml(T const& value) -> YAML::Node
{
    if constexpr (std::same_as<T, std::filesystem::path>)
        return YAML::Node{value.string()};
    else
    {
        std::ostringstream s;
        s << value;
        return YAML::Node{s.str()};
    }
}

template <typename E>
    requires std::is_enum_v<E>
auto to_yaml(E const& value) -> YAML::Node
{
    return YAML::Node{std::string{magic_enum::enum_name(value)}};
}

inline auto to_yaml(std::chrono::nanoseconds const& value) -> YAML::Node
{
    return YAML::Node{format_nanoseconds(value)};
}

template <typename Rep, typename Period>
    requires(!std::same_as<std::chrono::duration<Rep, Period>, std::chrono::nanoseconds>)
auto to_yaml(std::chrono::duration<Rep, Period> const& value) -> YAML::Node
{
    return YAML::Node{value.count()};
}

template <StructType T>
auto to_yaml(T const& value) -> YAML::Node
{
    using TupleType = typename decltype(tuple_tag_of(Type<T>))::type;
    constexpr auto names = field_names_of(Type<T>);
    auto fields = fields_of(value);
    YAML::Node result(YAML::NodeType::Map);
    [&]<size_t... I>(std::index_sequence<I...>)
    {
        ((result[std::string{names[I]}] = to_yaml(std::get<I>(fields))), ...);
    }(std::make_index_sequence<std::tuple_size_v<TupleType>>());
    return result;
}

template <tuple_like T>
auto to_yaml(T const& value) -> YAML::Node
{
    YAML::Node result(YAML::NodeType::Sequence);
    [&]<size_t... I>(std::index_sequence<I...>)
    {
        (result.push_back(to_yaml(std::get<I>(value))), ...);
    }(std::make_index_sequence<std::tuple_size_v<T>>());
    return result;
}

template <typename T>
auto to_yaml(std::vector<T> const& value) -> YAML::Node
{
    YAML::Node result(YAML::NodeType::Sequence);
    for (auto const& el : value)
        result.push_back(to_yaml(el));
    return result;
}

template <typename T>
auto to_yaml(std::optional<T> const& value) -> YAML::Node
{
    if (!value)
        return YAML::Node{YAML::NodeType::Null};
    return to_yaml(*value);
}

template <typename... Ts>
auto to_yaml(std::variant<Ts...> const& value) -> YAML::Node
{
    return std::visit(
        []<typename T>(T const& concrete_value)
        {
            YAML::Node result(YAML::NodeType::Map);
            result[std::string{serial::detail::type_field_name}] =
                std::string{type_name(Type<T>)};
            result[std::string{serial::detail::value_field_name}] =
                to_yaml(concrete_value);
            return result;
        },
        value);
}

template <MapLike T>
auto to_yaml(T const& value) -> YAML::Node
{
    YAML::Node result(YAML::NodeType::Map);
    for (auto const& [k, v] : value)
    {
        std::ostringstream s;
        s << k;
        result[s.str()] = to_yaml(v);
    }
    return result;
}

} // namespace detail

template <typename T>
auto to_yaml(T const& value) -> YAML::Node
{
    return detail::to_yaml(value);
}

} // namespace mpk::mix::serial::yaml
