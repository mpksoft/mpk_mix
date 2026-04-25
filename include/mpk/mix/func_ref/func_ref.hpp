/** @file
 * @brief FuncRef<S> non-owning type-erased callable reference
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/func_ref/fwd.hpp"
#include "mpk/mix/func_ref/tags.hpp"
#include "mpk/mix/signature.hpp"

#include <utility>

namespace mpk::mix
{

template <typename R, typename... A, bool NX>
class FuncRef<R(A...) noexcept(NX)> final
{
public:
    using S = R(A...) noexcept(NX);

    template <typename Context>
    using SX = R(Context*, A...) noexcept(NX);

    struct Storage final
    {
        SX<void>* f{};
        void* c{};
    };

    FuncRef() = default;

    // ---

    template <typename Context>
    FuncRef(SX<Context>* f, Context* c) :
      storage_{
          .f{reinterpret_cast<SX<void>*>(f)},
          .c{const_cast<void*>(reinterpret_cast<void const*>(c))}}
    {
    }

    // ---

    template <S* f>
    /* implicit */ FuncRef(Const_Tag<f>) :
      storage_{
          .f{+[](void*, A... args) noexcept(NX) -> R
             { return f(std::forward<A>(args)...); }},
          .c{nullptr}}
    {
    }

    template <
        typename AcceptedContext,
        SX<AcceptedContext>* f,
        typename PassedContext>
        requires std::convertible_to<PassedContext*, AcceptedContext*>
    FuncRef(Const_Tag<f>, PassedContext* ctx) :
      FuncRef{
          +[](AcceptedContext* c, A... args) noexcept(NX) -> R
          { return f(c, std::forward<A>(args)...); },
          static_cast<AcceptedContext*>(ctx)}
    {
    }

    template <typename Cls, auto f>
        requires MethodWithSignatureOf<decltype(f), Cls, S>
    FuncRef(Const_Tag<f>, Cls* cls) :
      FuncRef{
          +[](Cls* cls, A... args) noexcept(NX) -> R
          { return (cls->*f)(std::forward<A>(args)...); },
          cls}
    {
    }

    // ---

    FuncRef(Indirect_Tag, S* f) :
      FuncRef{
          noop_cast(
              +[](S* f, A... args) noexcept(NX) -> R
              { return f(std::forward<A>(args)...); }),
          f}
    {
    }

    // ---

    template <typename Cls>
        requires std::is_class_v<Cls>
    /* explicit */ FuncRef(Cls* cls) :
      FuncRef{
          +[](Cls* cls, A... args) noexcept(NX) -> R
          { return (*cls)(std::forward<A>(args)...); },
          cls}
    {
    }

    // ---

    auto empty() const noexcept -> bool
    {
        return storage_.f == nullptr;
    }

    operator bool() const noexcept
    {
        return !empty();
    }

    auto operator()(A... args) const noexcept(NX) -> R
    {
        return storage_.f(storage_.c, std::forward<A>(args)...);
    }

    // ---

    FuncRef(Unsafe_Tag, SX<void> f, void* c) noexcept : storage_{.f{f}, .c{c}}
    {
    }

    auto storage() && noexcept -> Storage
    {
        return std::move(storage_);
    }

    auto untyped() const noexcept -> UntypedFuncRef;

    operator UntypedFuncRef() const noexcept;

private:
    Storage storage_;

    static auto noop_cast(R (*p)(S*, A...) noexcept(NX)) noexcept -> SX<S>*
    {
        // The types of the argument and return value are in fact the same,
        // but clang++ (as of versions 18 and 19) reports an error without
        // a static cast, even though static_assert confirms the same type.
        // This is a clang bug; the static cast is a workaround.
        static_assert(std::same_as<R(S*, A...) noexcept(NX), SX<S>>);
#ifdef __clang__
        return static_cast<SX<S>*>(p);
#else
        return p;
#endif
    }
};

// ---

template <SignatureType S>
FuncRef(Indirect_Tag, S* f) -> FuncRef<S>;

template <SignatureType S, S* f>
FuncRef(Const_Tag<f>) -> FuncRef<S>;

template <
    typename AcceptedContext,
    typename R,
    typename... A,
    bool NX,
    R (*f)(AcceptedContext*, A...) noexcept(NX),
    typename PassedContext>
    requires std::convertible_to<PassedContext*, AcceptedContext*>
FuncRef(Const_Tag<f>, PassedContext* ctx) -> FuncRef<R(A...) noexcept(NX)>;

template <typename Cls, auto f>
    requires MethodOf<decltype(f), Cls>
FuncRef(Const_Tag<f>, Cls* cls) -> FuncRef<SignatureOf_t<decltype(f), Cls>>;

template <typename Cls>
    requires std::is_class_v<Cls>
FuncRef(Cls* cls) -> FuncRef<SignatureOf_t<decltype(&Cls::operator()), Cls>>;

// ---------------------------------------------------------------------------

class UntypedFuncRef final
{
public:
    struct Storage final
    {
        void* f{};
        void* c{};
    };

    UntypedFuncRef() = default;

    template <typename R, typename... A, bool NX>
    /* implicit */ UntypedFuncRef(FuncRef<R(A...) noexcept(NX)> f_ref) noexcept :
      storage_{make_storage(std::move(f_ref))}
    {
    }

    auto empty() const noexcept -> bool
    {
        return storage_.f == nullptr;
    }

    operator bool() const noexcept
    {
        return !empty();
    }

    template <SignatureType S>
    auto typed(Unsafe_Tag, Type_Tag<S> = {}) const noexcept -> FuncRef<S>
    {
        using f_type = typename FuncRef<S>::template SX<void>*;
        auto f = reinterpret_cast<f_type>(storage_.f);
        return {Unsafe, f, storage_.c};
    }

private:
    Storage storage_;

    template <typename R, typename... A, bool NX>
    static auto make_storage(FuncRef<R(A...) noexcept(NX)> f_ref) -> Storage
    {
        auto typed_storage = std::move(f_ref).storage();
        return {
            .f{reinterpret_cast<void*>(typed_storage.f)},
            .c{typed_storage.c}};
    }
};

// ---------------------------------------------------------------------------

template <typename R, typename... A, bool NX>
auto FuncRef<R(A...) noexcept(NX)>::untyped() const noexcept -> UntypedFuncRef
{
    return {*this};
}

template <typename R, typename... A, bool NX>
FuncRef<R(A...) noexcept(NX)>::operator UntypedFuncRef() const noexcept
{
    return untyped();
}

} // namespace mpk::mix
