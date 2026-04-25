/** @file
 * @brief General-purpose preprocessor utilities.
 *
 * Copyright (C) 2025 MPK Software, St.-Petersburg, Russia
 */

#pragma once

// ---------------------------------------------------------------------------
// MPKMIX_NUM_ARGS(extra, ...)
//
// Counts the number of arguments after the mandatory leading `extra` sentinel.
// Always call as MPKMIX_NUM_ARGS(extra, ...) — the sentinel absorbs the
// displacement introduced by the counting sequence.
// Maximum supported argument count: 12.
//
// Example:
//   MPKMIX_NUM_ARGS(extra)          → 0
//   MPKMIX_NUM_ARGS(extra, a, b)    → 2
// ---------------------------------------------------------------------------

#define MPKMIX_NUM_ARGS(...) \
    MPKMIX_NUM_ARGS_IMPL(__VA_ARGS__, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define MPKMIX_NUM_ARGS_IMPL( \
    extra, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, ...) \
    MPKMIX_NUM_ARGS_##a12

#define MPKMIX_NUM_ARGS_0  0
#define MPKMIX_NUM_ARGS_1  1
#define MPKMIX_NUM_ARGS_2  2
#define MPKMIX_NUM_ARGS_3  3
#define MPKMIX_NUM_ARGS_4  4
#define MPKMIX_NUM_ARGS_5  5
#define MPKMIX_NUM_ARGS_6  6
#define MPKMIX_NUM_ARGS_7  7
#define MPKMIX_NUM_ARGS_8  8
#define MPKMIX_NUM_ARGS_9  9
#define MPKMIX_NUM_ARGS_10 10
#define MPKMIX_NUM_ARGS_11 11
#define MPKMIX_NUM_ARGS_12 12

// ---------------------------------------------------------------------------
// Built-in separator tokens (used by MPKMIX_MAP_SEP_LIST via the _IMPL suffix
// convention — pass MPKMIX_COMMA, MPKMIX_SEMICOLON, or MPKMIX_COLON as `sep`).
// ---------------------------------------------------------------------------

#define MPKMIX_COMMA_IMPL     ,
#define MPKMIX_SEMICOLON_IMPL ;
#define MPKMIX_COLON_IMPL     :

#define MPKMIX_IDENTITY(x) x

// ---------------------------------------------------------------------------
// MPKMIX_MAP_SEP_LIST(map, payload, sep, ...)
//
// Applies the two-argument macro `map(payload, elem)` to each element in the
// variadic list, interleaving with separator `sep` (one of MPKMIX_COMMA,
// MPKMIX_SEMICOLON, MPKMIX_COLON, or a custom token that expands to `…_IMPL`).
//
// Example — declare struct fields:
//   #define FIELD(p, name) int name
//   struct S { MPKMIX_MAP_COMMA_SEP_LIST(FIELD, _, x, y, z); };
// ---------------------------------------------------------------------------

#define MPKMIX_MAP_SEP_LIST_0(m, p, s)
#define MPKMIX_MAP_SEP_LIST_1(m, p, s, a0) \
    m(p, a0)
#define MPKMIX_MAP_SEP_LIST_2(m, p, s, a0, a1) \
    m(p, a0) s m(p, a1)
#define MPKMIX_MAP_SEP_LIST_3(m, p, s, a0, a1, a2) \
    m(p, a0) s m(p, a1) s m(p, a2)
#define MPKMIX_MAP_SEP_LIST_4(m, p, s, a0, a1, a2, a3) \
    m(p, a0) s m(p, a1) s m(p, a2) s m(p, a3)
#define MPKMIX_MAP_SEP_LIST_5(m, p, s, a0, a1, a2, a3, a4) \
    m(p, a0) s m(p, a1) s m(p, a2) s m(p, a3) s m(p, a4)
#define MPKMIX_MAP_SEP_LIST_6(m, p, s, a0, a1, a2, a3, a4, a5) \
    m(p, a0) s m(p, a1) s m(p, a2) s m(p, a3) s m(p, a4) s m(p, a5)
#define MPKMIX_MAP_SEP_LIST_7(m, p, s, a0, a1, a2, a3, a4, a5, a6) \
    m(p, a0) s m(p, a1) s m(p, a2) s m(p, a3) s m(p, a4) \
    s m(p, a5) s m(p, a6)
#define MPKMIX_MAP_SEP_LIST_8(m, p, s, a0, a1, a2, a3, a4, a5, a6, a7) \
    m(p, a0) s m(p, a1) s m(p, a2) s m(p, a3) s m(p, a4) \
    s m(p, a5) s m(p, a6) s m(p, a7)
#define MPKMIX_MAP_SEP_LIST_9(m, p, s, a0, a1, a2, a3, a4, a5, a6, a7, a8) \
    m(p, a0) s m(p, a1) s m(p, a2) s m(p, a3) s m(p, a4) \
    s m(p, a5) s m(p, a6) s m(p, a7) s m(p, a8)
#define MPKMIX_MAP_SEP_LIST_10( \
    m, p, s, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9) \
    m(p, a0) s m(p, a1) s m(p, a2) s m(p, a3) s m(p, a4) \
    s m(p, a5) s m(p, a6) s m(p, a7) s m(p, a8) s m(p, a9)
