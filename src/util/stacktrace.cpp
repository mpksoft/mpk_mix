/** @file
 * @brief Implementation of crash-time stack trace capture.
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/util/stacktrace.hpp"

#ifdef MPKMIX_STACKTRACE

#include <cstdlib>
#include <format>
#include <optional>
#include <stacktrace>
#include <typeinfo>

#include <dlfcn.h>


namespace mpk::mix {

namespace {

thread_local std::optional<std::stacktrace> last_throw_stack_;

} // anonymous namespace


auto current_stacktrace_string(std::size_t skip) -> std::string
{
    return std::format("{}", std::stacktrace::current(skip + 1));
}

auto last_throw_stacktrace_string() -> std::string
{
    if (!last_throw_stack_)
        return {};
    return std::format("{}", *last_throw_stack_);
}

auto clear_last_throw_stacktrace() -> void
{
    last_throw_stack_.reset();
}

} // namespace mpk::mix


// __cxa_throw interception: capture a stack trace at the throw site, then
// forward to the real libstdc++ __cxa_throw resolved via dlsym(RTLD_NEXT).
//
// Note: this lives at file scope, outside any namespace, with C linkage so it
// participates in the Itanium ABI's symbol resolution for thrown exceptions.

// libstdc++ declares __cxa_throw with `void*` (not `std::type_info*`) for the
// second argument; match that to avoid a conflicting-declaration error.
extern "C" {

using cxa_throw_t = void (*)(void*, void*, void (*)(void*));

[[noreturn]] void __cxa_throw(void* obj,
                              void* tinfo,
                              void (*dest)(void*))
{
    static cxa_throw_t real_cxa_throw =
        reinterpret_cast<cxa_throw_t>(::dlsym(RTLD_NEXT, "__cxa_throw"));

    mpk::mix::last_throw_stack_ = std::stacktrace::current(1);

    if (real_cxa_throw == nullptr)
        std::abort();

    real_cxa_throw(obj, tinfo, dest);
    std::abort();
}

} // extern "C"

#endif // MPKMIX_STACKTRACE
