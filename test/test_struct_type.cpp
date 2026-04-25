/** @file
 * @brief TODO: Brief docstring.
 *
 * TODO: More documentation here
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/struct_type.hpp"
#include "mpk/mix/struct_type_macro.hpp"

#include <gtest/gtest.h>

#include <string>

using namespace mpk::mix;

// ---- Test struct with fields -----------------------------------------------

struct Point
{
    int x;
    int y;
};

MPKMIX_STRUCT_TYPE(Point, x, y);

static_assert(StructType<Point>);

TEST(StructType, FieldsOf)
{
    Point p{3, 7};
    auto [x, y] = fields_of(p);
    EXPECT_EQ(x, 3);
    EXPECT_EQ(y, 7);

    x = 10;
    EXPECT_EQ(p.x, 10);
}

TEST(StructType, FieldsOfConst)
{
    const Point p{4, 5};
    auto [x, y] = fields_of(p);
    EXPECT_EQ(x, 4);
    EXPECT_EQ(y, 5);
}

TEST(StructType, TuplTagOf)
{
    using ExpectedTuple = std::tuple<int, int>;
    static_assert(
        std::is_same_v<
            decltype(tuple_tag_of(Type<Point>)),
            Type_Tag<ExpectedTuple>>);
}

TEST(StructType, FieldNamesOf)
{
    auto names = field_names_of(Type<Point>);
    ASSERT_EQ(names.size(), 2u);
    EXPECT_EQ(names[0], "x");
    EXPECT_EQ(names[1], "y");
}

// ---- Empty struct (zero fields) --------------------------------------------

struct Empty
{
};

MPKMIX_STRUCT_TYPE(Empty);

static_assert(StructType<Empty>);

TEST(StructType, EmptyFieldNames)
{
    auto names = field_names_of(Type<Empty>);
    EXPECT_EQ(names.size(), 0u);
}

// ---- Non-struct type should not satisfy StructType -------------------------

static_assert(!StructType<int>);
static_assert(!StructType<double>);
