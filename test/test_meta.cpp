/** @file
 * @brief TODO: Brief docstring.
 *
 * TODO: More documentation here
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/meta.hpp"

#include <gtest/gtest.h>

using namespace mpk::mix;

// ---- Type_Tag / Type -------------------------------------------------------

static_assert(std::is_same_v<Type_Tag<int>::type, int>);
static_assert(std::is_same_v<decltype(Type<int>), const Type_Tag<int>>);
static_assert(std::is_same_v<decltype(Type<double>), const Type_Tag<double>>);

static_assert(TypeTagType<Type_Tag<int>>);
static_assert(TypeTagType<Type_Tag<double>>);
static_assert(!TypeTagType<int>);
static_assert(!TypeTagType<double>);

static_assert(!NonTypeTagType<Type_Tag<int>>);
static_assert(NonTypeTagType<int>);

// ---- Const_Tag / Const -----------------------------------------------------

static_assert(Const_Tag<42>::value == 42);
static_assert(std::is_same_v<Const_Tag<42>::type, int>);
static_assert(std::is_same_v<decltype(Const<42>), const Const_Tag<42>>);

// ---- TypePack / first_in_type_pack_t ---------------------------------------

static_assert(TypePack_Tag<>::size == 0);
static_assert(TypePack_Tag<int>::size == 1);
static_assert(TypePack_Tag<int, double, float>::size == 3);
static_assert(std::is_same_v<decltype(TypePack<int, double>),
                             const TypePack_Tag<int, double>>);

static_assert(std::is_same_v<first_in_type_pack_t<TypePack_Tag<int, double>>, int>);
static_assert(std::is_same_v<first_in_type_pack_t<TypePack_Tag<float>>, float>);

// ---- Nil -------------------------------------------------------------------

static_assert(std::is_same_v<decltype(Nil), const Nil_Tag>);

TEST(Meta, NilUsableAsDefaultMarker)
{
    auto x = Nil;
    (void)x;
}

// ---- Zero ------------------------------------------------------------------

static_assert(std::is_same_v<decltype(Zero), const Zero_Tag>);

TEST(Meta, ZeroComparisonInt)
{
    EXPECT_TRUE(Zero == 0);
    EXPECT_TRUE(0 == Zero);
    EXPECT_FALSE(Zero == 1);
    EXPECT_FALSE(1 == Zero);
}

TEST(Meta, ZeroComparisonDouble)
{
    EXPECT_TRUE(Zero == 0.0);
    EXPECT_TRUE(0.0 == Zero);
    EXPECT_FALSE(Zero == 1.0);
}

// ---- Func_Tag / Method_Tag -------------------------------------------------

namespace
{

auto free_add(int a, int b) -> int
{
    return a + b;
}

struct MyClass
{
    auto mul(int a, int b) -> int
    {
        return a * b;
    }
};

} // anonymous namespace

static_assert(std::is_same_v<
    Func_Tag<&free_add>::Signature,
    int(int, int)>);

static_assert(std::is_same_v<
    Method_Tag<&MyClass::mul>::Signature,
    int(int, int)>);

static_assert(std::is_same_v<
    Method_Tag<&MyClass::mul>::MethodClass,
    MyClass>);

TEST(Meta, FuncTagCall)
{
    constexpr auto func_ptr = Func_Tag<&free_add>::func;
    EXPECT_EQ(func_ptr(3, 4), 7);
}

TEST(Meta, MethodTagCall)
{
    auto obj = MyClass{};
    constexpr auto method_ptr = Method_Tag<&MyClass::mul>::method;
    EXPECT_EQ((obj.*method_ptr)(3, 4), 12);
}

// ---- EnumType --------------------------------------------------------------

enum class Color { Red, Green, Blue };
enum OldEnum { A, B };

static_assert(EnumType<Color>);
static_assert(EnumType<OldEnum>);
static_assert(!EnumType<int>);
static_assert(!EnumType<double>);
