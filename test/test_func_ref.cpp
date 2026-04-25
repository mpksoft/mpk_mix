/** @file
 * @brief TODO: Brief docstring.
 *
 * TODO: More documentation here
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/func_ref.hpp"
#include "mpk/mix/signature.hpp"

#include <gtest/gtest.h>

#include <numeric>

// -----------------------

namespace {

auto add(int a, int b) -> int
{
    return a + b;
}

auto sum_vec(const std::vector<int>* vec) -> int
{
    return std::accumulate(vec->begin(), vec->end(), 0);
}

struct ClassWithAddMethod
{
    auto add(int a, int b) -> int
    {
        return a + b;
    }
};

struct AddFunctor
{
    auto operator()(int a, int b) -> int
    {
        return a + b;
    }
};

} // anonymous namespace

// -----------------------

TEST(MpkMix_FuncRef, FreeFunc)
{
    using namespace mpk::mix;

    auto iref = FuncRef{Const<add>};
    EXPECT_EQ(iref(3, 4), 7);

    auto iref_i = FuncRef{Indirect, &add};
    EXPECT_EQ(iref_i(3, 4), 7);
}

TEST(MpkMix_FuncRef, FreeFuncPtr)
{
    using namespace mpk::mix;

    auto iref = FuncRef{Indirect, add};
    EXPECT_EQ(iref(3, 4), 7);
}

TEST(MpkMix_FuncRef, FreeFuncWithContext)
{
    using namespace mpk::mix;

    auto v = std::vector{1, 2, 3};
    auto iref = FuncRef{Const<sum_vec>, &v};

    EXPECT_EQ(iref(), 6);
}

TEST(MpkMix_FuncRef, ClassMethod)
{
    using namespace mpk::mix;

    auto summator = ClassWithAddMethod{};
    auto iref = FuncRef{Const<&ClassWithAddMethod::add>, &summator};

    EXPECT_EQ(iref(12, 21), 33);
}

TEST(MpkMix_FuncRef, Functor)
{
    using namespace mpk::mix;

    auto summator = AddFunctor{};
    auto iref = FuncRef{&summator};
    EXPECT_EQ(iref(3, 4), 7);
}

TEST(MpkMix_FuncRef, CapturelessDecayedLambda)
{
    using namespace mpk::mix;

    auto decayed_lambda = +[](int a, int b) { return a + b; };

    auto iref = FuncRef{Indirect, decayed_lambda};
    EXPECT_EQ(iref(3, 4), 7);
}

TEST(MpkMix_FuncRef, CapturelessLambda)
{
    using namespace mpk::mix;

    auto lambda = [](int a, int b) { return a + b; };

    auto iref = FuncRef{&lambda};
    EXPECT_EQ(iref(3, 4), 7);
}

TEST(MpkMix_FuncRef, LambdaWithCapture)
{
    using namespace mpk::mix;

    auto five = 5;
    auto lambda = [five](int a) { return a + five; };

    auto iref = FuncRef{&lambda};
    EXPECT_EQ(iref(3), 8);
}

TEST(MpkMix_FuncRef, Empty)
{
    using namespace mpk::mix;

    auto iref = FuncRef{Const<add>};
    EXPECT_TRUE(iref);
    EXPECT_FALSE(iref.empty());

    FuncRef<int(int, int)> empty_iref;
    EXPECT_FALSE(empty_iref);
    EXPECT_TRUE(empty_iref.empty());
}

TEST(MpkMix_FuncRef, UnsafeConstruct)
{
    using namespace mpk::mix;

    // Round-trip through unsafe storage
    auto iref = FuncRef{Const<add>};
    auto stor = std::move(iref).storage();
    auto iref2 = FuncRef<int(int, int)>{Unsafe, stor.f, stor.c};
    EXPECT_EQ(iref2(10, 20), 30);
}

TEST(MpkMix_FuncRef, UntypedFuncRef_RoundTrip)
{
    using namespace mpk::mix;

    auto iref = FuncRef{Const<add>};
    UntypedFuncRef u = iref;
    EXPECT_TRUE(u);

    auto iref2 = u.typed(Unsafe, Type<int(int, int)>);
    EXPECT_EQ(iref2(5, 6), 11);
}

TEST(MpkMix_FuncRef, UntypedFuncRef_Empty)
{
    using namespace mpk::mix;

    UntypedFuncRef u;
    EXPECT_FALSE(u);
    EXPECT_TRUE(u.empty());
}

TEST(MpkMix_FuncRef, UntypedFuncRef_FromUntyped)
{
    using namespace mpk::mix;

    auto iref = FuncRef{Const<add>};
    auto u = iref.untyped();
    EXPECT_TRUE(u);
    EXPECT_EQ(u.typed(Unsafe, Type<int(int, int)>)(10, 32), 42);
}

// -----------------------

TEST(MpkMix_Signature, SignatureOf_FreeFunc)
{
    using namespace mpk::mix;

    static_assert(
        std::same_as<SignatureOf_t<decltype(&add), void>, int(int, int)>);
}

TEST(MpkMix_Signature, SignatureOf_Method)
{
    using namespace mpk::mix;

    static_assert(std::same_as<
                  SignatureOf_t<decltype(&ClassWithAddMethod::add),
                                ClassWithAddMethod>,
                  int(int, int)>);
}

TEST(MpkMix_Signature, MethodOf)
{
    using namespace mpk::mix;

    static_assert(MethodOf<decltype(&ClassWithAddMethod::add), ClassWithAddMethod>);
    static_assert(!MethodOf<decltype(&add), void>);
}

TEST(MpkMix_Signature, MethodWithSignatureOf)
{
    using namespace mpk::mix;

    static_assert(MethodWithSignatureOf<
                  decltype(&ClassWithAddMethod::add),
                  ClassWithAddMethod,
                  int(int, int)>);
}

TEST(MpkMix_Signature, SignatureFreeFunc)
{
    using namespace mpk::mix;

    constexpr auto sig = signature(&add);
    static_assert(std::same_as<decltype(sig)::type, int(int, int)>);
}

TEST(MpkMix_Signature, ReturnType)
{
    using namespace mpk::mix;

    static_assert(std::same_as<ReturnType_t<int(int, int)>, int>);
    static_assert(std::same_as<ReturnType_t<void(float)>, void>);
}

TEST(MpkMix_Signature, Arity)
{
    using namespace mpk::mix;

    static_assert(arity(mpk::mix::Type<int(int, int)>) == 2u);
    static_assert(arity(mpk::mix::Type<void()>) == 0u);
    static_assert(arity(&add) == 2u);
}

TEST(MpkMix_Signature, Arg0Type)
{
    using namespace mpk::mix;

    static_assert(
        std::same_as<decltype(arg_0_type(mpk::mix::Type<int(float, double)>))::type,
                     float>);
}
