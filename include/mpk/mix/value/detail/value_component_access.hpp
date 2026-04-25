/** @file
 * @brief Internal: ValueComponentAccess<Type> and related dispatch machinery.
 *
 * Copyright (C) 2024-2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/value/type_fwd.hpp"
#include "mpk/mix/value/value_path.hpp"

#include "mpk/mix/detail/set_like.hpp"
#include "mpk/mix/meta/nil.hpp"
#include "mpk/mix/meta/type.hpp"
#include "mpk/mix/strong/fwd.hpp"
#include "mpk/mix/struct_type.hpp"
#include "mpk/mix/util/defer.hpp"
#include "mpk/mix/util/format_seq.hpp"
#include "mpk/mix/util/maybe_const.hpp"
#include "mpk/mix/util/throw.hpp"
#include "mpk/mix/util/tuple_like.hpp"

#include <algorithm>
#include <any>
#include <cassert>
#include <functional>
#include <memory>
#include <stdexcept>
#include <tuple>


namespace mpk::mix::value::detail {

template <typename T>
constexpr inline auto actually_equality_comparable_v =
    std::equality_comparable<T>;

template <typename T>
    requires requires(const T& v)
    { std::begin(v); }
constexpr inline auto actually_equality_comparable_v<T> =
    actually_equality_comparable_v<decltype(*std::begin(std::declval<T>()))>;

template <typename... Ts>
constexpr inline auto actually_equality_comparable_v<std::tuple<Ts...>> =
    (actually_equality_comparable_v<Ts> && ...);

template <typename T0, typename T1>
constexpr inline auto actually_equality_comparable_v<std::pair<T0, T1>> =
    actually_equality_comparable_v<T0> && actually_equality_comparable_v<T1>;

// NOTE: The reason to introduce `actually_equality_comparable` is that
// `std::equality_comparable` is always `true` for standard containers, which
// leads to compilation errors when comparing a container of `T` elements with
// `std::equality_comparable<T>` == false.
template <typename T>
concept actually_equality_comparable = actually_equality_comparable_v<T>;

// ---

template <typename Type>
struct ValueComponentAccess
{
    virtual ~ValueComponentAccess() = default;

    virtual auto path_intem_keys(const std::any& data) const
        -> std::vector<ValuePathItem> = 0;

    virtual auto get(ValuePathView path,
                     const std::any& data) const
        -> std::pair<const Type*, std::any> = 0;

    virtual auto set(ValuePathView path,
                     std::any& data,
                     const std::any& src) const
        -> void = 0;

    virtual auto size(ValuePathView path,
                      const std::any& data) const
        -> size_t = 0;

    virtual auto resize(ValuePathView path,
                        std::any& data,
                        size_t size) const
        -> void = 0;

    virtual auto keys(const std::any& data) const
        -> std::pair<const Type*, std::vector<std::any>> = 0;

    virtual auto contains(const std::any& data,
                          const std::any& key) const -> bool = 0;

    virtual auto insert(ValuePathView path,
                        std::any& data,
                        const std::any& key) const -> void = 0;

    virtual auto remove(ValuePathView path,
                        std::any& data,
                        const std::any& key) const -> void = 0;

    virtual auto equal(const std::any& lhs, const std::any& rhs) const
        -> bool = 0;

    virtual auto make_data() const
        -> std::any = 0;
};

// ---

template <typename T>
auto unpack(std::any& data, mpk::mix::Type_Tag<T> = {})
    -> T&
{ return std::any_cast<T&>(data); }

template <typename T>
auto unpack(const std::any& data, mpk::mix::Type_Tag<T> = {})
    -> const T&
{ return std::any_cast<const T&>(data); }

template <size_t N, typename F, typename... Args>
auto visit_index(std::integral_constant<size_t, N>,
                 size_t index,
                 F&& f,
                 Args&&... args)
    -> std::invoke_result_t< F, std::integral_constant<size_t, 0>, Args... >
{
    if constexpr(N == 0)
        throw std::invalid_argument("Index is out of range");

    else
    {
        constexpr auto prev = std::integral_constant<size_t, N-1>{};
        if (index + 1 == N)
            return std::invoke(std::forward<F>(f),
                               prev,
                               std::forward<Args>(args)...);
        else
            return visit_index(prev,
                               index,
                               std::forward<F>(f),
                               std::forward<Args>(args)...);
    }
}

// ---

template <typename Type, typename T>
struct ValueComponents;

template <typename Type, typename T>
struct ValueComponentAccessImpl final : ValueComponentAccess<Type>
{
    auto path_intem_keys(const std::any& data) const
        -> std::vector<ValuePathItem> override
    {
        constexpr auto make_path_index_items =
            [](size_t size)
            {
                auto result = std::vector<ValuePathItem>{};
                result.reserve(size);
                for (size_t index=0; index<size; ++index)
                    result.emplace_back(index);
                return result;
            };
        constexpr auto tag = mpk::mix::Type<T>;
        const auto& v = unpack(data, tag);
        if constexpr (mpk::mix::StructType<T>)
        {
            constexpr auto fields = field_names_of(tag);
            return std::vector<ValuePathItem>(fields.begin(), fields.end());
        }
        else if constexpr (requires { std::size(v); })
            return make_path_index_items(std::size(v));
        else if constexpr (mpk::mix::is_tuple_like_v<T>)
            return make_path_index_items(std::tuple_size_v<T>);
        else
            return {};
    }

    auto get(ValuePathView path, const std::any& data) const
        -> std::pair<const Type*, std::any> override
    {
        return ValueComponents<Type, T>::dispatch(
            path,
            unpack<T>(data),
            [](const auto& v, auto tag) -> std::pair<const Type*, std::any>
            { return {Type::of(tag), v}; });
    }

    auto set(ValuePathView path, std::any& data, const std::any& src) const
        -> void override
    {
        ValueComponents<Type, T>::dispatch(
            path,
            unpack<T>(data),
            [&](auto& v, auto tag)
            { v = unpack(src, tag); });
    }

    auto size(ValuePathView path, const std::any& data) const
        -> size_t override
    {
        return ValueComponents<Type, T>::dispatch(
            path,
            unpack<T>(data),
            [](const auto& v, auto /* tag */) -> size_t
            {
                if constexpr (requires { std::size(v); })
                    return std::size(v);
                else
                    throw std::invalid_argument(
                        "Objects of this type do not have a size");
            });
    }

    auto resize(ValuePathView path, std::any& data, size_t size) const
        -> void override
    {
        ValueComponents<Type, T>::dispatch(
            path,
            unpack<T>(data),
            [&](auto& v, auto /* tag */)
            {
                if constexpr (requires { v.resize(size); })
                    v.resize(size);
                else
                    throw std::invalid_argument(
                        "Objects of this type cannot be resized");
            });
    }

    auto keys(const std::any& data) const
        -> std::pair<const Type*, std::vector<std::any>> override
    {
        using Result = std::pair<const Type*, std::vector<std::any>>;

        constexpr auto make_path_index_items =
            [](size_t size) -> Result
        {
            auto keys = std::vector<std::any>{};
            keys.reserve(size);
            for (size_t index=0; index<size; ++index)
                keys.emplace_back(index);
            const auto* key_type = Type::of(mpk::mix::Type<size_t>);
            return {key_type, std::move(keys)};
        };
        constexpr auto tag = mpk::mix::Type<T>;
        const auto& v = unpack(data, tag);
        if constexpr (mpk::mix::StructType<T>)
        {
            constexpr auto fields = field_names_of(tag);
            const auto* key_type = Type::of(mpk::mix::Type<std::string_view>);
            auto keys = std::vector<std::any>(fields.begin(), fields.end());
            return {key_type, std::move(keys)};
        }
        else if constexpr (mpk::mix::detail::SetLikeType<T>)
        {
            const auto* key_type = Type::of(mpk::mix::Type<typename T::key_type>);
            auto keys = std::vector<std::any>(v.begin(), v.end());
            return {key_type, std::move(keys)};
        }
        else if constexpr (requires { std::size(v); })
            return make_path_index_items(std::size(v));
        else if constexpr (mpk::mix::is_tuple_like_v<T>)
            return make_path_index_items(std::tuple_size_v<T>);
        else
            throw std::invalid_argument(
                "Method 'keys' is not supported for objects of this type");
    }

    auto contains(const std::any& data,
                  const std::any& key) const -> bool override
    {
        if constexpr (mpk::mix::detail::SetLikeType<T>)
        {
            constexpr auto tag = mpk::mix::Type<T>;
            constexpr auto key_tag = mpk::mix::Type<typename T::key_type>;
            const auto& v = unpack(data, tag);
            const auto& k = unpack(key, key_tag);
            return v.contains(k);
        }
        else
            throw std::invalid_argument(
                "Method 'contains' is not supported for objects of this type");
    }

    auto insert(ValuePathView,
                std::any& data,
                const std::any& key) const -> void override
    {
        if constexpr (mpk::mix::detail::SetLikeType<T>)
        {
            constexpr auto tag = mpk::mix::Type<T>;
            constexpr auto key_tag = mpk::mix::Type<typename T::key_type>;
            auto& v = unpack(data, tag);
            const auto& k = unpack(key, key_tag);
            v.insert(k);
        }
        else
            throw std::invalid_argument(
                "Method 'insert' is not supported for objects of this type");
    }

    auto remove(ValuePathView,
                std::any& data,
                const std::any& key) const -> void override
    {
        if constexpr (mpk::mix::detail::SetLikeType<T>)
        {
            constexpr auto tag = mpk::mix::Type<T>;
            constexpr auto key_tag = mpk::mix::Type<typename T::key_type>;
            auto& v = unpack(data, tag);
            const auto& k = unpack(key, key_tag);
            while (true)
            {
                auto it = v.find(k);
                if (it == v.end())
                    return;
                v.erase(it);
            }
        }
        else
            throw std::invalid_argument(
                "Method 'remove' is not supported for objects of this type");
    }


    auto equal(const std::any& lhs, const std::any& rhs) const
        -> bool override
    {
        if constexpr (actually_equality_comparable<T>)
            return unpack<T>(lhs) == unpack<T>(rhs);
        else
        {
            auto k = path_intem_keys(lhs);
            if (k != path_intem_keys(rhs))
                return false;
            for (const auto& key : k)
            {
                auto [tl, dl] = get(ValuePath{key}, lhs);
                auto [tr, dr] = get(ValuePath{key}, rhs);
                if (tl != tr)
                    return false;
                if (!tl->value_component_access()->equal(dl, dr))
                    return false;
            }
            return true;
        }
    }

    auto make_data() const
        -> std::any override
    { return T{}; }
};

