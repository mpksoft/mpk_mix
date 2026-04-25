/** @file
 * @brief POSIX read/write/send wrappers with ByteSpan convenience overloads
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/meta/type.hpp"
#include "mpk/mix/types/byte_span.hpp"
#include "mpk/mix/wrap_c/fd.hpp"
#include "mpk/mix/wrap_c/sock_addr.hpp"

#include <sys/types.h>

#include <array>
#include <string>
#include <utility>

namespace mpk::mix::wrap_c {

auto shutdown(Fd const&, int how) -> void;

auto try_shutdown(Fd const&, int how) -> bool;

auto shutdown_ignore_enotconn(Fd const&, int how) -> void;

auto try_read(Fd const&, ByteSpan<> buf) noexcept -> ssize_t;

auto read(Fd const&, ByteSpan<> buf) -> ByteSpan<>;

auto read_all(Fd const&, ByteSpan<> buf) -> void;

auto write(Fd const&, ConstByteSpan<> buf) -> size_t;

auto write_all(Fd const&, ConstByteSpan<> buf) -> void;

auto setblocking(Fd const&, bool blocking) -> void;

template <typename T>
auto getsockopt(Fd const&, int level, int optname, Type_Tag<T> = {}) -> T;

auto setsockopt(Fd const&, int level, int optname, int optval) -> void;

auto connect(Fd const&, SockAddrLite addr) -> void;

auto bind(Fd const&, SockAddrLite addr) -> void;

auto listen(Fd const&, int backlog = 128) -> void;

auto accept(Fd const&) -> std::pair<Fd, SockAddrLite>;

auto addr(Fd const&) -> SockAddrLite;

auto recvfrom(Fd const&, ByteSpan<> buf) -> std::pair<size_t, SockAddrLite>;

auto sendto(Fd const&, ConstByteSpan<> buf, SockAddrLite const& addr) -> size_t;

auto read_size(Fd const&) -> size_t;

auto try_read_size(Fd const&) -> std::pair<size_t, int>;

// Socket-specific

auto socket(int domain, int type, int protocol) -> Fd;

auto socketpair(int domain, int type, int protocol) -> std::array<Fd, 2>;

// File-specific

auto file(std::string const& path, int flags, mode_t mode = 0) -> Fd;

auto ftruncate(Fd const& fd, off_t length) -> void;

// Shared memory-specific

auto mmap(
    void* ptr, size_t length, int prot, int flags, Fd const& fd, off_t offset)
    -> void*;

auto munmap(void* addr, size_t length) -> void;

// Template overloads for convenience

template <size_t extent>
    requires(extent != std::dynamic_extent)
auto try_read(Fd const& fd, std::span<std::byte, extent> buf) -> ssize_t
{
    return try_read(fd, ByteSpan<>{buf});
}

template <typename Arg0, typename... Args>
auto try_read(Fd const& fd, Arg0&& arg0, [[maybe_unused]] Args&&... args) noexcept -> ssize_t
{
    return try_read(fd, dyn_byte_span(std::forward<Arg0>(arg0)));
}

template <size_t extent>
    requires(extent != std::dynamic_extent)
auto read(Fd const& fd, std::span<std::byte, extent> buf) -> ByteSpan<>
{
    return read(fd, ByteSpan<>{buf});
}

template <typename Arg0, typename... Args>
auto read(Fd const& fd, Arg0&& arg0, [[maybe_unused]] Args&&... args) -> ByteSpan<>
{
    return read(fd, dyn_byte_span(std::forward<Arg0>(arg0)));
}

template <size_t extent>
    requires(extent != std::dynamic_extent)
auto read_all(Fd const& fd, std::span<std::byte, extent> buf) -> void
{
    read_all(fd, ByteSpan<>{buf});
}

template <typename Arg0, typename... Args>
auto read_all(Fd const& fd, Arg0&& arg0, [[maybe_unused]] Args&&... args) -> void
{
    read_all(fd, dyn_byte_span(std::forward<Arg0>(arg0)));
}

template <ByteOrConstByte B, size_t extent>
    requires(!std::same_as<std::span<B, extent>, ConstByteSpan<>>)
auto write(Fd const& fd, std::span<B, extent> buf) -> size_t
{
    return write(fd, ConstByteSpan<>{buf});
}

template <typename Arg0, typename... Args>
    requires(!ByteSpanType<Arg0>)
auto write(Fd const& fd, Arg0&& arg0, [[maybe_unused]] Args&&... args) -> size_t
{
    return write(fd, dyn_byte_span(std::forward<Arg0>(arg0)));
}

template <ByteOrConstByte B, size_t extent>
    requires(!std::same_as<std::span<B, extent>, ConstByteSpan<>>)
auto write_all(Fd const& fd, std::span<B, extent> buf) -> void
{
    write_all(fd, ConstByteSpan<>{buf});
}

template <typename Arg0, typename... Args>
    requires(!ByteSpanType<Arg0>)
auto write_all(Fd const& fd, Arg0&& arg0, [[maybe_unused]] Args&&... args) -> void
{
    write_all(fd, dyn_byte_span(std::forward<Arg0>(arg0)));
}

template <typename... Args>
auto send(Fd const& fd, Args&&... args) -> void
{
    write_all(fd, dyn_byte_span(std::forward<Args>(args)...));
}

template <typename... Args>
auto recvfrom(Fd const& fd, Args&&... args) -> std::pair<size_t, SockAddrLite>
{
    return recvfrom(fd, dyn_byte_span(std::forward<Args>(args)...));
}

} // namespace mpk::mix::wrap_c
