/** @file
 * @brief Unit tests for mpk/mix/macro.hpp
 *
 * Copyright (C) 2025 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/macro.hpp"

#include <gtest/gtest.h>

#include <string_view>

// ---------------------------------------------------------------------------
// MPKMIX_NUM_ARGS
// The sentinel `extra` is the mandatory first argument — see macro.hpp.
// ---------------------------------------------------------------------------

static_assert(MPKMIX_NUM_ARGS(extra) == 0);
static_assert(MPKMIX_NUM_ARGS(extra, a) == 1);
static_assert(MPKMIX_NUM_ARGS(extra, a, b) == 2);
static_assert(MPKMIX_NUM_ARGS(extra, a, b, c) == 3);
static_assert(MPKMIX_NUM_ARGS(extra, a, b, c, d, e, f, g, h, i, j, k, l) == 12);

// ---------------------------------------------------------------------------
// MPKMIX_STRINGIZE
// ---------------------------------------------------------------------------

static_assert(std::string_view(MPKMIX_STRINGIZE(hello)) == "hello");
static_assert(std::string_view(MPKMIX_STRINGIZE(hello world)) == "hello world");
static_assert(std::string_view(MPKMIX_STRINGIZE(42)) == "42");

// ---------------------------------------------------------------------------
// MPKMIX_DEFAULT_A0_TO
// ---------------------------------------------------------------------------

static_assert(MPKMIX_DEFAULT_A0_TO(99)       == 99);  // no extra args → default
static_assert(MPKMIX_DEFAULT_A0_TO(99, 42)   == 42);  // one extra arg → A0
static_assert(MPKMIX_DEFAULT_A0_TO(99, 42, 0) == 42); // two extra args → A0

// ---------------------------------------------------------------------------
// MPKMIX_DEFAULT_A1_TO
// ---------------------------------------------------------------------------

static_assert(MPKMIX_DEFAULT_A1_TO(99)         == 99); // no extra args → default
static_assert(MPKMIX_DEFAULT_A1_TO(99, 0)      == 99); // one extra arg → default
static_assert(MPKMIX_DEFAULT_A1_TO(99, 0, 42)  == 42); // two extra args → A1
static_assert(MPKMIX_DEFAULT_A1_TO(99, 0, 42, 0) == 42);

// ---------------------------------------------------------------------------
// MPKMIX_MAP_SEP_LIST with MPKMIX_SEMICOLON — struct field declarations.
// Each element becomes a separate declaration, so SEMICOLON is the right
// separator.  MPKMIX_MAP_COMMA_SEP_LIST is for expression lists (e.g.
// template args, pointer-to-member lists) where each item has no keyword.
// ---------------------------------------------------------------------------

namespace {

#define MAKE_INT_FIELD(p, name) int name

struct ThreeFields
{
    MPKMIX_MAP_SEP_LIST(MAKE_INT_FIELD, _, MPKMIX_SEMICOLON, x, y, z);
};

#undef MAKE_INT_FIELD

static_assert(sizeof(ThreeFields) == 3 * sizeof(int));

// ---------------------------------------------------------------------------
// MPKMIX_MAP_SEP_LIST with MPKMIX_SEMICOLON — statement generation
// ---------------------------------------------------------------------------

#define ASSIGN_ONE(obj, field) obj.field = 1

TEST(Macro, MapSepListSemicolon)
{
    ThreeFields s{};
    MPKMIX_MAP_SEP_LIST(ASSIGN_ONE, s, MPKMIX_SEMICOLON, x, y, z);
    EXPECT_EQ(s.x, 1);
    EXPECT_EQ(s.y, 1);
    EXPECT_EQ(s.z, 1);
}

#undef ASSIGN_ONE

// ---------------------------------------------------------------------------
// MPKMIX_MAP_COMMA_SEP_LIST — expression / pointer-to-member list.
// This is the idiomatic use: generate a comma-separated list of expressions,
// not declarations.
// ---------------------------------------------------------------------------

#define PTR_TO_MBR(p, name) &p::name

TEST(Macro, MapCommaSepListExprList)
{
    using MemberPtr = int ThreeFields::*;
    MemberPtr ptrs[] = {
        MPKMIX_MAP_COMMA_SEP_LIST(PTR_TO_MBR, ThreeFields, x, y, z)
    };
    ThreeFields s{};
    s.*ptrs[0] = 10;
    s.*ptrs[1] = 20;
    s.*ptrs[2] = 30;
    EXPECT_EQ(s.x, 10);
    EXPECT_EQ(s.y, 20);
    EXPECT_EQ(s.z, 30);
}

#undef PTR_TO_MBR

// ---------------------------------------------------------------------------
// Empty variadic list — MAP_SEP_LIST with zero elements
// ---------------------------------------------------------------------------

#define MAKE_INT_FIELD2(p, name) int name

struct ZeroFields
{
    MPKMIX_MAP_SEP_LIST(MAKE_INT_FIELD2, _, MPKMIX_SEMICOLON);
};

#undef MAKE_INT_FIELD2

static_assert(sizeof(ZeroFields) == 1); // empty struct has size 1 in C++

} // namespace
