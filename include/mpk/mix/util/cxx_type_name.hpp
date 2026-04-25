/** @file
 * @brief Retrieve the C++ type name of a type or expression at runtime
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/meta/type.hpp"
#include "mpk/mix/util/throw.hpp"

#include <cxxabi.h>

#include <string>
#include <typeinfo>


namespace mpk::mix {

inline auto cxx_type_name(std::type_info const& type_info) -> std::string
{
    int status;
    char* demangled =
        abi::__cxa_demangle(type_info.name(), nullptr, nullptr, &status);

    if (status == 0)
    {
        std::string result(demangled);
        free(demangled);
        return result;
    }

    throw_("Failed to demangle type name {}, abi::__cxa_demangle returned {}",
           type_info.name(),
           status);
}

template <typename T>
auto cxx_type_name(Type_Tag<T> = {}) -> std::string
{
    return cxx_type_name(typeid(T));
}

} // namespace mpk::mix
