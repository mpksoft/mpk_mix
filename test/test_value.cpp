/** @file
 * @brief Tests for mpk::mix::value::{Type, Value, ValuePath}.
 *
 * Copyright (C) 2024-2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/struct_type_macro.hpp"
#include "mpk/mix/value/value.hpp"

#include <gtest/gtest.h>

#include <format>
#include <initializer_list>


using namespace std::literals;
namespace mv = mpk::mix::value;

namespace {

struct MyStruct
{
    int foo{};
    double bar{};
    std::vector<unsigned int> flags;
};

MPKMIX_STRUCT_TYPE(MyStruct, foo, bar, flags);

MPKMIX_STRONG_TYPE(MyIndex, uint32_t);

} // anonymous namespace


TEST(MpkMix_Value, ActuallyEqualityComparable)
{
    using Vs = std::vector<MyStruct>;
    using Vi = std::vector<int>;
    static_assert(!mv::detail::actually_equality_comparable<MyStruct>);
    static_assert(!mv::detail::actually_equality_comparable<Vs>);
    static_assert(mv::detail::actually_equality_comparable<Vi>);
    static_assert(mv::detail::actually_equality_comparable<int>);
}

TEST(MpkMix_Value, Type)
{
    const auto* t_int = mv::Type::of<int>();
    const auto* t_int_vec = mv::Type::of<std::vector<int>>();
    const auto* t_bool = mv::Type::of<bool>();
    const auto* t_int_vec_vec = mv::Type::of<std::vector<std::vector<int>>>();
    const auto* t_tuple = mv::Type::of<std::tuple<int, bool, std::vector<float>>>();
    const auto* t_struct = mv::Type::of<MyStruct>();
    const auto* t_path = mv::Type::of<mv::ValuePath>();
    const auto* t_struct_arr_3 = mv::Type::of<std::array<MyStruct, 3>>();

    EXPECT_EQ(mv::Type::of<std::vector<int>>(), t_int_vec);
    EXPECT_EQ(mv::Type::of<MyStruct>(), t_struct);

    EXPECT_EQ(std::format("{}", t_int),
              "Type{I32}");
    EXPECT_EQ(std::format("{}", t_int_vec),
              "Type{Vector[I32]}");
    EXPECT_EQ(std::format("{}", t_bool),
              "Type{Bool}");
    EXPECT_EQ(std::format("{}", t_int_vec_vec),
              "Type{Vector[Vector[I32]]}");
    EXPECT_EQ(std::format("{}", t_tuple),
              "Type{Tuple{I32, Bool, Vector[F32]}}");
    EXPECT_EQ(std::format("{}", t_struct),
              "Type{Struct{foo: I32, bar: F64, flags: Vector[U32]}}");
    EXPECT_EQ(std::format("{}", t_path),
              "Type{Path}");
    EXPECT_EQ(std::format("{}", t_struct_arr_3),
              "Type{Array<3>[Struct{foo: I32, bar: F64, flags: Vector[U32]}]}");
}

TEST(MpkMix_Value, Array)
{
    using A = std::array<int, 3>;

    const auto* t_int_arr_3 = mv::Type::of<A>();
    EXPECT_EQ(std::format("{}", t_int_arr_3), "Type{Array<3>[I32]}");

    auto a = A{1, 4, 9};
    auto v = mv::Value{a};
    EXPECT_EQ(v.get(mv::ValuePath{0}), 1);
    EXPECT_EQ(v.get(mv::ValuePath{1}), 4);
    EXPECT_EQ(v.get(mv::ValuePath{2}), 9);

    EXPECT_EQ(std::format("{}", v), "[1,4,9]");
    EXPECT_EQ(v.size(), 3);
    EXPECT_THROW(v.resize(34), std::invalid_argument);
    EXPECT_EQ(v.size(), 3);

    v.set(mv::ValuePath{2}, 49);
    EXPECT_EQ(v.get(mv::ValuePath{2}), 49);
    EXPECT_EQ(std::format("{}", v), "[1,4,49]");
    EXPECT_THROW(v.set(mv::ValuePath{4}, 11), std::out_of_range);
}


enum class MyEnum : uint8_t
{
    Foo,
    Bar = 5,
    Baz
};

MPKMIX_VALUE_REGISTER_ENUM_TYPE(MyEnum, 1);

TEST(MpkMix_Value, EnumType)
{
    using Underlying = std::underlying_type_t<MyEnum>;
    using UnderlyingVec = std::vector<Underlying>;
    using StringViewVec = std::vector<std::string_view>;

    const auto* t_my_enum = mv::Type::of<MyEnum>();
    EXPECT_EQ(std::format("{}", t_my_enum), "Type{Enum<MyEnum: 1>}");

    auto v0 = mv::Value::make(t_my_enum);

    auto names =
        v0.get(mv::ValuePath{ "names"sv }).as<StringViewVec>();
    auto expected_names = StringViewVec{ "Foo"sv, "Bar"sv, "Baz"sv };
    EXPECT_EQ(names, expected_names);

    auto values =
        v0.get(mv::ValuePath{ "values"sv }).as<UnderlyingVec>();
    auto expected_values = UnderlyingVec{ 0, 5, 6 };
    EXPECT_EQ(values, expected_values);

    auto v = mv::Value{MyEnum::Baz};

    auto e = v.as<MyEnum>();
    EXPECT_EQ(e, MyEnum::Baz);

    v.set({}, MyEnum::Bar);
    EXPECT_EQ(v, MyEnum::Bar);

    auto vi = v.get(mv::ValuePath{ "index"sv }).as<size_t>();
    EXPECT_EQ(vi, magic_enum::enum_index(MyEnum::Bar));

    auto vv = v.get(mv::ValuePath{ "value"sv }).as<Underlying>();
    EXPECT_EQ(vv, magic_enum::enum_integer(MyEnum::Bar));

    auto vn = v.get(mv::ValuePath{ "name"sv }).as<std::string_view>();
    EXPECT_EQ(vn, "Bar"sv);

    v.set(mv::ValuePath{ "index"sv }, size_t{1});
    EXPECT_EQ(std::format("{}", v), "Bar");

    v.set(mv::ValuePath{ "name"sv }, "Foo"s);
    EXPECT_EQ(v.as<MyEnum>(), MyEnum::Foo);

    v.set(mv::ValuePath{ "value"sv },
          mv::Value{ mpk::mix::Type<Underlying>, 6 });
    EXPECT_EQ(v.as<MyEnum>(), MyEnum::Baz);
}

TEST(MpkMix_Value, EnumFlagsType)
{
    using MyFlags = mpk::mix::EnumFlags<MyEnum>;
    const auto* type = mv::Type::of<MyFlags>();
    EXPECT_EQ(std::format("{}", type), "Type{EnumFlags{Enum<MyEnum: 1>}}");

    auto flags = MyFlags{MyEnum::Foo};
    auto v = mv::Value{flags};

    EXPECT_EQ(v.as<MyFlags>(), flags);
    EXPECT_EQ(v.size(), 1);
    EXPECT_TRUE(v.contains(MyEnum::Foo));
    EXPECT_FALSE(v.contains(MyEnum::Bar));
    EXPECT_EQ(std::format("{}", v), std::format("{}", flags));

    flags |= MyEnum::Baz;
    v.insert(MyEnum::Baz);
    EXPECT_EQ(v.as<MyFlags>(), flags);
    EXPECT_EQ(v.size(), 2);

    {
        auto keys = v.keys();
        ASSERT_EQ(keys.size(), 2);
        EXPECT_EQ(keys[0].as<MyEnum>(), MyEnum::Foo);
        EXPECT_EQ(keys[1].as<MyEnum>(), MyEnum::Baz);
    }

    flags &= ~MyFlags{MyEnum::Foo};
    v.remove(MyEnum::Foo);
    EXPECT_EQ(v.as<MyFlags>(), flags);
    EXPECT_EQ(std::format("{}", flags), "{Baz}");

    v.set_default();
    EXPECT_EQ(v.size(), 0);
    EXPECT_FALSE(v.contains(MyEnum::Foo));
}

class MyBlob final
{};

MPKMIX_VALUE_REGISTER_CUSTOM_TYPE(MyBlob, 1);

TEST(MpkMix_Value, CustomType)
{
    const auto* t_my_blob = mv::Type::of<MyBlob>();
    EXPECT_EQ(std::format("{}", t_my_blob), "Type{Custom<MyBlob: 1>}");
}

TEST(MpkMix_Value, Scalar)
{
    auto val = mv::Value(mpk::mix::Type<int32_t>, 123);

    size_t visit_count{};
    mv::ScalarT{ val.type() }.visit(
        [&]<typename T>(mpk::mix::Type_Tag<T>)
        {
            constexpr auto is_int32_t = std::is_same_v<T, int32_t>;
            EXPECT_TRUE(is_int32_t);
            ++visit_count;
            if constexpr (std::is_integral_v<T>)
            {
                EXPECT_EQ(val.as<T>(), 123);
            }
        });
    EXPECT_EQ(visit_count, 1);

    auto& ival = val.as<int32_t>();
    EXPECT_EQ(ival, 123);
}

TEST(MpkMix_Value, String)
{
    auto check = [](const mv::Value& val,
                    auto tag,
                    std::string_view expected_val)
    {
        EXPECT_EQ(val.type(), mv::type_of(tag));
        EXPECT_EQ(val.as(tag), expected_val);

        auto as_s = val.convert_to<std::string>();
        static_assert(std::same_as<decltype(as_s), std::string>);
        EXPECT_EQ(as_s, expected_val);

        auto as_sv = val.convert_to<std::string_view>();
        static_assert(std::same_as<decltype(as_sv), std::string_view>);
        EXPECT_EQ(as_sv, expected_val);

        EXPECT_THROW(val.convert_to<int>(), std::invalid_argument);
    };

    constexpr auto ts = mpk::mix::Type<std::string>;
    constexpr auto tsv = mpk::mix::Type<std::string_view>;
    constexpr auto sv1 = "asd"sv;
    constexpr auto sv2 = "qwe"sv;

    auto v1_s_from_s = mv::Value(ts, std::string(sv1));
    auto v2_sv_from_sv = mv::Value(tsv, sv2);
    auto v2_s_from_sv = mv::Value(ts, sv2);

    check(v1_s_from_s, ts, sv1);
    check(v2_sv_from_sv, tsv, sv2);
    check(v2_s_from_sv, ts, sv2);
}

TEST(MpkMix_Value, DynamicValueAccess)
{
    auto v_int = mv::Value(123);
    EXPECT_EQ(v_int.get({}).as<int>(), 123);

    auto v_vec_double = mv::Value(std::vector<double>{ 1.2, 3.4, 5.6 });
    EXPECT_EQ(v_vec_double.get(mv::ValuePath{} / 1ul).as<double>(), 3.4);

    auto v_struct  = mv::Value(MyStruct{
        .foo = 123,
        .bar = 4.56,
        .flags = {12, 34, 56, 78, 90}
    });
    EXPECT_EQ(v_struct.get(mv::ValuePath{} / "foo"sv).as<int>(), 123);
    EXPECT_EQ(v_struct.get(mv::ValuePath{} / "bar"sv).as<double>(), 4.56);

    auto actual_flags =
        v_struct.get(mv::ValuePath{} / "flags"sv)
                .as<std::vector<unsigned int>>();
    auto expected_flags =
        std::vector<unsigned int>{12, 34, 56, 78, 90};
    EXPECT_EQ(actual_flags, expected_flags);

    v_struct.set(mv::ValuePath{} / "flags"sv / 3u, 912u);
    EXPECT_EQ(v_struct.get(mv::ValuePath{} / "flags"sv / 3u).as<unsigned>(), 912);

    v_struct.resize(mv::ValuePath{} / "flags"sv, 6);
    v_struct.set(mv::ValuePath{} / "flags"sv / 5u, 144u);
    EXPECT_EQ(v_struct.get(mv::ValuePath{} / "flags"sv / 5u).as<unsigned>(), 144);
}

TEST(MpkMix_Value, StrongType)
{
    static_assert(std::same_as<MyIndex::Weak, uint32_t>);

    const auto* type = mv::type_of<MyIndex>();
    EXPECT_EQ(std::format("{}", type), "Type{Strong{U32}}"sv);

    auto my_index = MyIndex{123};
    auto v = mv::Value{ my_index };
    EXPECT_EQ(v.as<MyIndex>(), my_index);

    const auto path = mv::ValuePath{} / "v"sv;
    EXPECT_EQ(v.get(path).as<MyIndex::Weak>(), my_index.v);

    v.set(path, MyIndex::Weak{456});
    EXPECT_EQ(v.get(path).as<MyIndex::Weak>(), 456);
    EXPECT_EQ(v.as<MyIndex>(), MyIndex{456});

    auto v1 = mv::Value::make(type);
    EXPECT_EQ(v1.as<MyIndex>(), MyIndex{0});
}

TEST(MpkMix_Value, FormatValue)
{
    EXPECT_EQ(std::format("{}", mv::Value(mpk::mix::Type<uint8_t>, 123)), "123");
    EXPECT_EQ(std::format("{}", mv::Value(mpk::mix::Type<int8_t>, 'A')), "65");
    EXPECT_EQ(std::format("{}", mv::Value(-1.23)), "-1.23");
    EXPECT_EQ(std::format("{}", mv::Value(std::byte{0x9c})), "9c");
    EXPECT_EQ(std::format("{}", mv::Value(true)), "true");
    EXPECT_EQ(std::format("{}", mv::Value(false)), "false");
    EXPECT_EQ(std::format("{}", mv::Value("Hello"sv)), "Hello");
    EXPECT_EQ(std::format("{}", mv::Value("World"s)), "World");
    EXPECT_EQ(std::format("{}", mv::Value(MyIndex{534})), "534");

    auto v = std::vector<int>{9,8,75};
    EXPECT_EQ(std::format("{}", mv::Value(v)), "[9,8,75]");

    auto t = std::make_tuple(1, 2.3, true, "hello"sv);
    EXPECT_EQ(std::format("{}", mv::Value(t)), "{1,2.3,true,hello}");

    auto s = MyStruct
    {
        .foo = 345,
        .bar = 1.3e11,
        .flags = {1, 3, 7, 13, 23}
    };
    EXPECT_EQ(std::format("{}", mv::Value(s)),
              "{foo=345,bar=1.3e+11,flags=[1,3,7,13,23]}");

    EXPECT_EQ(std::format("{}", mv::Value(MyBlob{})), "custom");
}

TEST(MpkMix_Value, ValueEquality)
{
    EXPECT_EQ(mv::Value{123}, mv::Value{123});
    EXPECT_NE(mv::Value{123}, mv::Value{123u});
    EXPECT_NE(mv::Value{123}, mv::Value{456});
    auto myval_1a = MyStruct{
        .foo = 1,
        .bar = 2.34,
        .flags = {4, 8}
    };
    auto myval_1b = myval_1a;
    auto myval_2 = MyStruct{
        .foo = 1,
        .bar = 2.34,
        .flags = {4, 8, 16}
    };
    EXPECT_EQ(mv::Value{myval_1a}, mv::Value{myval_1b});
    EXPECT_NE(mv::Value{myval_1a}, mv::Value{myval_2});
}

TEST(MpkMix_Value, ValuePath)
{
    auto p = mv::ValuePath{} / "foo"sv / 0u / "bar"sv;
    EXPECT_EQ(std::format("{}", p), "/foo/0/bar");

    auto p2 = mv::ValuePath::from_string("/a/1/b");
    EXPECT_EQ(p2.size(), 3);
    EXPECT_EQ(p2[0].name(), "a"sv);
    EXPECT_EQ(p2[1].index(), 1u);
    EXPECT_EQ(p2[2].name(), "b"sv);

    EXPECT_THROW(mv::ValuePath::from_string("no-leading-slash"),
                 std::invalid_argument);
}