#define MPKMIX_MAP_SEP_LIST_11( \
    m, p, s, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) \
    m(p, a0) s m(p, a1) s m(p, a2) s m(p, a3) s m(p, a4) \
    s m(p, a5) s m(p, a6) s m(p, a7) s m(p, a8) s m(p, a9) s m(p, a10)
#define MPKMIX_MAP_SEP_LIST_12( \
    m, p, s, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) \
    m(p, a0) s m(p, a1) s m(p, a2) s m(p, a3) s m(p, a4) \
    s m(p, a5) s m(p, a6) s m(p, a7) s m(p, a8) s m(p, a9) \
    s m(p, a10) s m(p, a11)

#define MPKMIX_MAP_SEP_LIST(map, p, sep, ...) \
    MPKMIX_MAP_SEP_LIST_N( \
        map, p, sep, MPKMIX_NUM_ARGS(extra, ##__VA_ARGS__), ##__VA_ARGS__)
#define MPKMIX_MAP_SEP_LIST_N(map, p, sep, n, ...) \
    MPKMIX_MAP_SEP_LIST_N_IMPL(map, p, sep, n, ##__VA_ARGS__)
#define MPKMIX_MAP_SEP_LIST_N_IMPL(map, p, sep, n, ...) \
    MPKMIX_MAP_SEP_LIST_##n(map, p, sep##_IMPL, ##__VA_ARGS__)

#define MPKMIX_MAP_COMMA_SEP_LIST(map, p, ...) \
    MPKMIX_MAP_SEP_LIST(map, p, MPKMIX_COMMA, ##__VA_ARGS__)

// ---------------------------------------------------------------------------
// MPKMIX_STRINGIZE(x)  — converts token x to a string literal.
// ---------------------------------------------------------------------------

#define MPKMIX_STRINGIZE(x) MPKMIX_STRINGIZE_IMPL(x)
#define MPKMIX_STRINGIZE_IMPL(x) #x

// ---------------------------------------------------------------------------
// MPKMIX_DEFAULT_A0_TO(DEFAULT, ...)
//
// Evaluates to DEFAULT when called with no further arguments, otherwise
// evaluates to the first additional argument.
//
// MPKMIX_DEFAULT_A1_TO(DEFAULT, ...)
//
// Evaluates to DEFAULT when called with fewer than two additional arguments,
// otherwise evaluates to the second additional argument.
// ---------------------------------------------------------------------------

#define MPKMIX_DEFAULT_A0_TO(DEFAULT, ...) \
    MPKMIX_DEFAULT_A0_TO_N( \
        DEFAULT, MPKMIX_NUM_ARGS(extra, ##__VA_ARGS__), ##__VA_ARGS__)
#define MPKMIX_DEFAULT_A0_TO_N(DEFAULT, n, ...) \
    MPKMIX_DEFAULT_A0_TO_N_IMPL(DEFAULT, n, ##__VA_ARGS__)
#define MPKMIX_DEFAULT_A0_TO_N_IMPL(DEFAULT, n, ...) \
    MPKMIX_DEFAULT_A0_TO_##n(DEFAULT, ##__VA_ARGS__)

#define MPKMIX_DEFAULT_A0_TO_0(DEFAULT)         DEFAULT
#define MPKMIX_DEFAULT_A0_TO_1(DEFAULT, A0)     A0
#define MPKMIX_DEFAULT_A0_TO_2(DEFAULT, A0, ...) A0
#define MPKMIX_DEFAULT_A0_TO_3(DEFAULT, A0, ...) A0
#define MPKMIX_DEFAULT_A0_TO_4(DEFAULT, A0, ...) A0
#define MPKMIX_DEFAULT_A0_TO_5(DEFAULT, A0, ...) A0
#define MPKMIX_DEFAULT_A0_TO_6(DEFAULT, A0, ...) A0
#define MPKMIX_DEFAULT_A0_TO_7(DEFAULT, A0, ...) A0
#define MPKMIX_DEFAULT_A0_TO_8(DEFAULT, A0, ...) A0
#define MPKMIX_DEFAULT_A0_TO_9(DEFAULT, A0, ...) A0
#define MPKMIX_DEFAULT_A0_TO_10(DEFAULT, A0, ...) A0
#define MPKMIX_DEFAULT_A0_TO_11(DEFAULT, A0, ...) A0
#define MPKMIX_DEFAULT_A0_TO_12(DEFAULT, A0, ...) A0

#define MPKMIX_DEFAULT_A1_TO(DEFAULT, ...) \
    MPKMIX_DEFAULT_A1_TO_N( \
        DEFAULT, MPKMIX_NUM_ARGS(extra, ##__VA_ARGS__), ##__VA_ARGS__)
#define MPKMIX_DEFAULT_A1_TO_N(DEFAULT, n, ...) \
    MPKMIX_DEFAULT_A1_TO_N_IMPL(DEFAULT, n, ##__VA_ARGS__)
#define MPKMIX_DEFAULT_A1_TO_N_IMPL(DEFAULT, n, ...) \
    MPKMIX_DEFAULT_A1_TO_##n(DEFAULT, ##__VA_ARGS__)

#define MPKMIX_DEFAULT_A1_TO_0(DEFAULT)              DEFAULT
#define MPKMIX_DEFAULT_A1_TO_1(DEFAULT, A0)           DEFAULT
#define MPKMIX_DEFAULT_A1_TO_2(DEFAULT, A0, A1, ...)  A1
#define MPKMIX_DEFAULT_A1_TO_3(DEFAULT, A0, A1, ...)  A1
#define MPKMIX_DEFAULT_A1_TO_4(DEFAULT, A0, A1, ...)  A1
#define MPKMIX_DEFAULT_A1_TO_5(DEFAULT, A0, A1, ...)  A1
#define MPKMIX_DEFAULT_A1_TO_6(DEFAULT, A0, A1, ...)  A1
#define MPKMIX_DEFAULT_A1_TO_7(DEFAULT, A0, A1, ...)  A1
#define MPKMIX_DEFAULT_A1_TO_8(DEFAULT, A0, A1, ...)  A1
#define MPKMIX_DEFAULT_A1_TO_9(DEFAULT, A0, A1, ...)  A1
#define MPKMIX_DEFAULT_A1_TO_10(DEFAULT, A0, A1, ...) A1
#define MPKMIX_DEFAULT_A1_TO_11(DEFAULT, A0, A1, ...) A1
#define MPKMIX_DEFAULT_A1_TO_12(DEFAULT, A0, A1, ...) A1
