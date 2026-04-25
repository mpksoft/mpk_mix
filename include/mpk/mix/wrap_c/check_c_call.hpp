/** @file
 * @brief check_c_call() throws on negative POSIX return values
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include <cstring>

namespace mpk::mix::wrap_c {

auto throw_if_minus_one(
    int value, char const* file, int line, char const* errmsg = nullptr) -> int;

auto throw_if_nonzero(
    int value, char const* file, int line, char const* errmsg = nullptr) -> int;

auto throw_if_zero(
    int value, char const* file, int line, char const* errmsg = nullptr) -> int;

auto terminate_if_minus_one(
    int value, char const* file, int line, char const* errmsg = nullptr) -> int;

auto terminate_if_nonzero(
    int value, char const* file, int line, char const* errmsg = nullptr) -> int;

auto terminate_if_zero(
    int value, char const* file, int line, char const* errmsg = nullptr) -> int;

auto log_if_minus_one(
    int value, char const* file, int line, char const* errmsg = nullptr) -> int;

auto log_if_nonzero(
    int value, char const* file, int line, char const* errmsg = nullptr) -> int;

auto log_if_zero(
    int value, char const* file, int line, char const* errmsg = nullptr) -> int;

} // namespace mpk::mix::wrap_c

#define MPKMIX_THROW_IF_MINUS_ONE(value, ...)          \
    ::mpk::mix::wrap_c::throw_if_minus_one(            \
        value, __FILE__, __LINE__, ##__VA_ARGS__)

#define MPKMIX_THROW_IF_NONZERO(value, ...)            \
    ::mpk::mix::wrap_c::throw_if_nonzero(              \
        value, __FILE__, __LINE__, ##__VA_ARGS__)

#define MPKMIX_THROW_IF_ZERO(value, ...)               \
    ::mpk::mix::wrap_c::throw_if_zero(                 \
        value, __FILE__, __LINE__, ##__VA_ARGS__)

#define MPKMIX_TERMINATE_IF_MINUS_ONE(value, ...)      \
    ::mpk::mix::wrap_c::terminate_if_minus_one(        \
        value, __FILE__, __LINE__, ##__VA_ARGS__)

#define MPKMIX_TERMINATE_IF_NONZERO(value, ...)        \
    ::mpk::mix::wrap_c::terminate_if_nonzero(          \
        value, __FILE__, __LINE__, ##__VA_ARGS__)

#define MPKMIX_TERMINATE_IF_ZERO(value, ...)           \
    ::mpk::mix::wrap_c::terminate_if_zero(             \
        value, __FILE__, __LINE__, ##__VA_ARGS__)

#define MPKMIX_LOG_IF_MINUS_ONE(value, ...)            \
    ::mpk::mix::wrap_c::log_if_minus_one(              \
        value, __FILE__, __LINE__, ##__VA_ARGS__)

#define MPKMIX_LOG_IF_NONZERO(value, ...)              \
    ::mpk::mix::wrap_c::log_if_nonzero(                \
        value, __FILE__, __LINE__, ##__VA_ARGS__)

#define MPKMIX_LOG_IF_ZERO(value, ...)                 \
    ::mpk::mix::wrap_c::log_if_zero(                   \
        value, __FILE__, __LINE__, ##__VA_ARGS__)