// ---

template <typename Type, ScalarType T>
struct ValueComponents<Type, T> final
{
    template <mpk::mix::MaybeConst<T> U, typename F>
    static auto dispatch([[maybe_unused]] ValuePathView path, U& data, F&& f)
    {
        assert(path.empty());
        return std::invoke(std::forward<F>(f), data, mpk::mix::Type<T>);
    }
};

template <typename Type, mpk::mix::detail::SetLikeType T>
struct ValueComponents<Type, T> final
{
    template <mpk::mix::MaybeConst<T> U, typename F>
    static auto dispatch([[maybe_unused]] ValuePathView path, U& data, F&& f)
    {
        // NOTE:
        //  Navigation through keys of a set is not possible because
        //  they cannot, in a general case, be represented by path items.
        assert(path.empty());
        return std::invoke(std::forward<F>(f), data, mpk::mix::Type<T>);
    }
};

template <typename Type>
struct ValueComponents<Type, ValuePath>
{
    template <mpk::mix::MaybeConst<ValuePath> U, typename F>
    static auto dispatch([[maybe_unused]] ValuePathView path, U& data, F&& f)
    {
        assert(path.empty());
        return std::invoke(std::forward<F>(f), data, mpk::mix::Type<ValuePath>);
    }
};

template <typename Type, typename T>
struct ValueComponents<Type, std::vector<T>>
{
    using V = std::vector<T>;

