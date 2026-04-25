/** @file
 * @brief TODO: Brief docstring.
 *
 * TODO: More documentation here
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/util/binomial.hpp"
#include "mpk/mix/util/downcaster.hpp"
#include "mpk/mix/util/checkpoint.hpp"
#include "mpk/mix/util/detail/hash.hpp"
#include "mpk/mix/util/detail/read_write_bytes.hpp"
#include "mpk/mix/util/storable_type.hpp"
#include "mpk/mix/util/chrono_timepoint.hpp"
#include "mpk/mix/util/exception_message.hpp"
#include "mpk/mix/util/mutability.hpp"
#include "mpk/mix/util/optional.hpp"
#include "mpk/mix/util/resource_list.hpp"
#include "mpk/mix/util/subscribers.hpp"
#include "mpk/mix/util/to_underlying.hpp"
#include "mpk/mix/util/transform_optional.hpp"
#include "mpk/mix/util/const_name_span.hpp"
#include "mpk/mix/util/const_span.hpp"
#include "mpk/mix/util/cxx_type_name.hpp"
#include "mpk/mix/util/defer.hpp"
#include "mpk/mix/util/fast_pimpl.hpp"
#include "mpk/mix/util/format_enum.hpp"
#include "mpk/mix/util/format_dynamic.hpp"
#include "mpk/mix/util/format_sep.hpp"
#include "mpk/mix/util/format_seq.hpp"
#include "mpk/mix/util/format_streamable.hpp"
#include "mpk/mix/util/holder.hpp"
#include "mpk/mix/util/impl_tag.hpp"
#include "mpk/mix/util/index_set.hpp"
#include "mpk/mix/util/linked_list.hpp"
#include "mpk/mix/util/maybe_const.hpp"
#include "mpk/mix/util/overloads.hpp"
#include "mpk/mix/util/pow2.hpp"
#include "mpk/mix/util/ring_buffer.hpp"
#include "mpk/mix/util/scoped_inc.hpp"
#include "mpk/mix/util/throw.hpp"
#include "mpk/mix/util/tuple_like.hpp"
#include "mpk/mix/util/type_hash.hpp"
#include "mpk/mix/util/with_exceptions.hpp"
#include "mpk/mix/util/with_message.hpp"
#include "mpk/mix/strong/strong.hpp"

#include <gtest/gtest.h>

#include <array>
#include <chrono>
#include <cstdint>
#include <exception>
#include <optional>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>
#include <variant>

// -----------------------

TEST(MpkMix_Overloads, BasicVisit)
{
    using namespace mpk::mix;

    auto v = std::variant<int, std::string>{"hello"};

    auto result = std::visit(
        Overloads{
            [](int x) { return std::string("int:") + std::to_string(x); },
            [](std::string const& s) { return std::string("str:") + s; }},
        v);

    EXPECT_EQ(result, "str:hello");
}

TEST(MpkMix_Overloads, MultipleCallables)
{
    using namespace mpk::mix;

    auto dispatch = Overloads{
        [](int x) { return x * 2; },
        [](double x) { return static_cast<int>(x * 3); }};

    EXPECT_EQ(dispatch(5), 10);
    EXPECT_EQ(dispatch(2.0), 6);
}

// -----------------------

TEST(MpkMix_Defer, RunsOnDestruct)
{
    using namespace mpk::mix;

    int count = 0;
    {
        auto d = Defer{[&] { ++count; }};
    }
    EXPECT_EQ(count, 1);
}

TEST(MpkMix_Defer, CommitRunsImmediately)
{
    using namespace mpk::mix;

    int count = 0;
    {
        auto d = Defer{[&] { ++count; }};
        d.commit();
        EXPECT_EQ(count, 1);
    }
    // destructor should NOT run again
    EXPECT_EQ(count, 1);
}

TEST(MpkMix_Defer, NoRunAfterCommit)
{
    using namespace mpk::mix;

    int count = 0;
    {
        auto d = Defer{[&] { ++count; }};
        d.commit(); // count becomes 1, committed_ = true
        // destructor sees committed_ = true → no second call
    }
    EXPECT_EQ(count, 1);
}

// -----------------------

TEST(MpkMix_TupleLike, BasicTypes)
{
    using namespace mpk::mix;

    static_assert(tuple_like<std::tuple<int, float>>);
    static_assert(tuple_like<std::pair<int, double>>);
    static_assert(tuple_like<std::array<int, 3>>);
    static_assert(!tuple_like<int>);
    static_assert(!tuple_like<std::vector<int>>);
}

// -----------------------

TEST(MpkMix_ScopedInc, IncrementAndDecrement)
{
    using namespace mpk::mix;

    int n = 5;
    {
        auto s = ScopedInc{n};
        EXPECT_EQ(n, 6);
    }
    EXPECT_EQ(n, 5);
}

TEST(MpkMix_ScopedInc, Nested)
{
    using namespace mpk::mix;

    int n = 0;
    {
        auto s1 = ScopedInc{n};
        EXPECT_EQ(n, 1);
        {
            auto s2 = ScopedInc{n};
            EXPECT_EQ(n, 2);
        }
        EXPECT_EQ(n, 1);
    }
    EXPECT_EQ(n, 0);
}

// -----------------------

TEST(MpkMix_MaybeConst, Concept)
{
    using namespace mpk::mix;

    static_assert(MaybeConst<int, int>);
    static_assert(MaybeConst<const int, int>);
    static_assert(!MaybeConst<float, int>);
    static_assert(!MaybeConst<const float, int>);
}

TEST(MpkMix_MaybeConst, AsConstAs)
{
    using namespace mpk::mix;

    static_assert(std::same_as<AsConstAs<int, float>, float>);
    static_assert(std::same_as<AsConstAs<const int, float>, const float>);
}

// -----------------------

TEST(MpkMix_ImplTag, IsEmptyStruct)
{
    using namespace mpk::mix;

    // Impl is a constexpr inline variable of type Impl_Tag
    static_assert(std::same_as<decltype(Impl), const Impl_Tag>);
    static_assert(sizeof(Impl_Tag) == 1); // empty struct
}

// -----------------------

TEST(MpkMix_Throw, ThrowsWithFormattedMessage)
{
    using namespace mpk::mix;

    try {
        throw_("value={}, name={}", 42, "test");
        FAIL() << "Expected exception";
    } catch (const std::runtime_error& e) {
        EXPECT_STREQ(e.what(), "value=42, name=test");
    }
}

TEST(MpkMix_Throw, ThrowsCustomException)
{
    using namespace mpk::mix;

    try {
        throw_<std::invalid_argument>("bad arg: {}", 7);
        FAIL() << "Expected exception";
    } catch (const std::invalid_argument& e) {
        EXPECT_STREQ(e.what(), "bad arg: 7");
    }
}

TEST(MpkMix_Throw, ExceptionPtrHoldsFormattedMessage)
{
    using namespace mpk::mix;

    auto eptr = exception_ptr("value={}", 99);
    ASSERT_TRUE(eptr);
    try {
        std::rethrow_exception(eptr);
    } catch (const std::runtime_error& e) {
        EXPECT_STREQ(e.what(), "value=99");
    }
}

// -----------------------

MPKMIX_STRONG_TYPE(TestId, int);
MPKMIX_STRONG_TYPE(TestCount, int, mpk::mix::StrongCountFeatures);

TEST(MpkMix_StrongFormatter, FormatsUnderlyingValue)
{
    auto id = TestId{42};
    EXPECT_EQ(std::format("{}", id), "42");
}

TEST(MpkMix_StrongFormatter, FormatsCountValue)
{
    auto count = TestCount{7};
    EXPECT_EQ(std::format("{}", count), "7");
}

// -----------------------

TEST(MpkMix_FormatSeq, EmptySequence)
{
    using namespace mpk::mix;
    std::vector<int> v{};
    EXPECT_EQ(format_seq(v), "");
}

TEST(MpkMix_FormatSeq, BasicInts)
{
    using namespace mpk::mix;
    std::vector<int> v{1, 2, 3};
    EXPECT_EQ(format_seq(v, ","), "1,2,3");
}

TEST(MpkMix_FormatSeq, CustomDelimiter)
{
    using namespace mpk::mix;
    std::vector<std::string> v{"a", "b", "c"};
    EXPECT_EQ(format_seq(v, " | "), "a | b | c");
}

TEST(MpkMix_FormatSeq, HeadTail)
{
    using namespace mpk::mix;
    std::vector<int> v{1, 2, 3};
    EXPECT_EQ(format_seq(v, ",", DefaultFormatter{}, "[", "]"), "[1,2,3]");
}

// -----------------------

TEST(MpkMix_FormatSep, TwoArgs)
{
    using namespace mpk::mix;
    EXPECT_EQ(format_sep(" ", 1, "hello"), "1 hello");
}

TEST(MpkMix_FormatSep, ThreeArgs)
{
    using namespace mpk::mix;
    EXPECT_EQ(format_sep(", ", "a", "b", "c"), "a, b, c");
}

TEST(MpkMix_FormatSep, EmptySep)
{
    using namespace mpk::mix;
    EXPECT_EQ(format_sep("", 1, 2, 3), "123");
}

// -----------------------

TEST(MpkMix_FormatDynamic, BasicFormat)
{
    using namespace mpk::mix;
    EXPECT_EQ(dynamic_format("{} + {} = {}", 1, 2, 3), "1 + 2 = 3");
}

TEST(MpkMix_FormatDynamic, RuntimeFormatString)
{
    using namespace mpk::mix;
    auto fmt = std::string{"{}"};
    EXPECT_EQ(dynamic_format(fmt, 42), "42");
}

// -----------------------

struct MyStreamable
{
    int value;
    friend auto operator<<(std::ostream& s, const MyStreamable& v) -> std::ostream&
    { return s << "MyStreamable(" << v.value << ")"; }
};

MPKMIX_DECL_OSTREAM_FORMATTER(MyStreamable);

TEST(MpkMix_OstreamFormatter, FormatsViaOperatorStream)
{
    EXPECT_EQ(std::format("{}", MyStreamable{42}), "MyStreamable(42)");
}

// -----------------------

TEST(MpkMix_Pow2, Floor)
{
    static_assert(mpk::mix::floor2(0u) == 0);
    static_assert(mpk::mix::floor2(1u) == 1);
    static_assert(mpk::mix::floor2(64u) == 64);
    static_assert(mpk::mix::floor2(100u) == 64);
}

TEST(MpkMix_Pow2, Ceil)
{
    static_assert(mpk::mix::ceil2(0u) == 0);
    static_assert(mpk::mix::ceil2(1u) == 1);
    static_assert(mpk::mix::ceil2(64u) == 64);
    static_assert(mpk::mix::ceil2(100u) == 128);
}

TEST(MpkMix_Pow2, TypeTagVariants)
{
    using mpk::mix::Type;
    static_assert(mpk::mix::floor2(Type<unsigned>, 100u) == 64u);
    static_assert(mpk::mix::ceil2(Type<unsigned>, 100u) == 128u);
}

// -----------------------

TEST(MpkMix_Binomial, Basic)
{
    using T = uint64_t;
    using mpk::mix::Type;

    auto check = [](T n, T k, T expected)
    { EXPECT_EQ(mpk::mix::binomial(Type<T>, n, k), expected); };

    check(0, 0, 1);

    check(1, 0, 1);
    check(1, 1, 1);

    check(2, 0, 1);
    check(2, 1, 2);
    check(2, 2, 1);

    check(3, 0, 1);
    check(3, 1, 3);
    check(3, 2, 3);
    check(3, 3, 1);

    check(4, 0, 1);
    check(4, 1, 4);
    check(4, 2, 6);
    check(4, 3, 4);
    check(4, 4, 1);

    check(5, 0, 1);
    check(5, 1, 5);
    check(5, 2, 10);
    check(5, 3, 10);
    check(5, 4, 5);
    check(5, 5, 1);
}

// -----------------------

TEST(MpkMix_WithExceptions, VoidFuncNoException)
{
    auto eptr = mpk::mix::with_exceptions([] {});
    EXPECT_EQ(eptr, nullptr);
}

TEST(MpkMix_WithExceptions, VoidFuncThrows)
{
    auto eptr = mpk::mix::with_exceptions(
        [] { throw std::runtime_error("oops"); });
    ASSERT_NE(eptr, nullptr);
    EXPECT_THROW(std::rethrow_exception(eptr), std::runtime_error);
}

TEST(MpkMix_WithExceptions, NonVoidFuncNoException)
{
    auto [eptr, value] = mpk::mix::with_exceptions([] { return 42; });
    EXPECT_EQ(eptr, nullptr);
    EXPECT_EQ(value, 42);
}

TEST(MpkMix_WithExceptions, NonVoidFuncThrows)
{
    auto [eptr, value] = mpk::mix::with_exceptions(
        []() -> int { throw std::runtime_error("bad"); });
    ASSERT_NE(eptr, nullptr);
    EXPECT_EQ(value, int{});
}

// -----------------------

TEST(MpkMix_WithMessage, CallsThrough)
{
    using Wm = mpk::mix::WithMessage<false, int, int>;
    auto fn = [](int x) { return x * 2; };
    auto wm = Wm(mpk::mix::FuncRef<int(int)>{&fn}, "double it");
    EXPECT_EQ(wm.func()(5), 10);
}

TEST(MpkMix_WithMessage, EmptyFunc)
{
    using Wm = mpk::mix::WithMessage<false, void>;
    auto wm = Wm{};
    EXPECT_TRUE(wm.empty());
}

TEST(MpkMix_WithMessage, MessageAccessor)
{
    using Wm = mpk::mix::WithMessage<false, void>;
    auto fn = [] {};
    auto wm = Wm(mpk::mix::FuncRef<void()>{&fn}, "my message");
    EXPECT_EQ(wm.message(), "my message");
}

// --- cxx_type_name ---

TEST(MpkMix_Util, CxxTypeName_Int)
{
    EXPECT_EQ(mpk::mix::cxx_type_name<int>(), "int");
}

TEST(MpkMix_Util, CxxTypeName_TypeInfo)
{
    EXPECT_EQ(mpk::mix::cxx_type_name(typeid(double)), "double");
}

// --- type_hash ---

TEST(MpkMix_Util, TypeHash_SameType)
{
    EXPECT_EQ(mpk::mix::type_hash<int>(), mpk::mix::type_hash<int>());
}

TEST(MpkMix_Util, TypeHash_DifferentTypes)
{
    EXPECT_NE(mpk::mix::type_hash<int>(), mpk::mix::type_hash<double>());
}

// --- format_enum ---

namespace {
enum class Color { Red, Green, Blue };
} // anonymous namespace

TEST(MpkMix_Util, FormatEnum)
{
    EXPECT_EQ(std::format("{}", Color::Green), "Green");
}

// --- holder ---

TEST(MpkMix_Util, ValueHolder)
{
    using H = mpk::mix::ValueHolder<int>;
    static_assert(mpk::mix::HolderType<H>);
    auto h = H{42};
    EXPECT_EQ(h.value(), 42);
    h.value() = 99;
    EXPECT_EQ(h.value(), 99);
}

TEST(MpkMix_Util, BaseOf)
{
    struct X : mpk::mix::BaseOf<X> { int v{}; };
    static_assert(mpk::mix::HolderType<mpk::mix::BaseOf<X>>);
    auto x = X{ .v = 7 };
    EXPECT_EQ(x.value().v, 7);
}

// --- ring_buffer ---

TEST(MpkMix_Util, RingBuffer_Basic)
{
    mpk::mix::RingBuffer<int> rb(3);
    rb.push_back(1);
    rb.push_back(2);
    rb.push_back(3);
    rb.push_back(4); // overwrites 1
    EXPECT_EQ(rb.size(), 3);
    EXPECT_EQ(rb.front(), 2);
    EXPECT_EQ(rb.back(), 4);
}

TEST(MpkMix_Util, RingBuffer_Iterator)
{
    mpk::mix::RingBuffer<int> rb(4);
    for (int i = 0; i < 6; ++i) rb.push_back(i); // [2,3,4,5]
    auto vals = std::vector<int>(rb.begin(), rb.end());
    EXPECT_EQ(vals, (std::vector<int>{2, 3, 4, 5}));
}

// --- fast_pimpl ---

TEST(MpkMix_Util, FastPimpl_Basic)
{
    struct Impl { int x; };
    mpk::mix::FastPimpl<Impl, sizeof(Impl), alignof(Impl)> fp;
    EXPECT_FALSE(fp.has_value());
    fp.emplace(Impl{42});
    EXPECT_TRUE(fp.has_value());
    EXPECT_EQ(fp->x, 42);
}

// --- const_span / const_name_span ---

namespace {
struct TagA {};
struct TagB {};
} // anonymous namespace

TEST(MpkMix_Util, ConstSpan)
{
    using namespace std::string_view_literals;
    auto sp = mpk::mix::const_name_span(
        mpk::mix::TypePack<TagA, TagB>,
        "alpha"sv, "beta"sv);
    ASSERT_EQ(sp.size(), 2);
    EXPECT_EQ(sp[0], "alpha");
    EXPECT_EQ(sp[1], "beta");
}

// --- linked_list ---

TEST(MpkMix_Util, ValueLinkedList)
{
    mpk::mix::ValueLinkedList<int> list;
    auto i1 = mpk::mix::ValueLinkedListItem<int>{1};
    auto i2 = mpk::mix::ValueLinkedListItem<int>{2};
    auto i3 = mpk::mix::ValueLinkedListItem<int>{3};
    list.link(i1);
    list.link(i2);
    list.link(i3);
    auto vals = std::vector<int>{};
    for (const auto& item : list)
        vals.push_back(item.value());
    EXPECT_EQ(vals, (std::vector<int>{1, 2, 3}));
    i2.unlink();
    vals.clear();
    for (const auto& item : list)
        vals.push_back(item.value());
    EXPECT_EQ(vals, (std::vector<int>{1, 3}));
}

// --- checkpoint ---

namespace {
struct SimpleHistory {
    std::vector<int> events;
    void reset() { events.clear(); }
    void clear() { events.clear(); }
};
} // anonymous namespace

TEST(MpkMix_Util, Checkpoint_Basic)
{
    using CP = mpk::mix::Checkpoint<SimpleHistory>;
    mpk::mix::IntrusiveLinkedList<CP> list;
    auto cp1 = CP{};
    auto cp2 = CP{};
    list.link(cp1);
    list.link(cp2);
    cp1.update([](SimpleHistory& h) { h.events.push_back(1); });
    auto h1 = cp1.sync();
    EXPECT_EQ(h1.events, (std::vector<int>{1}));
    auto h1b = cp1.sync();
    EXPECT_TRUE(h1b.events.empty());
}

// --- index_set ---

MPKMIX_STRONG_TYPE(ISCount, uint8_t, mpk::mix::StrongCountFeatures);
MPKMIX_STRONG_TYPE(ISIndex, uint8_t, mpk::mix::StrongIndexFeatures<ISCount>);
MPKMIX_STRONG_LITERAL_SUFFIX(ISIndex, _ii);

TEST(MpkMix_Util, IndexSet_Weak)
{
    using S = mpk::mix::IndexSet<uint8_t, 8>;
    auto s = S::all();
    EXPECT_EQ(s.size(), 8);
    EXPECT_TRUE(s.contains(uint8_t{3}));
    s.toggle(3);
    EXPECT_FALSE(s.contains(uint8_t{3}));
    EXPECT_EQ(s.size(), 7);
}

TEST(MpkMix_Util, IndexSet_Strong)
{
    using S = mpk::mix::IndexSet<ISIndex, 8>;
    auto s = S{ 1_ii, 3_ii, 5_ii };
    EXPECT_EQ(s.size(), 3);
    EXPECT_TRUE(s.contains(3_ii));
    EXPECT_FALSE(s.contains(2_ii));
    s.clear(3_ii);
    EXPECT_EQ(s.size(), 2);
}

// --- exception_message ---

TEST(MpkMix_Util, ExceptionMessage_FromException)
{
    auto ep = std::make_exception_ptr(std::runtime_error("oops"));
    EXPECT_EQ(mpk::mix::exception_message(ep), "oops");
}

TEST(MpkMix_Util, ExceptionMessage_Empty)
{
    EXPECT_EQ(mpk::mix::exception_message(nullptr), "");
}

// --- transform_optional ---

TEST(MpkMix_Util, TransformOptional_WithFn)
{
    std::optional<int> v = 4;
    auto r = mpk::mix::transform_optional<std::string>(v, [](int x){ return std::to_string(x); });
    ASSERT_TRUE(r.has_value());
    EXPECT_EQ(*r, "4");
}

TEST(MpkMix_Util, TransformOptional_Empty)
{
    std::optional<int> v;
    auto r = mpk::mix::transform_optional<std::string>(v, [](int x){ return std::to_string(x); });
    EXPECT_FALSE(r.has_value());
}

// --- is_optional_v ---

TEST(MpkMix_Util, IsOptional)
{
    static_assert(mpk::mix::is_optional_v<std::optional<int>>);
    static_assert(!mpk::mix::is_optional_v<int>);
    SUCCEED();
}

// --- ChronoTimepointType ---

TEST(MpkMix_Util, ChronoTimepointType_Concept)
{
    using TP = std::chrono::system_clock::time_point;
    static_assert(mpk::mix::ChronoTimepointType<TP>);
    static_assert(!mpk::mix::ChronoTimepointType<int>);
    SUCCEED();
}

// --- mutability ---

TEST(MpkMix_Util, Mutability_Of)
{
    static_assert(std::same_as<mpk::mix::MutabilityOf<const int&>, mpk::mix::Constant>);
    static_assert(std::same_as<mpk::mix::MutabilityOf<int&>, mpk::mix::Mutable>);
    static_assert(std::same_as<mpk::mix::MutabilityAs<mpk::mix::Constant, int>, const int>);
    SUCCEED();
}

// --- to_underlying ---

TEST(MpkMix_Util, ToUnderlying_Arithmetic)
{
    EXPECT_EQ(mpk::mix::to_underlying(42), 42);
    EXPECT_EQ(mpk::mix::to_underlying(3.14), 3.14);
}

TEST(MpkMix_Util, ToUnderlying_Enum)
{
    enum class E : int { A = 1, B = 2 };
    EXPECT_EQ(mpk::mix::to_underlying(E::B), 2);
}

// --- resource_list ---

TEST(MpkMix_Util, ResourceList_Make)
{
    mpk::mix::ResourceList rl;
    auto* p = rl.make<int>(42);
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(*p, 42);
}

// --- subscribers ---

TEST(MpkMix_Util, Subscribers_Basic)
{
    mpk::mix::Subscribers<std::function<void(int)>> subs;
    int total = 0;
    subs.add([&](int x){ total += x; });
    subs.add([&](int x){ total += x * 2; });
    subs(3);
    EXPECT_EQ(total, 9);
}

// --- detail/hash ---

TEST(MpkMix_Util, Hash_Integral)
{
    EXPECT_EQ(mpk::mix::detail::hash(42), std::hash<int>{}(42));
}

TEST(MpkMix_Util, Hash_String)
{
    std::string s = "hello";
    EXPECT_EQ(mpk::mix::detail::hash(s), std::hash<std::string>{}(s));
}

TEST(MpkMix_Util, Hash_Pair)
{
    auto p = std::pair{1, 2};
    auto h = mpk::mix::detail::hash(p);
    EXPECT_NE(h, 0u);
}

TEST(MpkMix_Util, HashStruct_StringHash)
{
    mpk::mix::detail::StringHash sh;
    EXPECT_EQ(sh("hello"), std::hash<std::string_view>{}("hello"));
}

// --- storable_type ---

TEST(MpkMix_Util, StorableType_Concepts)
{
    static_assert(mpk::mix::PlainStorableType<int>);
    static_assert(!mpk::mix::PlainStorableType<std::string>);
    static_assert(mpk::mix::StorableType<int>);
    using Fixed3 = std::span<int, 3>;
    static_assert(mpk::mix::StaticExtentSpanOfPlainStorableType<Fixed3>);
    SUCCEED();
}

// --- detail/read_write_bytes ---

TEST(MpkMix_Util, ReadWriteBytes)
{
    int src = 0xDEADBEEF;
    int dst = 0;
    auto src_span = mpk::mix::byte_span(src);
    auto dst_span = mpk::mix::byte_span(dst);
    mpk::mix::detail::write_bytes(dst_span, src_span);
    EXPECT_EQ(dst, src);
}

// --- Downcaster ---

TEST(MpkMix_Util, Downcaster)
{
    struct Base
    {
        virtual ~Base() = default;
        int x = 0;
    };
    struct Derived : Base
    {
        int y = 42;
    };

    Derived d;
    Base* b = &d;

    auto down = mpk::mix::Downcaster<Derived, Base>{};
    EXPECT_EQ(down(b), &d);
    EXPECT_EQ(down(b)->y, 42);

    const Base* cb = &d;
    EXPECT_EQ(down(cb), &d);
}
