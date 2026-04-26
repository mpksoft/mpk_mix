/** @file
 * @brief Implementation of command-line argument parser
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/util/command_line_args.hpp"

#include "mpk/mix/detail/format.hpp"

#include <cassert>
#include <iostream>
#include <numeric>
#include <regex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std::string_view_literals;

namespace mpk::mix
{

auto Arg::parse_name(std::string_view name)
    -> std::tuple<std::string, char, bool>
{
    if (name.empty())
        throw_<std::invalid_argument>("Empty argument names are not allowed");
    auto rx = std::regex("^(--)?(\\w+)(,-[a-zA-Z])?$");
    auto const name_str = std::string{name};
    auto first_match_it =
        std::sregex_iterator(name_str.begin(), name_str.end(), rx);
    if (first_match_it == std::sregex_iterator())
        throw_<std::invalid_argument>(
            "Argument name '{}' does not match the required pattern", name);
    auto& match = *first_match_it;
    auto positional = match[1].str().empty();
    auto long_name = match[2].str();
    auto tail = match[3].str();
    auto short_name = '\0';
    if (!tail.empty())
    {
        if (positional)
            throw_<std::invalid_argument>(
                "Short form is not accepted for positional argument '{}'", name);
        assert(tail.size() == 3);
        short_name = tail[2];
    }
    ++first_match_it;
    assert(first_match_it == std::sregex_iterator());
    return {long_name, short_name, positional};
}

ArgParser::ArgParser(std::string description)
    : description_{std::move(description)}
{}

auto ArgParser::arg(Help_Tag) -> ArgParser&
{
    return arg("--help,-h", help_, help_, "Display this help message and exit");
}

auto ArgParser::run(ArgParseState& parse_state, bool more_parsers_follow) -> void
{
    size_t pos_index = 0;
    using OptMap = std::unordered_map<std::string_view, Arg const*>;
    OptMap opt_map;
    using ShortOptMap = std::unordered_map<char, Arg const*>;
    ShortOptMap short_opt_map;
    using PosVec = std::vector<Arg const*>;
    PosVec pos_vec;
    for (auto const& arg: args_)
    {
        if (arg.positional)
            pos_vec.push_back(&arg);
        else
        {
            opt_map[arg.long_name] = &arg;
            if (arg.short_name != '\0')
                short_opt_map[arg.short_name] = &arg;
        }
    }

    auto set_error =
        [&]<typename... Args>(MPKMIX_FORMAT_NS::format_string<Args...> fmt, Args&&... args)
        -> void { run_error_ = mpk::mix::format(fmt, std::forward<Args>(args)...); };

    auto consume_positional = [&](std::string_view arg_str)
    {
        if (pos_index >= pos_vec.size())
            return set_error("Too many positional arguments");
        auto& arg = pos_vec[pos_index++];
        arg->consume(arg_str);
    };

    std::unordered_set<std::string_view> long_options_specified;
    std::unordered_set<char> short_options_specified;

    ConsumeArgFunc consume_next;

    auto consume_long_option = [&](std::string_view arg_str)
    {
        auto key = arg_str.substr(2);
        auto it = opt_map.find(key);
        if (it == opt_map.end())
            return set_error("Unknown option '{}'", arg_str);
        long_options_specified.insert(key);
        if (auto short_name = it->second->short_name; short_name != '\0')
            short_options_specified.insert(short_name);
        auto& arg = *it->second;
        if (arg.trigger)
            arg.trigger();
        else
            consume_next = arg.consume;
    };

    auto consume_short_option = [&](std::string_view arg_str)
    {
        auto options_str = arg_str.substr(1);
        auto option_count = options_str.size();
        for (size_t option_index = 0; option_index < option_count; ++option_index)
        {
            auto key = options_str[option_index];
            auto it = short_opt_map.find(key);
            if (it == short_opt_map.end())
                return set_error("Unknown option '-{}'", key);
            auto& arg = *it->second;
            short_options_specified.insert(key);
            long_options_specified.insert(it->second->long_name);
            if (arg.trigger)
                arg.trigger();
            else if (option_count == 1)
                consume_next = arg.consume;
            else
                return set_error(
                    "Combined short options with arguments: '{}'", arg_str);
        }
    };

    auto last_arg_processed = false;
    auto last_option_processed = false;
    for (; parse_state.index < parse_state.argc && !last_arg_processed;
         ++parse_state.index)
    {
        auto arg_str = std::string_view{parse_state.argv[parse_state.index]};
        if (consume_next)
        {
            consume_next(arg_str);
            consume_next = {};
            continue;
        }

        auto is_positional =
            arg_str.empty() || arg_str[0] != '-' || last_option_processed;
        if (arg_str.size() > 1 && isdigit(arg_str[1]))
            is_positional = true;

        if (is_positional)
        {
            if (pos_index == pos_vec.size())
                break;
            consume_positional(arg_str);
            if (more_parsers_follow && pos_index == pos_vec.size())
                last_arg_processed = true;
        }
        else if (arg_str == "--"sv)
            last_option_processed = true;
        else if (arg_str.size() >= 2 && arg_str[1] == '-')
            consume_long_option(arg_str);
        else
            consume_short_option(arg_str);
    }

    if (consume_next)
        return set_error("Missing the value of an option");

    auto missing_mandatory_pos_args = std::accumulate(
        pos_vec.begin() + pos_index,
        pos_vec.end(),
        size_t{},
        [](size_t acc, Arg const* arg) -> size_t
        { return arg->default_value.has_value() ? acc : acc + 1; });
    if (missing_mandatory_pos_args > 0)
        return set_error(
            "Some positional arguments are missing ({})",
            missing_mandatory_pos_args);

    for (auto const& opt: opt_map)
    {
        if (long_options_specified.contains(opt.first))
            continue;
        if (opt.second->default_value.has_value())
            continue;
        return set_error("Mandatary option '--{}' is missing", opt.first);
    }

    for (auto const& opt: short_opt_map)
    {
        if (short_options_specified.contains(opt.first))
            continue;
        if (opt.second->default_value.has_value())
            continue;
        return set_error("Mandatary option '-{}' is missing", opt.first);
    }

    if (!more_parsers_follow && parse_state.index != parse_state.argc)
        return set_error(
            "Extra argument '{}'", parse_state.argv[parse_state.index]);
}

auto ArgParser::run(int argc, char** argv) -> void
{
    ArgParseState parse_state{argc, argv};
    run(parse_state, false);
}

auto ArgParser::help(bool more_parsers_follow) -> bool
{
    if (!help_)
        return false;

    auto print_arg_default = [](Arg const& arg)
    {
        std::cout << " (";
        if (arg.default_value.has_value())
        {
            if (arg.consume)
                std::cout << "optional, default='" << *arg.default_value << "'";
            else
                std::cout << "optional flag, disabled by default";
        }
        else
            std::cout << "mandatory";
        std::cout << ')' << std::endl;
    };

    std::cout << description_ << "\n\n";
    for (auto const& arg: args_)
    {
        if (arg.positional)
        {
            std::cout << "  " << arg.long_name;
            print_arg_default(arg);
        }
        else
        {
            std::cout << "  --" << arg.long_name;
            if (arg.short_name != '\0')
                std::cout << ",-" << arg.short_name;
            if (arg.consume)
                std::cout << " VALUE";
            print_arg_default(arg);
        }
        std::cout << "    " << arg.description << "\n\n";
    }

    if (more_parsers_follow)
        std::cout << "  ... (subcommand arguments)\n\n";

    return true;
}

auto ArgParser::validate() -> void
{
    if (run_error_.empty())
        return;
    throw std::invalid_argument(std::move(run_error_));
}

} // namespace mpk::mix