    template <mpk::mix::MaybeConst<V> U, typename F>
    static auto dispatch(ValuePathView path, U& data, F&& f)
    {
        if(path.empty())
            return std::invoke(std::forward<F>(f), data, mpk::mix::Type<V>);

        auto index = path[0].index();
        auto& element = data.at(index);
        return ValueComponents<Type, T>::dispatch(
            path.subspan(1), element, std::forward<F>(f));
    }
};

template <typename Type, typename T, size_t N>
struct ValueComponents<Type, std::array<T, N>>
{
    using V = std::array<T, N>;

    template <mpk::mix::MaybeConst<V> U, typename F>
    static auto dispatch(ValuePathView path, U& data, F&& f)
    {
        if(path.empty())
            return std::invoke(std::forward<F>(f), data, mpk::mix::Type<V>);

        auto index = path[0].index();
        auto& element = data.at(index);
        return ValueComponents<Type, T>::dispatch(
            path.subspan(1), element, std::forward<F>(f));
    }
};

template <typename Type, typename... Ts>
struct ValueComponents<Type, std::tuple<Ts...>>
{
    using V = std::tuple<Ts...>;

    template <mpk::mix::MaybeConst<V> U, typename F>
    static auto dispatch(ValuePathView path, U& data, F&& f)
    {
        if(path.empty())
            return std::invoke(std::forward<F>(f), data, mpk::mix::Type<V>);

        auto index = path[0].index();

        return visit_index(
            std::integral_constant<size_t, sizeof...(Ts)>{},
            index,
            [&]<size_t I>(std::integral_constant<size_t, I>)
            {
                auto& element = std::get<I>(data);
                using E = std::remove_cvref_t<decltype(element)>;
                return ValueComponents<Type, E>::dispatch(
                    path.subspan(1), element, std::forward<F>(f));
            });
    }
};

