/** @file
 * @brief Structured command-line argument parsing
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/serial/parse_simple_value.hpp"
#include "mpk/mix/util/throw.hpp"

#include <magic_enum/magic_enum.hpp>

#include <cassert>
#include <cstddef>
#include <format>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

namespace mpk::mix
{

struct ArgParseState final
{
    int argc;
    char** argv;
    int index{1};
};

using ConsumeArgFunc = std::function<void(std::string_view)>;
using TriggerArgFunc = std::function<void()>;

struct Arg
{
    std::string long_name;
    char short_name;
    std::string description;
    std::optional<std::string> default_value;
    bool positional;
    ConsumeArgFunc consume;
    TriggerArgFunc trigger;

    template <typename T>
    static auto make(
        std::string_view name,
        T& dst,
        std::type_identity_t<T const&> default_value,
        std::string description) -> Arg
    {
        dst = default_value;
        std::string default_str;
        if constexpr (std::is_enum_v<T>)
            default_str = magic_enum::enum_name(default_value);
        else
            default_str = std::format("{}", default_value);
        return make_impl(name, dst, std::move(default_str), std::move(description));
    }

    template <typename T>
    static auto make(std::string_view name, T& dst, std::string description)
        -> Arg
    {
        return make_impl(name, dst, std::nullopt, std::move(description));
    }

private:
    template <typename T>
    static auto make_impl(
        std::string_view name,
        T& dst,
        std::optional<std::string> default_value,
        std::string description) -> Arg
    {
        auto [long_name, short_name, positional] = parse_name(name);
        ConsumeArgFunc consume;
        TriggerArgFunc trigger;
        if constexpr (std::same_as<std::string, T>)
            consume = [&dst](std::string_view str) { dst = str; };
        else if constexpr (std::same_as<bool, T>)
        {
            if (positional)
                throw_("Positional argument cannot be of type `bool`");
            if (dst || !default_value.has_value())
                throw_(
                    "For a boolean option, the default value should be `false`");
            trigger = [&dst]() { dst = true; };
        }
        else
            consume = [&dst](std::string_view str_view)
            { parse_simple_value(dst, str_view); };
        return {
            .long_name = std::move(long_name),
            .short_name = short_name,
            .description = std::move(description),
            .default_value = std::move(default_value),
            .positional = positional,
            .consume = std::move(consume),
            .trigger = std::move(trigger)};
    }

    static auto parse_name(std::string_view name)
        -> std::tuple<std::string, char, bool>;
};

class ArgParser final
{
public:
    struct Help_Tag final
    {
    };
    static constexpr auto Help = Help_Tag{};

    explicit ArgParser(std::string description);

    template <typename... Args>
    auto arg(Args&&... args) -> ArgParser&
    {
        args_.push_back(Arg::make(std::forward<Args>(args)...));
        return *this;
    }

    auto arg(Help_Tag) -> ArgParser&;

    auto run(ArgParseState& parse_state, bool more_parsers_follow) -> void;

    auto run(int argc, char** argv) -> void;

    auto help(bool more_parsers_follow) -> bool;

    auto validate() -> void;

private:
    std::string description_;
    std::vector<Arg> args_;
    bool help_{false};
    std::string run_error_;
};

} // namespace mpk::mix
