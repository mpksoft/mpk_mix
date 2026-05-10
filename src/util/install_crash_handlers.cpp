/** @file
 * @brief Terminate + fatal-signal handler installation.
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/util/install_crash_handlers.hpp"

#ifdef MPKMIX_STACKTRACE

#include "mpk/mix/util/stacktrace.hpp"

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <typeinfo>

#include <csignal>
#include <unistd.h>


namespace mpk::mix {

namespace {

auto write_all(int fd, const char* s, std::size_t n) -> void
{
    while (n > 0) {
        auto w = ::write(fd, s, n);
        if (w <= 0) return;
        s += w;
        n -= static_cast<std::size_t>(w);
    }
}

auto write_cstr(int fd, const char* s) -> void
{
    write_all(fd, s, std::strlen(s));
}

[[noreturn]] auto terminate_handler() -> void
{
    auto throw_stack = last_throw_stacktrace_string();

    std::fputs("\n=== std::terminate called ===\n", stderr);

    if (auto eptr = std::current_exception()) {
        try {
            std::rethrow_exception(eptr);
        } catch (const std::exception& e) {
            std::fprintf(stderr, "exception type: %s\nwhat(): %s\n",
                         typeid(e).name(), e.what());
        } catch (...) {
            std::fputs("exception type: <non-std::exception>\n", stderr);
        }
    } else {
        std::fputs("(no active exception)\n", stderr);
    }

    if (!throw_stack.empty()) {
        std::fputs("\n--- stack at throw site ---\n", stderr);
        std::fputs(throw_stack.c_str(), stderr);
        std::fputc('\n', stderr);
    } else {
        auto here = current_stacktrace_string(1);
        std::fputs("\n--- stack at terminate ---\n", stderr);
        std::fputs(here.c_str(), stderr);
        std::fputc('\n', stderr);
    }

    std::fflush(stderr);
    std::abort();
}

std::atomic<bool> in_signal_handler_{false};

extern "C" auto signal_handler(int sig) -> void
{
    // Best-effort recursion guard: if a fatal signal hits while we are
    // formatting the trace, just bail out and let the default disposition run.
    bool expected = false;
    if (!in_signal_handler_.compare_exchange_strong(expected, true)) {
        std::signal(sig, SIG_DFL);
        std::raise(sig);
        return;
    }

    write_cstr(STDERR_FILENO, "\n=== fatal signal ");
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%d", sig);
    write_cstr(STDERR_FILENO, buf);
    write_cstr(STDERR_FILENO, " ===\n");

    auto trace = current_stacktrace_string(1);
    write_all(STDERR_FILENO, trace.data(), trace.size());
    write_cstr(STDERR_FILENO, "\n");

    // Re-raise with default disposition so the kernel can produce a core dump.
    std::signal(sig, SIG_DFL);
    std::raise(sig);
}

} // anonymous namespace


auto install_crash_handlers() -> void
{
    std::set_terminate(&terminate_handler);

    constexpr int signals[] = {
        SIGSEGV, SIGABRT, SIGFPE, SIGILL, SIGBUS,
    };
    for (int s : signals)
        std::signal(s, &signal_handler);
}

} // namespace mpk::mix

#endif // MPKMIX_STACKTRACE