template <typename Type, mpk::mix::StructType T>
struct ValueComponents<Type, T>
{
    template <mpk::mix::MaybeConst<T> U, typename F>
    static auto dispatch(ValuePathView path, U& data, F&& f)
    {
        if(path.empty())
            return std::invoke(std::forward<F>(f), data, mpk::mix::Type<T>);

        auto field_name = path[0].name();
        constexpr auto field_names = field_names_of(mpk::mix::Type<T>);
        auto it = std::find(field_names.begin(), field_names.end(), field_name);
        assert(it != field_names.end());
        auto index = it - field_names.begin();
        auto fields = fields_of(data);
        constexpr auto field_count = field_names.size();

        return visit_index(
            std::integral_constant<size_t, field_count>{},
            index,
            [&]<size_t I>(std::integral_constant<size_t, I>)
            {
                auto& element = std::get<I>(fields);
                using E = std::remove_cvref_t<decltype(element)>;
                return ValueComponents<Type, E>::dispatch(
                    path.subspan(1), element, std::forward<F>(f));
            });
    }
};

template <typename Type, RegisteredCustomType T>
struct ValueComponents<Type, T>
{
    template <mpk::mix::MaybeConst<T> U, typename F>
    static auto dispatch([[maybe_unused]] ValuePathView path, U& data, F&& f)
    {
        assert(path.empty());
        return std::invoke(std::forward<F>(f), data, mpk::mix::Type<T>);
    }

    static constexpr auto type_specific_data = mpk::mix::Nil;
};

