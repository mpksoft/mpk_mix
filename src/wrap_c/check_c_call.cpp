/** @file
 * @brief Implementation of POSIX error-checking wrapper
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/wrap_c/check_c_call.hpp"

#include "mpk/mix/log.hpp"
#include "mpk/mix/util/throw.hpp"

#include <cerrno>
#include <cstring>
#include <iostream>

namespace mpk::mix::wrap_c {

namespace {

struct Throw
{
    auto operator()(
        int err, char const* file, int line, char const* errmsg) const -> void
    {
        throw_("{}:{}: {}", file, line, errmsg ? errmsg : strerror(err));
    }
};

struct Terminate
{
    auto operator()(
        int err, char const* file, int line, char const* errmsg) const -> void
    {
        std::cerr << "FATAL ERROR: " << file << ":" << line << ": "
                  << (errmsg ? errmsg : strerror(err)) << std::endl;
        std::terminate();
    }
};

struct Log
{
    auto operator()(
        int err, char const* file, int line, char const* errmsg) const -> void
    {
        // For the case of logging disabled
        std::ignore = err;
        std::ignore = file;
        std::ignore = line;
        std::ignore = errmsg;

        MPKMIX_LOG_ERROR(
            default_logger(),
            "{}:{}: {}",
            file,
            line,
            errmsg ? errmsg : strerror(err));
    }
};

template <typename Action, typename Pred>
auto fail_if(
    int value,
    Action act,
    Pred pred,
    char const* file,
    int line,
    char const* errmsg) -> int
{
    auto err = errno;

    if (!pred(value))
        return value;

    act(err, file, line, errmsg);
    return value;
}

template <typename Action>
auto fail_if_minus_one(
    int value, Action act, char const* file, int line, char const* errmsg) -> int
{
    return fail_if(
        value, act, [](int x) { return x == -1; }, file, line, errmsg);
}

template <typename Action>
auto fail_if_nonzero(
    int value, Action act, char const* file, int line, char const* errmsg) -> int
{
    return fail_if(
        value, act, [](int x) { return x != 0; }, file, line, errmsg);
}

template <typename Action>
auto fail_if_zero(
    int value, Action act, char const* file, int line, char const* errmsg) -> int
{
    return fail_if(
        value, act, [](int x) { return x == 0; }, file, line, errmsg);
}

} // anonymous namespace

auto throw_if_minus_one(
    int value, char const* file, int line, char const* errmsg) -> int
{
    return fail_if_minus_one(value, Throw{}, file, line, errmsg);
}

auto throw_if_nonzero(int value, char const* file, int line, char const* errmsg)
    -> int
{
    return fail_if_nonzero(value, Throw{}, file, line, errmsg);
}

auto throw_if_zero(int value, char const* file, int line, char const* errmsg)
    -> int
{
    return fail_if_zero(value, Throw{}, file, line, errmsg);
}

auto terminate_if_minus_one(
    int value, char const* file, int line, char const* errmsg) -> int
{
    return fail_if_minus_one(value, Terminate{}, file, line, errmsg);
}

auto terminate_if_nonzero(
    int value, char const* file, int line, char const* errmsg) -> int
{
    return fail_if_nonzero(value, Terminate{}, file, line, errmsg);
}

auto terminate_if_zero(int value, char const* file, int line, char const* errmsg)
    -> int
{
    return fail_if_zero(value, Terminate{}, file, line, errmsg);
}

auto log_if_minus_one(int value, char const* file, int line, char const* errmsg)
    -> int
{
    return fail_if_minus_one(value, Log{}, file, line, errmsg);
}

auto log_if_nonzero(int value, char const* file, int line, char const* errmsg)
    -> int
{
    return fail_if_nonzero(value, Log{}, file, line, errmsg);
}

auto log_if_zero(int value, char const* file, int line, char const* errmsg)
    -> int
{
    return fail_if_zero(value, Log{}, file, line, errmsg);
}

} // namespace mpk::mix::wrap_c
