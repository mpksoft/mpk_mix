/** @file
 * @brief Implementation of YAML-to-Value parser
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/serial/yaml/parse_value.hpp"

#include "mpk/mix/value/parse_simple_value.hpp"
#include "mpk/mix/value/type.hpp"
#include "mpk/mix/value/value.hpp"
#include "mpk/mix/value/value_path.hpp"

#include "mpk/mix/util/format_streamable.hpp"
#include "mpk/mix/util/throw.hpp"

#include <yaml-cpp/yaml.h>

MPKMIX_DECL_OSTREAM_FORMATTER(YAML::Node);

using namespace std::string_view_literals;

namespace mpk::mix::serial::yaml {
namespace {

using value::ArrayT;
using value::CustomT;
using value::EnumT;
using value::PathT;
using value::ScalarT;
using value::SetT;
using value::StringT;
using value::StrongT;
using value::StructT;
using value::TupleT;
using value::Value;
using value::ValuePath;
using value::VectorT;
using value::visit;

struct YamlValueParser final
{
    auto operator()(const ArrayT& t, Value& value, const YAML::Node& node) const
        -> void
    {
        if (value.size() != node.size())
            throw_(
                "YamlValueParser: Failed to parse value of type {} because "
                "actual array size {} differs from expected size {}",
                t.type(),
                node.size(),
                value.size());

        for (const auto& key: value.path_item_keys())
        {
            auto element_value_node = node[key.index()];

            auto path = ValuePath{} / key;
            auto element_value = value.get(path);
            visit(
                element_value.type(),
                YamlValueParser{},
                element_value,
                element_value_node);

            value.set(path, element_value);
        }
    }

    auto operator()(const CustomT& t, Value&, const YAML::Node&) const -> void
    {
        throw_(
            "YamlValueParser: Failed to parse value of type {} because custom "
            "types are not supported",
            t.type());
    }

    auto operator()(const EnumT& t, Value& value, const YAML::Node& node) const
        -> void
    {
        value = value::parse_simple_value(node.as<std::string>(), t.type());
    }

    auto operator()(const PathT& t, Value& value, const YAML::Node& node) const
        -> void
    {
        value = value::parse_simple_value(node.as<std::string>(), t.type());
    }

    auto operator()(
        const ScalarT& t, Value& value, const YAML::Node& node) const -> void
    {
        value = value::parse_simple_value(node.as<std::string>(), t.type());
    }

    auto operator()(const SetT& t, Value& value, const YAML::Node& node) const
        -> void
    {
        value.set_default();
        const auto* key_type = t.key_type();
        if (node.IsSequence())
        {
            for (size_t index = 0, n = node.size(); index < n; ++index)
            {
                auto key_node = node[index];
                auto k = Value::make(key_type);
                visit(key_type, YamlValueParser{}, k, key_node);
                value.insert(k);
            }
        }
        else if (node.IsScalar())
        {
            auto node_str = node.as<std::string>();
            if (node_str == "all")
            {
                auto k = Value::make(key_type);
                auto names = k.get(ValuePath{"names"sv})
                                 .as<std::vector<std::string_view>>();
                for (auto name: names)
                {
                    k.set(ValuePath{"name"sv}, std::string{name});
                    value.insert(k);
                }
            }
            else
                throw_(
                    "YamlValueParser: Failed to parse value of type {} - if "
                    "specified as a scalar, only 'all' is recognized",
                    t.type());
        }
        else
            throw_(
                "YamlValueParser: Failed to parse value of type {} - node is "
                "neither an array nor a scalar",
                t.type());
    }

    auto operator()(
        const StringT& t, Value& value, const YAML::Node& node) const -> void
    {
        value = value::parse_simple_value(node.as<std::string>(), t.type());
    }

    auto operator()(
        const StrongT& t, Value& value, const YAML::Node& node) const -> void
    {
        value = value::parse_simple_value(node.as<std::string>(), t.type());
    }

    auto operator()(
        const StructT&, Value& value, const YAML::Node& node) const -> void
    {
        for (const auto& key: value.path_item_keys())
        {
            auto field_value_node = node[key.name()];
            if (!field_value_node)
                continue;

            auto path = ValuePath{} / key;

            auto field_value = value.get(path);

            visit(
                field_value.type(),
                YamlValueParser{},
                field_value,
                field_value_node);

            value.set(path, field_value);
        }
    }

    auto operator()(const TupleT& t, Value& value, const YAML::Node& node) const
        -> void
    {
        auto keys = value.path_item_keys();
        if (node.size() != keys.size())
            throw_(
                "YamlValueParser: Failed to parse value of type {} because of "
                "tuple size mismatch",
                t.type());

        for (const auto& key: keys)
        {
            auto element_value_node = node[key.index()];

            auto path = ValuePath{} / key;
            auto element_value = value.get(path);
            visit(
                element_value.type(),
                YamlValueParser{},
                element_value,
                element_value_node);

            value.set(path, element_value);
        }
    }

    auto operator()(
        const VectorT&, Value& value, const YAML::Node& node) const -> void
    {
        value.resize({}, node.size());
        for (const auto& key: value.path_item_keys())
        {
            auto element_value_node = node[key.index()];

            auto path = ValuePath{} / key;
            auto element_value = value.get(path);
            visit(
                element_value.type(),
                YamlValueParser{},
                element_value,
                element_value_node);

            value.set(path, element_value);
        }
    }
};

} // anonymous namespace

auto parse_value(
    const YAML::Node& node,
    const value::Type* type,
    const value::ValueRegistry<const value::Type*>&) -> value::Value
{
    auto result = Value::make(type);
    visit(type, YamlValueParser{}, result, node);
    return result;
}

auto parse_value(
    const YAML::Node& node,
    const value::ValueRegistry<const value::Type*>& type_registry)
    -> value::Value
{
    auto type_name = node["type"].as<std::string>();
    const auto* type = type_registry.at(type_name);

    auto value_node = node["value"];
    if (!value_node)
        throw_("parse_value failed: Value is missing for node\n{}", node);

    return parse_value(value_node, type, type_registry);
}

} // namespace mpk::mix::serial::yaml