template <typename Type, RegisteredEnumType T>
struct ValueComponents<Type, T>
{
    template <mpk::mix::MaybeConst<T> U, typename F>
    static auto dispatch(ValuePathView path, U& data, F&& f)
    {
        namespace me = magic_enum;

        if (path.empty())
            return std::invoke(std::forward<F>(f), data, mpk::mix::Type<T>);

        auto name = path[0].name();
        if (name == "index")
        {
            auto opt_index = me::enum_index(data);
            if (!opt_index)
                mpk::mix::throw_(
                    "Unable to get an index for a value of enumerated type {}",
                    Type::template of<T>());
            auto defer = mpk::mix::Defer{
                [&]{
                    if constexpr (!std::is_const_v<U>)
                        data = me::enum_value<T>(*opt_index);
                }
            };

            return ValueComponents<Type, size_t>::dispatch(
                path.subspan(1), *opt_index, std::forward<F>(f));
        }

        if (name == "value")
        {
            using Underlying = me::underlying_type_t<T>;
            auto underlying = me::enum_integer(data);
            auto defer = mpk::mix::Defer{
                [&]{
                    if constexpr (!std::is_const_v<U>)
                    {
                        auto opt_value = me::enum_cast<T>(underlying);
                        if (!opt_value)
                            mpk::mix::throw_(
                                "Unable to convert integer '{}' into a value of enumerated type {}",
                                underlying,
                                Type::template of<T>());
                        data = *opt_value;
                    }
                }
            };

            return ValueComponents<Type, Underlying>::dispatch(
                path.subspan(1), underlying, std::forward<F>(f));
        }

        if (name == "name")
        {
            if constexpr (std::is_const_v<U>)
            {
                auto name = me::enum_name(data);
                return ValueComponents<Type, std::string_view>::dispatch(
                    path.subspan(1), name, std::forward<F>(f));
            }
            else
            {
                auto name = std::string{ me::enum_name(data) };
                auto defer = mpk::mix::Defer{
                    [&]{
                        auto opt_value = me::enum_cast<T>(name);
                        if (!opt_value)
                            mpk::mix::throw_(
                                "Unable to convert string '{}' into a value of enumerated type {}. Valid values are {}.",
                                name,
                                Type::template of<T>(),
                                mpk::mix::format_seq(
                                    magic_enum::enum_names<T>(), ", "));
                        data = *opt_value;
                    }
                };
                return ValueComponents<Type, std::string>::dispatch(
                    path.subspan(1), name, std::forward<F>(f));
            }
        }

        if (name == "names")
        {
            using Inner = std::vector<std::string_view>;
            constexpr auto names = me::enum_names<T>();
            auto name_vec = Inner(names.begin(), names.end());
            return ValueComponents<Type, Inner>::dispatch(
                    path.subspan(1), name_vec, std::forward<F>(f));
        }

        if (name == "values")
        {
            using Inner = std::vector<std::underlying_type_t<T>>;
            auto value_vec = []<size_t... I>(std::index_sequence<I...>) -> Inner
            {
                return { me::enum_integer(me::enum_value<T>(I)) ... };
            }(std::make_index_sequence<me::enum_count<T>()>());
            return ValueComponents<Type, Inner>::dispatch(
                path.subspan(1), value_vec, std::forward<F>(f));
        }

        mpk::mix::throw_(
            "For an enumerated type, only possible path keys are 'index', "
            "'name', 'names', 'value', and 'values'. Got '{}' for type {}",
            name, Type::template of<T>());
    }
};

template <typename Type, StringType T>
struct ValueComponents<Type, T>
{
    template <mpk::mix::MaybeConst<T> U, typename F>
    static auto dispatch([[maybe_unused]] ValuePathView path, U& data, F&& f)
    {
        assert(path.empty());
        return std::invoke(std::forward<F>(f), data, mpk::mix::Type<T>);
    }
};

template <typename Type, mpk::mix::StrongType T>
struct ValueComponents<Type, T>
{
    template <mpk::mix::MaybeConst<T> U, typename F>
    static auto dispatch(ValuePathView path, U& data, F&& f)
    {
        if(path.empty())
            return std::invoke(std::forward<F>(f), data, mpk::mix::Type<T>);

        auto name = path[0].name();
        if (name != "v")
            mpk::mix::throw_(
                "For a strong type, the only possible path key is 'v'. "
                "Got '{}' for type {}",
                name, Type::template of<T>());

        using Weak = typename T::Weak;
        auto& weak = data.v;
        return ValueComponents<Type, Weak>::dispatch(
            path.subspan(1), weak, std::forward<F>(f));
    }
};

template <typename T>
using ValueComponentsAccessFactoryFunc =
    std::unique_ptr<ValueComponentAccess<Type>>(*)();

template <typename Type, typename T>
constexpr auto value_components_access_factory(mpk::mix::Type_Tag<Type> = {},
                                               mpk::mix::Type_Tag<T> = {})
    -> ValueComponentsAccessFactoryFunc<Type>
{
    constexpr auto result = +[]()
        -> std::unique_ptr<ValueComponentAccess<Type>>
    {
        return std::make_unique< ValueComponentAccessImpl<Type, T> >();
    };
    return result;
}

} // namespace mpk::mix::value::detail
