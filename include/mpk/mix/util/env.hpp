/** @file
 * @brief Read environment variables with optional default values
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/meta/type.hpp"
#include "mpk/mix/serial/parse_simple_value.hpp"
#include "mpk/mix/util/transform_optional.hpp"

#include <cstdlib>
#include <filesystem>
#include <optional>
#include <string>

namespace mpk::mix
{

inline auto get_env(char const* name) -> std::optional<std::string>
{
    auto const* value = ::getenv(name);
    if (!value)
        return std::nullopt;
    return std::string{value};
}

template <typename T>
auto get_env(char const* name, Type_Tag<T> = {}) -> std::optional<T>
{
    auto transform = [](std::string const& s) -> T
    {
        auto result = T{};
        parse_simple_value(result, std::string_view{s});
        return result;
    };
    return transform_optional<T>(get_env(name), transform);
}

inline auto get_env_path(char const* name)
    -> std::optional<std::filesystem::path>
{
    return transform_optional<std::filesystem::path>(get_env(name));
}

} // namespace mpk::mix
