/** @file
 * @brief POSIX socket/socketpair/connect/bind/listen/accept wrappers
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/func_ref/tags.hpp"
#include "mpk/mix/types/byte_span.hpp"
#include "mpk/mix/wrap_c/sock_addr.hpp"

#include <sys/types.h>

#include <array>
#include <cstddef>
#include <utility>

namespace mpk::mix::wrap_c {

class Socket;

using SocketPair = std::array<Socket, 2>;

class Socket final
{
public:
    using ByteSpan = mpk::mix::ByteSpan<>;
    using ConstByteSpan = mpk::mix::ConstByteSpan<>;

    Socket() = default;
    ~Socket();

    Socket(Socket&&) noexcept;
    Socket(const Socket&) = delete;

    auto operator=(Socket&&) noexcept -> Socket&;
    auto operator=(const Socket&) -> Socket& = delete;

    auto fd() const -> int;

    auto attach(Unsafe_Tag, int) -> void;

    auto detach(Unsafe_Tag) -> int;

    auto shutdown(int how) -> void;

    auto try_shutdown(int how) -> bool;

    auto close() -> void;

    auto empty() const noexcept -> bool;

    auto dup() const -> Socket;

    auto try_read(ByteSpan buf) noexcept -> ssize_t;

    auto read(ByteSpan buf) -> size_t;

    auto read_all(ByteSpan buf) -> void;

    auto write(ConstByteSpan buf) -> size_t;

    auto write_all(ConstByteSpan buf) -> void;

    auto setblocking(bool blocking) -> void;

    auto setsockopt(int level, int optname, int optval) -> void;

    auto connect(const SockAddrLite& addr) -> void;

    auto bind(const SockAddrLite& addr) -> void;

    auto listen(int backlog = 128) -> void;

    auto accept() -> std::pair<Socket, SockAddrLite>;

    auto addr() const -> SockAddrLite;

    auto recvfrom(ByteSpan buf) -> std::pair<size_t, SockAddrLite>;

    auto sendto(ConstByteSpan buf, const SockAddrLite& addr) -> size_t;

    auto read_size() -> size_t;

    auto try_read_size() -> std::pair<size_t, int>;

    template <typename... Args>
    auto try_read(Args&&... args) noexcept -> ssize_t
    {
        return try_read(mpk::mix::dyn_byte_span(std::forward<Args>(args)...));
    }

    template <typename... Args>
    auto read(Args&&... args) -> size_t
    {
        return read(mpk::mix::dyn_byte_span(std::forward<Args>(args)...));
    }

    template <typename... Args>
    auto read_all(Args&&... args) -> void
    {
        read_all(mpk::mix::dyn_byte_span(std::forward<Args>(args)...));
    }

    auto write(ByteSpan buf) -> size_t { return write(ConstByteSpan{buf}); }

    template <typename... Args>
    auto write(Args&&... args) -> size_t
    {
        return write(mpk::mix::dyn_byte_span(std::forward<Args>(args)...));
    }

    auto write_all(ByteSpan buf) -> void { write_all(ConstByteSpan{buf}); }

    template <typename... Args>
    auto write_all(Args&&... args) -> void
    {
        write_all(mpk::mix::dyn_byte_span(std::forward<Args>(args)...));
    }

    template <typename... Args>
    auto send(Args&&... args) -> void
    {
        write_all(mpk::mix::dyn_byte_span(std::forward<Args>(args)...));
    }

    template <typename... Args>
    auto recvfrom(Args&&... args) -> std::pair<size_t, SockAddrLite>
    {
        return recvfrom(mpk::mix::dyn_byte_span(std::forward<Args>(args)...));
    }

    static auto socket(int domain, int type, int protocol) -> Socket;

    static auto socketpair(int domain, int type, int protocol) -> SocketPair;

private:
    Socket(Unsafe_Tag, int fd);

    int fd_{};
};

class AttachedSocket
{
public:
    explicit AttachedSocket(int fd) { sock_.attach(Unsafe, fd); }

    ~AttachedSocket() { sock_.detach(Unsafe); }

    AttachedSocket(AttachedSocket&&) = delete;
    auto operator=(AttachedSocket&&) -> AttachedSocket& = delete;

    operator Socket&() noexcept { return sock_; }

    auto operator->() noexcept -> Socket* { return &sock_; }

private:
    Socket sock_;
};

} // namespace mpk::mix::wrap_c
