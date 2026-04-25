/** @file
 * @brief MPKMIX_STRUCT_TYPE macro for declaring struct types
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/macro.hpp"
#include "mpk/mix/meta/type.hpp"

#include <array>
#include <string_view>
#include <tuple>

#define MPKMIX_STRUCT_TYPE_FIELD_OF(x, field) x.field

#define MPKMIX_STRUCT_TYPE_FIELD_NAME(_, field) #field

#define MPKMIX_STRUCT_TYPE_FIELD_TYPE(Class, field) decltype(Class::field)

#define MPKMIX_STRUCT_TYPE_FIELD_TYPE_REF(Class, field) decltype(Class::field)&

#define MPKMIX_STRUCT_TYPE_FIELD_TYPE_CREF(Class, field) \
    const decltype(Class::field)&

#define MPKMIX_STRUCT_TYPE(Struct, ...)                                       \
    [[maybe_unused]]                                                          \
    constexpr inline auto fields_of(Struct& x)                                \
        -> std::tuple<MPKMIX_MAP_COMMA_SEP_LIST(                              \
            MPKMIX_STRUCT_TYPE_FIELD_TYPE_REF, Struct, ##__VA_ARGS__)>        \
    {                                                                         \
        std::ignore = x;                                                      \
        return {MPKMIX_MAP_COMMA_SEP_LIST(                                    \
            MPKMIX_STRUCT_TYPE_FIELD_OF, x, ##__VA_ARGS__)};                  \
    }                                                                         \
                                                                              \
    [[maybe_unused]]                                                          \
    constexpr inline auto fields_of(const Struct& x)                          \
        -> std::tuple<MPKMIX_MAP_COMMA_SEP_LIST(                              \
            MPKMIX_STRUCT_TYPE_FIELD_TYPE_CREF, Struct, ##__VA_ARGS__)>       \
    {                                                                         \
        std::ignore = x;                                                      \
        return {MPKMIX_MAP_COMMA_SEP_LIST(                                    \
            MPKMIX_STRUCT_TYPE_FIELD_OF, x, ##__VA_ARGS__)};                  \
    }                                                                         \
                                                                              \
    auto fields_of(Struct&& x) = delete;                                      \
                                                                              \
    [[maybe_unused]]                                                          \
    constexpr inline auto tuple_tag_of(::mpk::mix::Type_Tag<Struct>)          \
        -> ::mpk::mix::Type_Tag<std::tuple<MPKMIX_MAP_COMMA_SEP_LIST(         \
            MPKMIX_STRUCT_TYPE_FIELD_TYPE, Struct, ##__VA_ARGS__)>>           \
    {                                                                         \
        return {};                                                            \
    }                                                                         \
                                                                              \
    [[maybe_unused]]                                                          \
    constexpr inline auto field_names_of(::mpk::mix::Type_Tag<Struct>)        \
        -> std::array<std::string_view, MPKMIX_NUM_ARGS(_, ##__VA_ARGS__)>    \
    {                                                                         \
        return {MPKMIX_MAP_COMMA_SEP_LIST(                                    \
            MPKMIX_STRUCT_TYPE_FIELD_NAME, _, ##__VA_ARGS__)};                \
    }                                                                         \
    static_assert(true)
