/** @file
 * @brief Tests for mpk::mix::EnumFlags<E>.
 *
 * Copyright (C) 2025-2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/detail/set_like.hpp"
#include "mpk/mix/enum_flags.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <format>


namespace {

enum class Fruit : uint8_t
{
    Apple,
    Banana,
    Orange
};

using FruitFlags = mpk::mix::EnumFlags<Fruit>;

static_assert(mpk::mix::EnumFlagsType<FruitFlags>);
static_assert(mpk::mix::detail::SetLikeType<FruitFlags>);

} // anonymous namespace


TEST(MpkMix_EnumFlags, Basic)
{
    auto flags = FruitFlags{Fruit::Apple, Fruit::Banana};

    EXPECT_EQ(std::format("{}", flags), "{Apple, Banana}");
    EXPECT_TRUE(!flags.all());
    EXPECT_TRUE(flags.any());
    EXPECT_EQ(flags.size(), 2);
    EXPECT_FALSE(flags.empty());
    EXPECT_TRUE(flags & Fruit::Apple);
    EXPECT_TRUE(flags & Fruit::Banana);
    EXPECT_FALSE(flags & Fruit::Orange);

    flags &= ~FruitFlags{Fruit::Apple};
    EXPECT_EQ(std::format("{}", flags), "{Banana}");
    EXPECT_TRUE(!flags.all());
    EXPECT_TRUE(flags.any());
    EXPECT_EQ(flags.size(), 1);
    EXPECT_FALSE(flags.empty());
    EXPECT_FALSE(flags & Fruit::Apple);
    EXPECT_TRUE(flags & Fruit::Banana);
    EXPECT_FALSE(flags & Fruit::Orange);

    flags.clear(Fruit::Banana);
    EXPECT_EQ(std::format("{}", flags), "{}");
    EXPECT_TRUE(!flags.all());
    EXPECT_TRUE(!flags.any());
    EXPECT_EQ(flags.size(), 0);
    EXPECT_TRUE(flags.empty());

    flags ^= Fruit::Orange;
    EXPECT_TRUE(flags & Fruit::Orange);
    EXPECT_EQ(std::format("{}", flags), "{Orange}");

    flags ^= Fruit::Orange;
    EXPECT_FALSE(flags & Fruit::Orange);

    flags |= FruitFlags{Fruit::Apple, Fruit::Banana, Fruit::Orange};
    EXPECT_EQ(std::format("{}", flags), "{Apple, Banana, Orange}");
    EXPECT_TRUE(flags.all());
    EXPECT_EQ(flags.size(), 3);
}

TEST(MpkMix_EnumFlags, Set)
{
    auto flags = FruitFlags{};

    EXPECT_TRUE(flags.empty());

    {
        auto [it, inserted] = flags.insert(Fruit::Banana);
        EXPECT_EQ(*it, Fruit::Banana);
        EXPECT_TRUE(inserted);
        ++it;
        EXPECT_EQ(it, flags.end());
        EXPECT_FALSE(flags.empty());
        EXPECT_EQ(std::distance(flags.begin(), flags.end()), 1);
    }

    {
        auto [it, inserted] = flags.insert(Fruit::Orange);
        EXPECT_EQ(*it, Fruit::Orange);
        EXPECT_TRUE(inserted);
        ++it;
        EXPECT_EQ(it, flags.end());
        EXPECT_EQ(std::distance(flags.begin(), flags.end()), 2);
    }

    {
        auto [it, inserted] = flags.insert(Fruit::Banana);
        EXPECT_EQ(*it, Fruit::Banana);
        EXPECT_FALSE(inserted);
        ++it;
        EXPECT_EQ(*it, Fruit::Orange);
        ++it;
        EXPECT_EQ(it, flags.end());
        EXPECT_EQ(std::distance(flags.begin(), flags.end()), 2);
    }

    {
        auto it = flags.find(Fruit::Banana);
        EXPECT_NE(it, flags.end());
        it = flags.erase(it);
        EXPECT_EQ(*it, Fruit::Orange);
        ++it;
        EXPECT_EQ(it, flags.end());
        EXPECT_EQ(std::distance(flags.begin(), flags.end()), 1);
    }

    {
        auto it = flags.find(Fruit::Apple);
        EXPECT_EQ(it, flags.end());
    }
}
