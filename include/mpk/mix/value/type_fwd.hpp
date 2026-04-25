/** @file
 * @brief Type system forward declarations — scalar/string/set type IDs, concepts,
 *        and registration macros for enum and custom types.
 *
 * Copyright (C) 2024-2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/enum_flags.hpp"
#include "mpk/mix/meta/enum_type.hpp"
#include "mpk/mix/meta/type.hpp"
#include "mpk/mix/struct_type.hpp"

#include <magic_enum/magic_enum.hpp>

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>


namespace mpk::mix::value {

enum class ScalarTypeId : uint8_t
{
    Bool,
    Byte,
    F32,
    F64,
    I8,
    I16,
    I32,
    I64,
    U8,
    U16,
    U32,
    U64
};

#define MPKMIX_VALUE_DECL_SCALAR_TYPE_ID(T, id)                             \
    constexpr inline auto scalar_type_id_of(mpk::mix::Type_Tag<T>) noexcept \
        -> ScalarTypeId                                                     \
    { return ScalarTypeId::id; }                                            \
    static_assert(true)

MPKMIX_VALUE_DECL_SCALAR_TYPE_ID(bool     , Bool);
MPKMIX_VALUE_DECL_SCALAR_TYPE_ID(std::byte, Byte);
MPKMIX_VALUE_DECL_SCALAR_TYPE_ID(float    , F32 );
MPKMIX_VALUE_DECL_SCALAR_TYPE_ID(double   , F64 );
MPKMIX_VALUE_DECL_SCALAR_TYPE_ID(int8_t   , I8  );
MPKMIX_VALUE_DECL_SCALAR_TYPE_ID(int16_t  , I16 );
MPKMIX_VALUE_DECL_SCALAR_TYPE_ID(int32_t  , I32 );
MPKMIX_VALUE_DECL_SCALAR_TYPE_ID(int64_t  , I64 );
MPKMIX_VALUE_DECL_SCALAR_TYPE_ID(uint8_t  , U8  );
MPKMIX_VALUE_DECL_SCALAR_TYPE_ID(uint16_t , U16 );
MPKMIX_VALUE_DECL_SCALAR_TYPE_ID(uint32_t , U32 );
MPKMIX_VALUE_DECL_SCALAR_TYPE_ID(uint64_t , U64 );

#undef MPKMIX_VALUE_DECL_SCALAR_TYPE_ID

template <typename T>
concept ScalarType =
    requires{ scalar_type_id_of(mpk::mix::Type<T>); };


enum class StringTypeId : uint8_t
{
    String,
    StringView
};

#define MPKMIX_VALUE_DECL_STRING_TYPE_ID(T, id)                             \
constexpr inline auto string_type_id_of(mpk::mix::Type_Tag<T>) noexcept    \
    -> StringTypeId                                                         \
{ return StringTypeId::id; }                                                \
    static_assert(true)

MPKMIX_VALUE_DECL_STRING_TYPE_ID(std::string     , String);
MPKMIX_VALUE_DECL_STRING_TYPE_ID(std::string_view, StringView);

#undef MPKMIX_VALUE_DECL_STRING_TYPE_ID

template <typename T>
concept StringType =
    std::same_as<T, std::string> || std::same_as<T, std::string_view>;


enum class SetTypeId : uint8_t
{
    EnumFlags
};

template <mpk::mix::EnumFlagsType T>
constexpr inline auto set_type_id_of(mpk::mix::Type_Tag<T>) noexcept
    -> SetTypeId
{ return SetTypeId::EnumFlags; }


template <typename> struct CustomTypeToId;
template <uint8_t> struct IdToCustomType;

template <typename T>
concept RegisteredCustomType =
    requires
{
    CustomTypeToId<T>::id;
    CustomTypeToId<T>::name;
    requires IdToCustomType<CustomTypeToId<T>::id>::id == CustomTypeToId<T>::id;
};


template <mpk::mix::EnumType E> struct EnumTypeToId;
template <uint8_t> struct IdToEnumType;

template <typename T>
concept RegisteredEnumType =
    requires
{
    requires mpk::mix::EnumType<T>;
    EnumTypeToId<T>::id;
    EnumTypeToId<T>::name;
    requires IdToEnumType<EnumTypeToId<T>::id>::id == EnumTypeToId<T>::id;
};

enum class AggregateType : uint8_t
{
    Array,
    Custom,
    Enum,
    Path,
    Scalar,
    Set,
    String,
    Strong,
    Struct,
    Tuple,
    Vector
};

class Type;

} // namespace mpk::mix::value


// Registration macros — use at file scope (outside any namespace)

#define MPKMIX_VALUE_IMPL_REGISTER_TYPE(T, id_, MapToId, MapFromId)         \
    namespace mpk::mix::value {                                             \
    template <> struct MapToId<T>                                           \
    {                                                                       \
        using type = T;                                                     \
        static constexpr uint8_t id = id_;                                  \
        static constexpr std::string_view name = #T;                        \
    };                                                                      \
                                                                            \
    template <> struct MapFromId<id_>                                       \
    {                                                                       \
        using type = T;                                                     \
        static constexpr uint8_t id = id_;                                  \
    };                                                                      \
    }                                                                       \
    static_assert(true)

#define MPKMIX_VALUE_REGISTER_CUSTOM_TYPE(T, id_)                           \
    MPKMIX_VALUE_IMPL_REGISTER_TYPE(T, id_, CustomTypeToId, IdToCustomType)

#define MPKMIX_VALUE_REGISTER_ENUM_TYPE(T, id_)                             \
    MPKMIX_VALUE_IMPL_REGISTER_TYPE(T, id_, EnumTypeToId, IdToEnumType)
