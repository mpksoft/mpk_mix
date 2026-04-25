/** @file
 * @brief TODO: Brief docstring.
 *
 * TODO: More documentation here
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/serial/yaml/parse_value.hpp"

#include "mpk/mix/struct_type_macro.hpp"
#include "mpk/mix/value/parse_simple_value.hpp"
#include "mpk/mix/value/type.hpp"
#include "mpk/mix/value/value.hpp"

#include <yaml-cpp/yaml.h>

#include <gtest/gtest.h>

using namespace std::string_view_literals;
namespace mv = mpk::mix::value;
namespace my = mpk::mix::serial::yaml;

namespace {

enum class Color : uint8_t
{
    Red,
    Green,
    Blue
};

struct Point
{
    int32_t x{};
    int32_t y{};
};

MPKMIX_STRUCT_TYPE(Point, x, y);

MPKMIX_STRONG_TYPE(OrderId, uint64_t);

} // anonymous namespace

MPKMIX_VALUE_REGISTER_ENUM_TYPE(Color, 1);

TEST(MpkMix_Value_ParseSimpleValue, Int)
{
    const auto* t = mv::Type::of<int32_t>();
    EXPECT_EQ(mv::parse_simple_value("42"sv, t).as<int32_t>(), 42);
    EXPECT_EQ(mv::parse_simple_value("0x10"sv, t).as<int32_t>(), 16);
    EXPECT_EQ(mv::parse_simple_value("#ff"sv, t).as<int32_t>(), 255);
    EXPECT_THROW(mv::parse_simple_value("abc"sv, t), std::runtime_error);
}

TEST(MpkMix_Value_ParseSimpleValue, Bool)
{
    const auto* t = mv::Type::of<bool>();
    EXPECT_TRUE(mv::parse_simple_value("true"sv, t).as<bool>());
    EXPECT_FALSE(mv::parse_simple_value("false"sv, t).as<bool>());
    EXPECT_THROW(mv::parse_simple_value("maybe"sv, t), std::runtime_error);
}

TEST(MpkMix_Value_ParseSimpleValue, Enum)
{
    const auto* t = mv::Type::of<Color>();
    EXPECT_EQ(mv::parse_simple_value("Red"sv, t).as<Color>(), Color::Red);
    EXPECT_EQ(mv::parse_simple_value("Green"sv, t).as<Color>(), Color::Green);
}

TEST(MpkMix_Value_ParseSimpleValue, Strong)
{
    const auto* t = mv::Type::of<OrderId>();
    EXPECT_EQ(mv::parse_simple_value("1234"sv, t).as<OrderId>(), OrderId{1234});
    EXPECT_EQ(
        mv::parse_simple_value("0x10"sv, t).as<OrderId>(), OrderId{16});
}

TEST(MpkMix_Value_ParseSimpleValue, String)
{
    const auto* t = mv::Type::of<std::string>();
    EXPECT_EQ(mv::parse_simple_value("hello"sv, t).as<std::string>(), "hello");
}

TEST(MpkMix_SerialYaml_ParseValue, Struct)
{
    const auto* t = mv::Type::of<Point>();
    auto node = YAML::Load("x: 3\ny: 7\n");
    auto v = my::parse_value(node, t, {});
    auto p = v.as<Point>();
    EXPECT_EQ(p.x, 3);
    EXPECT_EQ(p.y, 7);
}

TEST(MpkMix_SerialYaml_ParseValue, Vector)
{
    const auto* t = mv::Type::of<std::vector<int32_t>>();
    auto node = YAML::Load("[1, 2, 3, 4]");
    auto v = my::parse_value(node, t, {});
    auto vec = v.as<std::vector<int32_t>>();
    ASSERT_EQ(vec.size(), 4u);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[3], 4);
}

TEST(MpkMix_SerialYaml_ParseValue, TypeFieldDispatch)
{
    mv::ValueRegistry<const mv::Type*> reg;
    reg.register_value("Point", mv::Type::of<Point>());

    auto node = YAML::Load("type: Point\nvalue:\n  x: 10\n  y: 20\n");
    auto v = my::parse_value(node, reg);
    auto p = v.as<Point>();
    EXPECT_EQ(p.x, 10);
    EXPECT_EQ(p.y, 20);
}

TEST(MpkMix_SerialYaml_ParseValue, MissingValueFieldThrows)
{
    mv::ValueRegistry<const mv::Type*> reg;
    reg.register_value("Point", mv::Type::of<Point>());

    auto node = YAML::Load("type: Point\n");
    EXPECT_THROW(my::parse_value(node, reg), std::runtime_error);
}

TEST(MpkMix_Value_ValueRegistry, AtAndMaybeAt)
{
    mv::ValueRegistry<int> reg;
    reg.register_value("a", 1);
    reg.register_value("b", 2);
    EXPECT_EQ(reg.at("a"), 1);
    EXPECT_EQ(reg.at("b"), 2);
    EXPECT_EQ(reg.maybe_at("c"), std::nullopt);
    EXPECT_THROW(reg.at("c"), std::domain_error);
    EXPECT_EQ(reg.size(), 2u);
}
