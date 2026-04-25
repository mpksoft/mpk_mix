/** @file
 * @brief Parse JSON documents into structured data
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/func_ref/func_ref.hpp"
#include "mpk/mix/func_ref/tags.hpp"
#include "mpk/mix/serial/concepts/map_like.hpp"
#include "mpk/mix/serial/detail/type_value_field_names.hpp"
#include "mpk/mix/serial/parse_simple_value.hpp"
#include "mpk/mix/serial/path.hpp"
#include "mpk/mix/serial/type_name.hpp"
#include "mpk/mix/strong/fwd.hpp"
#include "mpk/mix/struct_type.hpp"
#include "mpk/mix/util/throw.hpp"
#include "mpk/mix/util/tuple_like.hpp"

#include <nlohmann/json.hpp>

#include <array>
#include <cassert>
#include <optional>
#include <span>
#include <string>
#include <tuple>
#include <type_traits>
#include <variant>
#include <vector>

namespace mpk::mix::serial::json {

namespace detail {

template <typename T>
    requires std::is_arithmetic_v<T>
auto parse_value(T& value, nlohmann::json const& node, Path& path) -> void;

template <StrongNumericType T>
auto parse_value(T& value, nlohmann::json const& node, Path& path) -> void;

template <typename T>
    requires requires(T& value, std::string_view s) {
        requires !std::is_arithmetic_v<T>;
        requires !StrongNumericType<T>;
        parse_simple_value(value, s);
    }
auto parse_value(T& value, nlohmann::json const& node, Path& path) -> void;

template <StructType T>
auto parse_value(T& value, nlohmann::json const& node, Path& path) -> void;

template <tuple_like T>
auto parse_value(T& value, nlohmann::json const& node, Path& path) -> void;

template <typename T>
auto parse_value(
    std::vector<T>& value, nlohmann::json const& node, Path& path) -> void;

template <typename T>
auto parse_value(
    std::optional<T>& value, nlohmann::json const& node, Path& path) -> void;

template <typename... Ts>
auto parse_value(
    std::variant<Ts...>& value, nlohmann::json const& node, Path& path)
    -> void;

template <MapLike T>
auto parse_value(T& value, nlohmann::json const& node, Path& path) -> void;

// ---

template <typename T>
    requires std::is_arithmetic_v<T>
auto parse_value(T& value, nlohmann::json const& node, Path& path) -> void
{
    try
    {
        node.get_to(value);
    }
    catch (std::exception& e)
    {
        throw_<std::invalid_argument>(
            "Failed to parse JSON value at {}: {}", path, e.what());
    }
}

template <StrongNumericType T>
auto parse_value(T& value, nlohmann::json const& node, Path& path) -> void
{
    try
    {
        node.get_to(value.v);
    }
    catch (std::exception& e)
    {
        throw_<std::invalid_argument>(
            "Failed to parse JSON value at {}: {}", path, e.what());
    }
}

template <typename T>
    requires requires(T& value, std::string_view s) {
        requires !std::is_arithmetic_v<T>;
        requires !StrongNumericType<T>;
        parse_simple_value(value, s);
    }
auto parse_value(T& value, nlohmann::json const& node, Path& path) -> void
{
    try
    {
        parse_simple_value(value, node.get<std::string>());
    }
    catch (std::exception& e)
    {
        throw_<std::invalid_argument>(
            "Failed to parse JSON value at {}: {}", path, e.what());
    }
}

auto check_field_names(
    std::span<std::string_view const> field_names,
    nlohmann::json const& node,
    PathView path) -> void;

auto check_variant_fields(nlohmann::json const& node, PathView path) -> void;

auto check_variant_type(
    std::span<std::string_view const> variant_types,
    std::string_view parsed_type,
    PathView path) -> void;

auto with_child_node(
    nlohmann::json const& parent_node,
    PathItem child_key,
    FuncRef<void(nlohmann::json const&)> f,
    Path& parent_path) -> void;

auto node_keys(nlohmann::json const& node, PathView path)
    -> std::vector<std::string>;

template <StructType T>
auto parse_value(T& value, nlohmann::json const& node, Path& path) -> void
{
    using TupleType = typename decltype(tuple_tag_of(Type<T>))::type;
    constexpr auto names = field_names_of(Type<T>);
    check_field_names(names, node, path);
    constexpr auto n = std::tuple_size_v<TupleType>;
    assert(node.size() == n);

    auto fields = fields_of(value);
    auto parse_one = [&]<size_t I>(Const_Tag<I>)
    {
        ScopedPathAppender path_app(path, names[I]);
        parse_value(std::get<I>(fields), node[names[I]], path);
    };

    [&]<size_t... I>(std::index_sequence<I...>)
    { (parse_one(Const<I>), ...); }(std::make_index_sequence<n>());
}

template <tuple_like T>
auto parse_value(T& value, nlohmann::json const& node, Path& path) -> void
{
    constexpr auto n = std::tuple_size_v<T>;
    if (node.size() != n)
        throw_(
            "Failed to parse value at path {} because of tuple size mismatch: "
            "expected {}, got {}",
            path,
            n,
            node.size());

    auto parse_one = [&]<size_t I>(Const_Tag<I>)
    {
        ScopedPathAppender path_app(path, I);
        parse_value(std::get<I>(value), node[I], path);
    };

    [&]<size_t... I>(std::index_sequence<I...>)
    { (parse_one(Const<I>), ...); }(std::make_index_sequence<n>());
}

template <typename T>
auto parse_value(
    std::vector<T>& value, nlohmann::json const& node, Path& path) -> void
{
    auto n = node.size();
    value.clear();
    value.reserve(n);
    for (size_t i = 0; i < n; ++i)
    {
        auto& element = value.emplace_back();
        ScopedPathAppender path_app(path, i);
        parse_value(element, node[i], path);
    }
}

template <typename T>
auto parse_value(
    std::optional<T>& value, nlohmann::json const& node, Path& path) -> void
{
    if (node.empty())
        return value.reset();
    if (node.is_string() && node.get<std::string>() == "none")
        return value.reset();
    value = T{};
    parse_value(*value, node, path);
}

template <typename... Ts>
auto parse_value(
    std::variant<Ts...>& value, nlohmann::json const& node, Path& path) -> void
{
    constexpr auto types =
        std::array<std::string_view, sizeof...(Ts)>{type_name(Type<Ts>)...};

    check_variant_fields(node, path);

    std::string type;
    auto parse_type = [&](nlohmann::json const& child_node)
    {
        parse_value(type, child_node, path);
        check_variant_type(types, type, path);
    };

    with_child_node(node, serial::detail::type_field_name, &parse_type, path);

    auto maybe_parse_value =
        [&]<size_t I, typename T>(
            nlohmann::json const& child_node, Const_Tag<I>, Type_Tag<T>)
    {
        if (type != types[I])
            return;
        auto concrete_value = T{};
        parse_value(concrete_value, child_node, path);
        value = concrete_value;
    };

    auto parse_val = [&](nlohmann::json const& child_node)
    {
        [&]<size_t... I>(std::index_sequence<I...>)
        {
            (maybe_parse_value(child_node, Const<I>, Type<Ts>), ...);
        }(std::index_sequence_for<Ts...>());
    };

    with_child_node(
        node, serial::detail::value_field_name, &parse_val, path);
}

template <MapLike T>
auto parse_value(T& value, nlohmann::json const& node, Path& path) -> void
{
    for (auto const& key_str: node_keys(node, path))
    {
        auto parse_item = [&](nlohmann::json const& value_node)
        {
            auto item_key = typename T::key_type{};
            parse_simple_value(item_key, key_str);

            auto item_value = typename T::mapped_type{};
            parse_value(item_value, value_node, path);

            value.emplace(item_key, item_value);
        };
        with_child_node(node, key_str, &parse_item, path);
    }
}

} // namespace detail

template <typename T>
auto parse_value(
    T& value, nlohmann::json const& node, PathView root = {}) -> void
{
    auto path = Path::from_view(root);
    detail::parse_value(value, node, path);
}

template <typename T>
auto parse_value(
    nlohmann::json const& node, Type_Tag<T> = {}, PathView root = {}) -> T
{
    auto result = T{};
    parse_value(result, node, root);
    return result;
}

} // namespace mpk::mix::serial::json
