/** @file
 * @brief Implementation of RAII file descriptor
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/wrap_c/fd.hpp"

#include "mpk/mix/wrap_c/check_c_call.hpp"

#include <unistd.h>

#include <cassert>

namespace mpk::mix::wrap_c {

Fd::~Fd()
{
    close();
}

Fd::Fd() noexcept = default;

Fd::Fd(Unsafe_Tag, int fd) noexcept : fd_{fd}
{
}

Fd::Fd(Fd&& that) noexcept : fd_{that.detach(Unsafe)}
{
}

auto Fd::operator=(Fd&& that) noexcept -> Fd&
{
    if (this != &that)
    {
        close();
        fd_ = that.detach(Unsafe);
    }
    return *this;
}

auto Fd::empty() const noexcept -> bool
{
    return fd_ == 0;
}

auto Fd::fd() const noexcept -> int
{
    return fd_;
}

auto Fd::detach(Unsafe_Tag) noexcept -> int
{
    auto result = fd_;
    fd_ = 0;
    return result;
}

auto Fd::close() -> void
{
    if (fd_ != 0)
    {
        MPKMIX_LOG_IF_MINUS_ONE(::close(fd_));
        fd_ = 0;
    }
}

auto Fd::dup() const -> Fd
{
    assert(fd_ != 0);
    auto fd = MPKMIX_THROW_IF_MINUS_ONE(::dup(fd_));
    assert(fd != 0);
    return {Unsafe, fd};
}

} // namespace mpk::mix::wrap_c
