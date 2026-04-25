/** @file
 * @brief TypePack<Ts...> compile-time list of types
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

namespace mpk::mix
{

template <typename... Ts>
struct TypePack_Tag final
{
    static constexpr auto size = sizeof...(Ts);
};

template <typename... Ts>
constexpr inline auto TypePack = TypePack_Tag<Ts...>{};

// ---

template <typename>
struct first_in_type_pack;

template <typename T0, typename... Ts>
struct first_in_type_pack<TypePack_Tag<T0, Ts...>> final
{
    using type = T0;
};

template <typename Pack>
using first_in_type_pack_t = typename first_in_type_pack<Pack>::type;

} // namespace mpk::mix
