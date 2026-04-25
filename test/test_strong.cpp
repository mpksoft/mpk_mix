/** @file
 * @brief TODO: Brief docstring.
 *
 * TODO: More documentation here
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/strong.hpp"
#include "mpk/mix/util/index_range.hpp"

#include <gtest/gtest.h>

#include <sstream>
#include <string>
#include <string_view>

using namespace std::literals;

namespace
{

MPKMIX_STRONG_TYPE_WITH_DEFAULT(
    IntCountDefaultedTo33, int, 33, mpk::mix::StrongCountFeatures);

MPKMIX_STRONG_TYPE(Count, ptrdiff_t, mpk::mix::StrongCountFeatures);
MPKMIX_STRONG_TYPE(Index, ptrdiff_t, mpk::mix::StrongIndexFeatures<Count>);

MPKMIX_STRONG_TYPE(Count2, ptrdiff_t, mpk::mix::StrongCountFeatures);
MPKMIX_STRONG_TYPE(Index2, ptrdiff_t, mpk::mix::StrongIndexFeatures<Count2>);

MPKMIX_STRONG_TYPE(Id, int, mpk::mix::StrongIdFeatures);
MPKMIX_STRONG_TYPE(FloatId, double, mpk::mix::StrongIdFeatures);
MPKMIX_STRONG_TYPE(FloatCount, double, mpk::mix::StrongCountFeatures);
MPKMIX_STRONG_TYPE(
    FloatIndex, double, mpk::mix::StrongIndexFeatures<FloatCount>);

template <typename Lhs, typename Rhs>
concept CanAdd = requires(Lhs lhs, Rhs rhs) { lhs + rhs; };

template <typename Lhs, typename Rhs>
concept CanSub = requires(Lhs lhs, Rhs rhs) { lhs - rhs; };

MPKMIX_STRONG_STRING(Str);

} // anonymous namespace

TEST(MpkMix_Strong, Basic)
{
    using X = IntCountDefaultedTo33;
    X x;
    EXPECT_EQ(x.v, 33);
    static_assert(X::arithmetic);
    static_assert(X::is_count);
    static_assert(!X::is_index);

    X x1 = mpk::mix::Zero;
    EXPECT_EQ(x1.v, 0);

    X x2{1};
    EXPECT_EQ(x2.v, 1);

    auto sum = x + x2;
    static_assert(std::same_as<decltype(sum), X>);
    EXPECT_EQ(sum.v, 34);

    auto diff = x - x2;
    static_assert(std::same_as<decltype(sum), X>);
    EXPECT_EQ(diff.v, 32);
}

TEST(MpkMix_Strong, Arithmetic)
{
    auto i0 = Index{};
    auto i1 = Index{1};
    auto i2 = Index{5};
    auto c0 = Count{3};
    auto c1 = Count{4};

    EXPECT_EQ(i0.v, 0);
    EXPECT_EQ(i1.v, 1);
    EXPECT_EQ(i2.v, 5);
    EXPECT_EQ(c0.v, 3);
    EXPECT_EQ(c1.v, 4);

    static_assert(!CanAdd<Index, Index>);

    static_assert(CanAdd<Index, Count>);
    auto index_plus_count = i2 + c0;
    static_assert(std::same_as<decltype(index_plus_count), Index>);
    EXPECT_EQ(index_plus_count.v, 8);

    static_assert(CanAdd<Count, Index>);
    auto count_plus_index = c0 + i2;
    static_assert(std::same_as<decltype(count_plus_index), Index>);
    EXPECT_EQ(count_plus_index.v, 8);

    static_assert(CanAdd<Count, Count>);
    auto count_plus_count = c0 + c1;
    static_assert(std::same_as<decltype(count_plus_count), Count>);
    EXPECT_EQ(count_plus_count.v, 7);

    static_assert(CanSub<Index, Index>);
    auto index_minus_index = i2 - i1;
    static_assert(std::same_as<decltype(index_minus_index), Count>);
    EXPECT_EQ(index_minus_index.v, 4);

    static_assert(CanSub<Index, Count>);
    auto index_minus_count = i2 - c0;
    static_assert(std::same_as<decltype(index_minus_count), Index>);
    EXPECT_EQ(index_minus_count.v, 2);

    static_assert(!CanSub<Count, Index>);

    static_assert(CanSub<Count, Count>);
    auto count_minus_count = c0 - c1;
    static_assert(std::same_as<decltype(count_minus_count), Count>);
    EXPECT_EQ(count_minus_count.v, -1);

    EXPECT_EQ(i1++, Index{1});
    EXPECT_EQ(i1, Index{2});
    EXPECT_EQ(++i1, Index{3});
    EXPECT_EQ(i1, Index{3});
    EXPECT_EQ(i1++, Index{3});
    EXPECT_EQ(i1, Index{4});

    EXPECT_EQ(i1--, Index{4});
    EXPECT_EQ(i1, Index{3});
    EXPECT_EQ(--i1, Index{2});
    EXPECT_EQ(i1, Index{2});
    EXPECT_EQ(i1--, Index{2});
    EXPECT_EQ(i1, Index{1});

    i1 += c0;
    EXPECT_EQ(i1, Index{4});
    i1 -= c0;
    EXPECT_EQ(i1, Index{1});

    c0 += c1;
    EXPECT_EQ(c0, Count{7});
    c0 -= c1;
    EXPECT_EQ(c0, Count{3});
}

TEST(MpkMix_Strong, IntArithmeticScale)
{
    auto count = Count{42};
    [[maybe_unused]] auto index = Index{27};
    [[maybe_unused]] auto id = Id{3};

    EXPECT_EQ(count * 3, Count{126});
    EXPECT_EQ(3 * count, Count{126});
    EXPECT_EQ(count / 7, Count{6});

    count *= 3;
    EXPECT_EQ(count.v, 126);
    count /= 7;
    EXPECT_EQ(count.v, 18);
}

TEST(MpkMix_Strong, ConstexprIntArithmeticScale)
{
    constexpr auto count = Count{3};
    constexpr auto l_upscaled_count = 5 * count;
    constexpr auto r_upscaled_count = count * 5;
    constexpr auto downscaled_count = count / 2;

    static_assert(l_upscaled_count == Count{15});
    static_assert(r_upscaled_count == Count{15});
    static_assert(downscaled_count == Count{1});
}

TEST(MpkMix_Strong, FloatArithmeticScale)
{
    auto f_count = FloatCount{3.33};
    [[maybe_unused]] auto f_index = FloatIndex{3.33};
    [[maybe_unused]] auto f_id = FloatId{3.33};

    EXPECT_EQ(f_count * 3, FloatCount{9.99});
    EXPECT_EQ(3 * f_count, FloatCount{9.99});
    EXPECT_EQ(f_count / 3, FloatCount{1.11});
    EXPECT_EQ(f_count * 0.1, FloatCount{0.333});
    EXPECT_EQ(0.1 * f_count, FloatCount{0.333});

    f_count *= 3;
    EXPECT_EQ(f_count.v, 9.99);

    f_count /= 2;
    EXPECT_EQ(f_count.v, 4.995);
}

TEST(MpkMix_Strong, ConstexprFloatArithmeticScale)
{
    constexpr auto f_count = FloatCount{3};
    constexpr auto l_upscaled_f_count = 5 * f_count;
    constexpr auto r_upscaled_f_count = f_count * 5;
    constexpr auto downscaled_f_count = f_count / 2;

    static_assert(l_upscaled_f_count == FloatCount{15});
    static_assert(r_upscaled_f_count == FloatCount{15});
    static_assert(downscaled_f_count == FloatCount{1.5});
}

TEST(MpkMix_Strong, IntCompare)
{
    auto id1 = Id{1};
    auto id2 = Id{2};
    auto count1 = Count{1};
    auto count2 = Count{2};
    auto index1 = Index{1};
    auto index2 = Index{2};

    EXPECT_TRUE(id1 < id2);
    EXPECT_TRUE(count1 < count2);
    EXPECT_TRUE(index1 < index2);
    EXPECT_FALSE(id1 == id2);
    EXPECT_FALSE(count1 == count2);
    EXPECT_FALSE(index1 == index2);
    EXPECT_TRUE(id1 != id2);
    EXPECT_TRUE(count1 != count2);
    EXPECT_TRUE(index1 != index2);
}

TEST(MpkMix_Strong, FloatCompare)
{
    auto f_id1 = FloatId{1};
    auto f_id2 = FloatId{2};
    auto f_count1 = FloatCount{1};
    auto f_count2 = FloatCount{2};
    auto f_index1 = FloatIndex{1};
    auto f_index2 = FloatIndex{2};

    EXPECT_TRUE(f_id1 < f_id2);
    EXPECT_TRUE(f_count1 < f_count2);
    EXPECT_TRUE(f_index1 < f_index2);
    EXPECT_FALSE(f_id1 == f_id2);
    EXPECT_FALSE(f_count1 == f_count2);
    EXPECT_FALSE(f_index1 == f_index2);
    EXPECT_TRUE(f_id1 != f_id2);
    EXPECT_TRUE(f_count1 != f_count2);
    EXPECT_TRUE(f_index1 != f_index2);
}

TEST(MpkMix_Strong, IntMath)
{
    auto a = Index{123};
    auto b = Index{321};
    auto n = Count{-888};
    auto p = Count{444};

    EXPECT_EQ(max(a, b), Index{321});
    EXPECT_EQ(min(a, b), Index{123});
    EXPECT_EQ(-n, Count{888});
    EXPECT_EQ(magnitude(n), -n);
    EXPECT_EQ(magnitude(p), p);
}

TEST(MpkMix_Strong, FloatMath)
{
    auto a = FloatIndex{1.23};
    auto b = FloatIndex{3.51};
    auto n = FloatCount{-8.88};
    auto p = FloatCount{4.44};
    auto i = FloatIndex{5};

    EXPECT_EQ(max(a, b), FloatIndex{3.51});
    EXPECT_EQ(min(a, b), FloatIndex{1.23});
    EXPECT_EQ(-n, FloatCount{8.88});
    EXPECT_EQ(magnitude(n), -n);
    EXPECT_EQ(magnitude(p), p);

    EXPECT_EQ(floor(a), FloatIndex{1});
    EXPECT_EQ(floor(b), FloatIndex{3});
    EXPECT_EQ(floor(n), FloatCount{-9});
    EXPECT_EQ(floor(i), FloatIndex{5});

    EXPECT_EQ(ceil(a), FloatIndex{2});
    EXPECT_EQ(ceil(b), FloatIndex{4});
    EXPECT_EQ(ceil(n), FloatCount{-8});
    EXPECT_EQ(ceil(i), FloatIndex{5});

    EXPECT_EQ(round(a), FloatIndex{1});
    EXPECT_EQ(round(b), FloatIndex{4});
    EXPECT_EQ(round(n), FloatCount{-9});
    EXPECT_EQ(round(i), FloatIndex{5});

    EXPECT_EQ(round(n, FloatCount{10}), FloatCount{-10});
    EXPECT_EQ(round(p, FloatCount{10}), FloatCount{0});
    EXPECT_EQ(round(p, FloatCount{5}), FloatCount{5});
    EXPECT_EQ(round(i, FloatIndex{5}), FloatIndex{5});
}

TEST(MpkMix_Strong, FloatMathClamp)
{
    using namespace mpk::mix;

    auto lower = FloatCount{1};
    auto upper = FloatCount{10};
    auto x1 = FloatCount{0.5};
    auto x2 = FloatCount{5};
    auto x3 = FloatCount{123};
    auto N = [](auto v) { return ClampedResult{v, ClampType::None}; };
    auto L = [](auto v) { return ClampedResult{v, ClampType::Lower}; };
    auto U = [](auto v) { return ClampedResult{v, ClampType::Upper}; };
    EXPECT_EQ(clamped(x1, lower, upper), L(lower));
    EXPECT_EQ(clamped(lower, lower, upper), N(lower));
    EXPECT_EQ(clamped(x2, lower, upper), N(x2));
    EXPECT_EQ(clamped(upper, lower, upper), N(upper));
    EXPECT_EQ(clamped(x3, lower, upper), U(upper));

    auto clamp_result = clamp(x1, lower, upper);
    EXPECT_EQ(x1, lower);
    EXPECT_EQ(clamp_result, ClampType::Lower);

    clamp_result = clamp(x1, lower, upper);
    EXPECT_EQ(x1, lower);
    EXPECT_EQ(clamp_result, ClampType::None);

    clamp_result = clamp(x2, lower, upper);
    EXPECT_EQ(x2, FloatCount{5});
    EXPECT_EQ(clamp_result, ClampType::None);

    clamp_result = clamp(x3, lower, upper);
    EXPECT_EQ(x3, upper);
    EXPECT_EQ(clamp_result, ClampType::Upper);

    clamp_result = clamp(x3, lower, upper);
    EXPECT_EQ(x3, upper);
    EXPECT_EQ(clamp_result, ClampType::None);
}

TEST(MpkMix_Strong, Ranges)
{
    auto i1 = Index{5};
    auto i2 = Index{10};
    auto c = Count{10};

    auto test_range =
        []<typename R, typename I>(
            R range, mpk::mix::Type_Tag<I>, size_t begin, size_t size)
    {
        std::ignore = begin;
        auto has_first = false;
        auto iter_count = size_t{};
        I first;
        for (auto i: range)
        {
            static_assert(std::same_as<I, decltype(i)>);
            if (!has_first)
            {
                first = i;
                has_first = true;
            }
            EXPECT_EQ(i, first + typename R::Count(iter_count));
            ++iter_count;
        }
        EXPECT_EQ(typename R::Count(iter_count), range.size());
        EXPECT_EQ(iter_count, size);
    };

    constexpr auto index_tag = mpk::mix::Type<Index>;
    constexpr auto int_tag = mpk::mix::Type<int>;
    test_range(mpk::mix::index_range(i1, i2), index_tag, i1.v, (i2 - i1).v);
    test_range(mpk::mix::index_range(i1, c), index_tag, i1.v, c.v);
    test_range(mpk::mix::sized_index_range(i1, c), index_tag, i1.v, c.v);
    test_range(mpk::mix::index_range<Index>(c), index_tag, 0, c.v);
    test_range(mpk::mix::index_range(5, 10), int_tag, 5, 10 - 5);
    test_range(mpk::mix::sized_index_range(5, 10), int_tag, 5, 10);
    test_range(mpk::mix::index_range(10), int_tag, 0, 10);
}

TEST(MpkMix_Strong, Vector)
{
    using V = mpk::mix::StrongVector<std::string, Index>;

    V v(Count(3));
    v[mpk::mix::Zero] = "one";
    v.at(Index{1}) = "two";
    v.at(Index{2}) = "three";
    v.reserve(Count{11});
    EXPECT_EQ(v.capacity(), Count{11});
    v.resize(Count{6}, "many");
    v.resize(Count{10});
    v.back() = "a lot";
    v.push_back("a lot + 1");
    EXPECT_EQ(v.size(), Count{11});

    auto expected = std::initializer_list<std::string_view>{
        "one"sv,
        "two"sv,
        "three"sv,
        "many"sv,
        "many"sv,
        "many"sv,
        ""sv,
        ""sv,
        ""sv,
        "a lot"sv,
        "a lot + 1"sv};
    auto expected_it = expected.begin();
    for (auto const& s: v)
    {
        EXPECT_EQ(s, *expected_it);
        ++expected_it;
    }

    expected_it = expected.begin();
    for (auto i: v.index_range())
    {
        EXPECT_EQ(v[i], *expected_it);
        EXPECT_EQ(v.at(i), *expected_it);
        ++expected_it;
    }

    v.pop_back();
    EXPECT_EQ(v.size(), Count{10});

    EXPECT_THROW(v.at(Index{11}), std::out_of_range);

    auto v1 = v;
    EXPECT_EQ(v1, v);
    EXPECT_FALSE(v1 < v);
    EXPECT_FALSE(v1 != v);
    EXPECT_FALSE(v1 > v);
    EXPECT_TRUE(v1 <= v);
    EXPECT_TRUE(v1 >= v);
}

TEST(MpkMix_Strong, Span)
{
    auto check_span = []<typename V, typename I, std::size_t E>(
                          mpk::mix::StrongSpan<V, I, E> span,
                          std::initializer_list<V> contents)
    {
        using C = typename I::StrongDiff;
        EXPECT_EQ(span.size(), C(contents.size()));
        EXPECT_EQ(span.empty(), span.size() == mpk::mix::Zero);
        auto it = contents.begin();
        for (I i: span.index_range())
        {
            EXPECT_EQ(span[i], *it);
            ++it;
        }

        it = contents.begin();
        for (auto const& v: span)
            EXPECT_EQ(v, *it++);
    };

    {
        using V = mpk::mix::StrongVector<std::string, Index>;
        using W = V::Weak;
        auto v = V{W{"one", "two", "three", "four"}};
        using S = mpk::mix::StrongSpan<std::string, Index>;

        auto s = S{v};
        check_span(s, {"one"s, "two"s, "three"s, "four"s});
        check_span(s.subspan(Count{2}), {"one"s, "two"s});
        check_span(s.subspan(Index{2}), {"three"s, "four"s});
        check_span(s.subspan(Index{1}, Count{2}), {"two"s, "three"s});
        check_span(s.subspan(Index{1}, Index{2}), {"two"s});
        check_span(s.subspan(Index{1}, Index{1}), {});
    }

    {
        int n3[] = {123, 45, 67};
        using S3 = mpk::mix::StrongSpan<int, Index, 3>;
        using S = mpk::mix::StrongSpan<int, Index>;

        auto s3 = S3{n3};
        auto s = S{n3};

        check_span(s3, {123, 45, 67});
        check_span(s, {123, 45, 67});

        check_span(s3.subspan(Index{1}), {45, 67});
        check_span(s.subspan(Index{1}), {45, 67});
    }
}

TEST(MpkMix_Strong, String)
{
    static_assert(std::same_as<StrView, Str::View>);

    auto str_view = StrView{"asd"};
    auto str = Str{"qwe"};

    auto str_from_v = Str{str_view};
    static_assert(std::same_as<decltype(str_from_v), Str>);

    auto v_from_str = str_from_v.view();
    ASSERT_EQ(v_from_str, str_view);
    static_assert(std::same_as<decltype(v_from_str), StrView>);

    static_assert(mpk::mix::StrongType<Str>);
    static_assert(mpk::mix::StrongStringType<Str>);

    static_assert(mpk::mix::StrongType<StrView>);
    static_assert(!mpk::mix::StrongStringType<StrView>);

    [[maybe_unused]] auto s_v = str <=> v_from_str;
    [[maybe_unused]] auto v_s = v_from_str <=> str;
    [[maybe_unused]] auto s_eq_v = str == v_from_str;
    [[maybe_unused]] auto v_eq_s = v_from_str == str;
    [[maybe_unused]] auto s_neq_v = str != v_from_str;
    [[maybe_unused]] auto v_neq_s = v_from_str != str;

    EXPECT_FALSE(v_from_str == str);
    EXPECT_FALSE(str == v_from_str);
    EXPECT_TRUE(v_from_str == str_view);
}

TEST(MpkMix_Strong, ImplicitView)
{
    auto f = [](Str::View actual, std::string_view expected)
    { EXPECT_EQ(actual.v, expected); };

    auto str = Str{"Hello"};
    auto expected = str.view().v;
    f(str, expected);
    f(Str{str}, expected);
    f(str.view(), expected);
}


// --- Grouped / StrongGrouped ---

MPKMIX_STRONG_TYPE(GNodeCount, uint32_t, mpk::mix::StrongCountFeatures);
MPKMIX_STRONG_TYPE(GNodeIndex, uint32_t, mpk::mix::StrongIndexFeatures<GNodeCount>);
MPKMIX_STRONG_TYPE(GEdgeCount, uint32_t, mpk::mix::StrongCountFeatures);
MPKMIX_STRONG_TYPE(GEdgeIndex, uint32_t, mpk::mix::StrongIndexFeatures<GEdgeCount>);

TEST(MpkMix_Strong, Grouped_Basic)
{
    mpk::mix::Grouped<int> g;
    add_to_last_group(g, 10);
    add_to_last_group(g, 20);
    next_group(g);
    add_to_last_group(g, 30);
    next_group(g);

    EXPECT_EQ(group_count(g), 2u);
    auto g0 = group(g, 0);
    EXPECT_EQ(g0.size(), 2u);
    EXPECT_EQ(g0[0], 10);
    EXPECT_EQ(g0[1], 20);
    auto g1 = group(g, 1);
    EXPECT_EQ(g1.size(), 1u);
    EXPECT_EQ(g1[0], 30);
}

TEST(MpkMix_Strong, Grouped_Format)
{
    mpk::mix::Grouped<int> g;
    add_to_last_group(g, 1);
    add_to_last_group(g, 2);
    next_group(g);
    add_to_last_group(g, 3);
    next_group(g);

    std::ostringstream s;
    s << g;
    EXPECT_EQ(s.str(), "[(1,2), (3)]");
}

TEST(MpkMix_Strong, StrongGrouped_Basic)
{
    using SG = mpk::mix::StrongGrouped<int, GNodeIndex, GEdgeIndex>;
    SG sg;
    add_to_last_group(sg, 10);
    add_to_last_group(sg, 20);
    next_group(sg);
    add_to_last_group(sg, 30);
    next_group(sg);

    EXPECT_EQ(group_count(sg), GNodeCount{2});
    auto g0 = group(sg, GNodeIndex{0});
    EXPECT_EQ(g0.size(), GEdgeCount{2});
    EXPECT_EQ(g0[GEdgeIndex{0}], 10);
    EXPECT_EQ(g0[GEdgeIndex{1}], 20);
}
