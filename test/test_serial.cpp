/** @file
 * @brief TODO: Brief docstring.
 *
 * TODO: More documentation here
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/serial/json/from_file.hpp"
#include "mpk/mix/serial/json/parse.hpp"
#include "mpk/mix/serial/json/to_json.hpp"
#include "mpk/mix/serial/parse_simple_value.hpp"
#include "mpk/mix/serial/path.hpp"
#include "mpk/mix/serial/type_name.hpp"
#include "mpk/mix/serial/yaml/parse.hpp"
#include "mpk/mix/util/time_point.hpp"

#include <yaml-cpp/yaml.h>

#include <gtest/gtest.h>

namespace mpk::mix
{

MPKMIX_DECL_TYPE_NAME(int, "int");

} // namespace mpk::mix

using namespace mpk::mix;

TEST(Serial_Path, default_constructed_path_is_empty)
{
    Path p;
    EXPECT_TRUE(p.empty());
}

TEST(Serial_Path, path_item_index)
{
    PathItem item{size_t{3}};
    EXPECT_TRUE(item.is_index());
    EXPECT_EQ(item.index(), 3u);
}

TEST(Serial_Path, path_item_name)
{
    PathItem item{std::string_view{"foo"}};
    EXPECT_TRUE(item.is_name());
    EXPECT_EQ(item.name(), "foo");
}

TEST(Serial_Path, path_item_from_string_index)
{
    auto item = PathItem::from_string("42");
    EXPECT_TRUE(item.is_index());
    EXPECT_EQ(item.index(), 42u);
}

TEST(Serial_Path, path_item_from_string_name)
{
    auto item = PathItem::from_string("field");
    EXPECT_TRUE(item.is_name());
    EXPECT_EQ(item.name(), "field");
}

TEST(Serial_Path, path_from_string)
{
    auto p = Path::from_string("/a/0/b");
    ASSERT_EQ(p.size(), 3u);
    EXPECT_EQ(p[0].name(), "a");
    EXPECT_EQ(p[1].index(), 0u);
    EXPECT_EQ(p[2].name(), "b");
}

TEST(Serial_ParseSimpleValue, int_decimal)
{
    int v{};
    parse_simple_value(v, "42");
    EXPECT_EQ(v, 42);
}

TEST(Serial_ParseSimpleValue, int_hex)
{
    int v{};
    parse_simple_value(v, "0xFF");
    EXPECT_EQ(v, 255);
}

TEST(Serial_ParseSimpleValue, bool_true)
{
    bool v{};
    parse_simple_value(v, "true");
    EXPECT_TRUE(v);
}

TEST(Serial_ParseSimpleValue, bool_false)
{
    bool v{};
    parse_simple_value(v, "false");
    EXPECT_FALSE(v);
}

TEST(Serial_ParseSimpleValue, string)
{
    std::string v;
    parse_simple_value(v, "hello");
    EXPECT_EQ(v, "hello");
}

TEST(Serial_ParseSimpleValue, nanoseconds_ms)
{
    std::chrono::nanoseconds v{};
    parse_simple_value(v, "5ms");
    EXPECT_EQ(v, std::chrono::milliseconds{5});
}

TEST(Serial_Yaml, parse_scalar)
{
    auto node = YAML::Load("42");
    Path path;
    int v{};
    serial::yaml::detail::parse_value(v, node, path);
    EXPECT_EQ(v, 42);
}

TEST(Serial_Yaml, parse_string)
{
    auto node = YAML::Load("hello");
    Path path;
    std::string v;
    serial::yaml::detail::parse_value(v, node, path);
    EXPECT_EQ(v, "hello");
}

TEST(Serial_Yaml, parse_sequence)
{
    auto node = YAML::Load("[1, 2, 3]");
    Path path;
    std::vector<int> v;
    serial::yaml::detail::parse_value(v, node, path);
    ASSERT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 3);
}

TEST(Serial_Json, parse_scalar)
{
    auto node = nlohmann::json(42);
    Path path;
    int v{};
    serial::json::detail::parse_value(v, node, path);
    EXPECT_EQ(v, 42);
}

TEST(Serial_Json, parse_string)
{
    auto node = nlohmann::json(std::string{"hello"});
    Path path;
    std::string v;
    serial::json::detail::parse_value(v, node, path);
    EXPECT_EQ(v, "hello");
}

TEST(Serial_Json, to_json_int)
{
    nlohmann::json j;
    to_json(j, 42);
    EXPECT_EQ(j.get<int>(), 42);
}

TEST(Serial_TypeName, int_type_name)
{
    auto name = type_name(Type_Tag<int>{});
    EXPECT_EQ(name, "int");
}
