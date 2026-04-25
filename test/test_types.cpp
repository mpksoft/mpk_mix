/** @file
 * @brief TODO: Brief docstring.
 *
 * TODO: More documentation here
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/types/byte_span.hpp"
#include "mpk/mix/types/byte_buf.hpp"
#include "mpk/mix/types/capped_vec.hpp"
#include "mpk/mix/types/fixed_length_string.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <string>

using namespace mpk::mix;

// ---- CastableToBytes -------------------------------------------------------

TEST(MpkMix_CastableToBytes, TrivialTypes)
{
    static_assert(CastableToBytes<int>);
    static_assert(CastableToBytes<double>);
    static_assert(CastableToBytes<std::byte>);
    static_assert(!CastableToBytes<std::string>);
}

// ---- ByteSpan --------------------------------------------------------------

TEST(MpkMix_ByteSpan, FromArray)
{
    int arr[4] = {1, 2, 3, 4};
    auto s = byte_span(arr);
    EXPECT_EQ(s.size(), sizeof(arr));
    EXPECT_EQ(s.data(), reinterpret_cast<std::byte*>(arr));
}

TEST(MpkMix_ByteSpan, FromStdArray)
{
    std::array<int, 3> arr = {10, 20, 30};
    auto s = byte_span(arr);
    EXPECT_EQ(s.size(), sizeof(arr));
}

TEST(MpkMix_ByteSpan, FromConst)
{
    int const x = 42;
    auto s = byte_span(x);
    EXPECT_EQ(s.size(), sizeof(int));
}

TEST(MpkMix_ByteSpan, FromVoidPtr)
{
    int x = 7;
    auto s = byte_span(&x, sizeof(x));
    EXPECT_EQ(s.size(), sizeof(int));
}

TEST(MpkMix_ByteSpan, ByteOrConstByteConcept)
{
    static_assert(ByteOrConstByte<std::byte>);
    static_assert(ByteOrConstByte<std::byte const>);
    static_assert(!ByteOrConstByte<int>);
}

// ---- ByteBuf ---------------------------------------------------------------

TEST(MpkMix_ByteBuf, DefaultEmpty)
{
    ByteBuf buf;
    EXPECT_TRUE(buf.empty());
    EXPECT_EQ(buf.size(), 0u);
}

TEST(MpkMix_ByteBuf, AppendSpan)
{
    ByteBuf buf;
    std::array<std::byte, 4> data{
        std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}};
    buf.append(ConstByteSpan<4>{data});
    EXPECT_EQ(buf.size(), 4u);
    EXPECT_EQ(buf.data()[0], std::byte{1});
    EXPECT_EQ(buf.data()[3], std::byte{4});
}

TEST(MpkMix_ByteBuf, AppendSingleByte)
{
    ByteBuf buf;
    buf.append(std::byte{0xAB});
    EXPECT_EQ(buf.size(), 1u);
    EXPECT_EQ(buf.data()[0], std::byte{0xAB});
}

TEST(MpkMix_ByteBuf, AppendTypedInt)
{
    ByteBuf buf;
    buf.append(Type<uint16_t>, uint8_t{5});
    EXPECT_EQ(buf.size(), 2u);
}

TEST(MpkMix_ByteBuf, PrependAndDiscard)
{
    ByteBuf buf;
    buf.alloc(8);
    std::array<std::byte, 4> prefix{};
    buf.load(ConstByteSpan<4>{prefix}, 4); // begin=4, end=8, size=4
    buf.prepend(ConstByteSpan<4>{prefix}); // begin=0, end=8, size=8
    EXPECT_EQ(buf.size(), 8u);
    buf.discard_prefix(2); // begin=2, end=8, size=6
    EXPECT_EQ(buf.size(), 6u);
}

TEST(MpkMix_ByteBuf, Shrink)
{
    ByteBuf buf;
    std::array<std::byte, 4> data{};
    buf.load(ConstByteSpan<4>{data}, 4);
    buf.discard_prefix(2);
    buf.shrink();
    EXPECT_EQ(buf.offset(), 0u);
}

TEST(MpkMix_ByteBuf, ConversionOperators)
{
    ByteBuf buf;
    buf.append(std::byte{0xFF});
    ConstByteSpan<> cs = buf;
    EXPECT_EQ(cs.size(), 1u);
    ByteSpan<> ms = buf;
    EXPECT_EQ(ms.size(), 1u);
}

// ---- FixedLengthString -----------------------------------------------------

TEST(MpkMix_FixedLengthString, ConstructFromLiteral)
{
    FixedLengthString<16> s{"hello"};
    EXPECT_EQ(s.size(), 5u);
    EXPECT_EQ(s.view(), "hello");
}

TEST(MpkMix_FixedLengthString, ConstructFromStdString)
{
    std::string src = "world";
    FixedLengthString<16> s{src};
    EXPECT_EQ(s.size(), 5u);
    EXPECT_EQ(s.view(), "world");
}

TEST(MpkMix_FixedLengthString, TooLongThrows)
{
    using sv = std::string_view;
    EXPECT_THROW((FixedLengthString<3>{sv{"toolong"}}), std::invalid_argument);
}

TEST(MpkMix_FixedLengthString, StringMethod)
{
    FixedLengthString<8> s{"abc"};
    EXPECT_EQ(s.string(), std::string{"abc"});
}

TEST(MpkMix_FixedLengthString, Comparison)
{
    FixedLengthString<8> a{"abc"};
    FixedLengthString<8> b{"abd"};
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(a == "abc");
    EXPECT_TRUE(a != "xyz");
}

TEST(MpkMix_FixedLengthString, ConversionToStringView)
{
    FixedLengthString<8> s{"test"};
    std::string_view sv = s;
    EXPECT_EQ(sv, "test");
}

// ---- CappedVec -------------------------------------------------------------

TEST(MpkMix_CappedVec, PushPopSize)
{
    CappedVec<int, 4> v;
    EXPECT_EQ(v.size(), 0u);
    v.push_back(1);
    v.push_back(2);
    EXPECT_EQ(v.size(), 2u);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v.back(), 2);
    v.pop_back();
    EXPECT_EQ(v.size(), 1u);
}

TEST(MpkMix_CappedVec, CapacityExhaustedThrows)
{
    CappedVec<int, 2> v;
    v.push_back(1);
    v.push_back(2);
    EXPECT_THROW(v.push_back(3), std::range_error);
}

TEST(MpkMix_CappedVec, ResizeAndClear)
{
    CappedVec<int, 4> v;
    v.resize(3, 99);
    EXPECT_EQ(v.size(), 3u);
    EXPECT_EQ(v[2], 99);
    v.clear();
    EXPECT_EQ(v.size(), 0u);
}

TEST(MpkMix_CappedVec, AtOutOfRangeThrows)
{
    CappedVec<int, 4> v;
    EXPECT_THROW(v.at(0), std::range_error);
}

TEST(MpkMix_CappedVec, Iteration)
{
    CappedVec<int, 4> v;
    v.push_back(10);
    v.push_back(20);
    v.push_back(30);
    int sum = 0;
    for (auto x : v)
        sum += x;
    EXPECT_EQ(sum, 60);
}
