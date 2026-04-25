/** @file
 * @brief Implementation of JSON document parser
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/serial/json/parse.hpp"

#include "mpk/mix/util/format_seq.hpp"
#include "mpk/mix/util/throw.hpp"

#include <algorithm>
#include <ranges>
#include <sstream>
#include <unordered_set>

namespace mpk::mix::serial::json::detail {

auto check_field_names(
    std::span<std::string_view const> field_names,
    nlohmann::json const& node,
    PathView path) -> void
{
    if (!node.is_object())
        throw_<std::invalid_argument>(
            "Expected a JSON object at {} with fields {}",
            path,
            format_seq(field_names));

    auto has_field = [&](std::string_view name) -> bool
    { return std::ranges::find(field_names, name) != field_names.end(); };

    auto missing_names = std::vector<std::string_view>{};
    auto extra_names = std::vector<std::string>{};
    auto duplicate_names = std::unordered_set<std::string>{};

    auto node_keys = std::unordered_set<std::string>{};
    for (auto const& [name, _]: node.items())
    {
        if (!has_field(name))
            extra_names.push_back(name);
        auto inserted = node_keys.emplace(name).second;
        if (!inserted)
            duplicate_names.emplace(name);
    }

    for (auto field_name: field_names)
        if (!node_keys.contains(std::string{field_name}))
            missing_names.push_back(field_name);

    if (missing_names.empty() && extra_names.empty() && duplicate_names.empty())
        return;

    std::ostringstream s;
    s << "Failed to parse JSON node at " << path << ": ";
    auto const* delim = "";
    if (!missing_names.empty())
    {
        s << delim << "missing fields: " << format_seq(missing_names);
        delim = "; ";
    }
    if (!extra_names.empty())
    {
        s << delim << "extra fields: " << format_seq(extra_names);
        delim = "; ";
    }
    if (!duplicate_names.empty())
    {
        s << delim << "duplicate fields: " << format_seq(duplicate_names);
    }
    throw std::invalid_argument(s.str());
}

auto check_variant_fields(nlohmann::json const& node, PathView path) -> void
{
    if (!node.is_object())
        throw_<std::invalid_argument>(
            "Expected a JSON object at {} with fields type, value", path);

    auto extra_names = std::vector<std::string>{};
    for (auto const& [name, _]: node.items())
    {
        if (name != serial::detail::type_field_name
            && name != serial::detail::value_field_name)
            extra_names.push_back(name);
    }
    if (!extra_names.empty())
        throw_<std::invalid_argument>(
            "Expected just fields type and value at {}, "
            "got extra fields [{}]",
            path,
            format_seq(extra_names));

    if (node.size() != 2)
        throw_<std::invalid_argument>(
            "Expected just fields type and value at {}", path);
}

auto check_variant_type(
    std::span<std::string_view const> variant_types,
    std::string_view parsed_type,
    PathView path) -> void
{
    auto match_count = std::ranges::count_if(
        variant_types,
        [&](std::string_view t) { return t == parsed_type; });

    if (match_count == 0)
        throw_<std::invalid_argument>(
            "Failed to parse JSON node at {}: type must be one of [{}], "
            "got {}",
            path,
            format_seq(variant_types),
            parsed_type);

    if (match_count != 1)
        throw_<std::logic_error>(
            "Failed to parse JSON node at {}: {} is found "
            "among types [{}] more than once",
            path,
            parsed_type,
            format_seq(variant_types));
}

auto with_child_node(
    nlohmann::json const& parent_node,
    PathItem child_key,
    FuncRef<void(nlohmann::json const&)> f,
    Path& parent_path) -> void
{
    nlohmann::json child_node{};
    try
    {
        if (child_key.is_index())
            child_node = parent_node[child_key.index()];
        else
            child_node = parent_node[std::string{child_key.name()}];
    }
    catch (std::exception& e)
    {
        throw_<std::invalid_argument>(
            "Failed to access child node '{}' at path {}: {}",
            child_key,
            parent_path,
            e.what());
    }

    ScopedPathAppender path_app(parent_path, child_key);
    f(child_node);
}

auto node_keys(nlohmann::json const& node, PathView path)
    -> std::vector<std::string>
{
    if (!node.is_object())
        throw_<std::invalid_argument>(
            "Expected a JSON object at {}", path);

    auto result = std::vector<std::string>{};
    for (auto const& [name, _]: node.items())
        result.push_back(name);
    return result;
}

} // namespace mpk::mix::serial::json::detail
