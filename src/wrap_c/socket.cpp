/** @file
 * @brief Implementation of POSIX socket creation wrappers
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/wrap_c/socket.hpp"

#include "mpk/mix/util/throw.hpp"
#include "mpk/mix/wrap_c/check_c_call.hpp"
#include "mpk/mix/wrap_c/zero_filled.hpp"

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cassert>
#include <cerrno>
#include <cstring>
#include <utility>

namespace mpk::mix::wrap_c {

namespace {

auto addr_from_sockaddr_in(sockaddr_in& addr, socklen_t addrlen) -> SockAddrLite
{
    if (addrlen != sizeof(sockaddr_in))
        throw std::runtime_error("Unrecognized reply from `getsockname`");

    if (addr.sin_family != AF_INET)
        throw std::runtime_error("Socket family differs from AF_INET");

    return {addr.sin_addr.s_addr, ntohs(addr.sin_port)};
}

auto sockaddr_ptr(struct sockaddr_in& addr) -> struct sockaddr*
{
    return reinterpret_cast<struct sockaddr*>(&addr);
}

} // anonymous namespace


Socket::~Socket()
{
    close();
}

Socket::Socket(Unsafe_Tag, int fd)
    : fd_{fd}
{}

Socket::Socket(Socket&& that) noexcept
    : fd_{that.detach(Unsafe)}
{}

auto Socket::operator=(Socket&& that) noexcept -> Socket&
{
    if (&that != this)
    {
        close();
        fd_ = that.detach(Unsafe);
    }
    return *this;
}

auto Socket::fd() const -> int
{
    return fd_;
}

auto Socket::attach(Unsafe_Tag, int fd) -> void
{
    close();
    assert(fd == 0 || fd != fd_);
    fd_ = fd;
}

auto Socket::detach(Unsafe_Tag) -> int
{
    return std::exchange(fd_, 0);
}

auto Socket::shutdown(int how) -> void
{
    assert(fd_ != 0);
    MPKMIX_THROW_IF_MINUS_ONE(::shutdown(fd_, how));
}

auto Socket::try_shutdown(int how) -> bool
{
    assert(fd_ != 0);
    return MPKMIX_LOG_IF_MINUS_ONE(::shutdown(fd_, how)) == 0;
}

auto Socket::close() -> void
{
    if (fd_ == 0)
        return;

    MPKMIX_LOG_IF_NONZERO(::close(fd_));

    fd_ = 0;
}

auto Socket::empty() const noexcept -> bool
{
    return fd_ == 0;
}

auto Socket::dup() const -> Socket
{
    auto fd = MPKMIX_THROW_IF_MINUS_ONE(::dup(fd_));
    assert(fd != 0);
    auto result = Socket{};
    result.attach(Unsafe, fd);
    return result;
}

auto Socket::try_read(ByteSpan buf) noexcept -> ssize_t
{
    return ::read(fd_, buf.data(), buf.size());
}

auto Socket::read(ByteSpan buf) -> size_t
{
    return MPKMIX_THROW_IF_MINUS_ONE(::read(fd_, buf.data(), buf.size()));
}

auto Socket::read_all(ByteSpan buf) -> void
{
    auto read_total = size_t{};
    while (!buf.empty())
    {
        auto n_read = read(buf);

        if (n_read == 0)
            throw_<std::runtime_error>(
                "Read {} of {} bytes and reached end of file",
                read_total,
                read_total + buf.size());

        buf = buf.subspan(n_read);
        read_total += n_read;
    }
}

auto Socket::write(ConstByteSpan buf) -> size_t
{
    return MPKMIX_THROW_IF_MINUS_ONE(::write(fd_, buf.data(), buf.size()));
}

auto Socket::write_all(ConstByteSpan buf) -> void
{
    auto write_total = size_t{};
    while (!buf.empty())
    {
        auto n_written = write(buf);
        if (n_written == 0)
            throw_<std::runtime_error>(
                "Wrote {} of {} bytes", write_total, write_total + buf.size());

        buf = buf.subspan(n_written);
        write_total += n_written;
    }
}

auto Socket::setblocking(bool blocking) -> void
{
    int flags = MPKMIX_THROW_IF_MINUS_ONE(fcntl(fd_, F_GETFL, 0));
    if (!!(flags & O_NONBLOCK) == !blocking)
        return;
    flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    MPKMIX_THROW_IF_NONZERO(fcntl(fd_, F_SETFL, flags));
}

auto Socket::setsockopt(int level, int optname, int optval) -> void
{
    MPKMIX_THROW_IF_MINUS_ONE(
        ::setsockopt(fd_, level, optname, &optval, sizeof(int)));
}

auto Socket::connect(const SockAddrLite& addr) -> void
{
    auto sock_addr = zero_filled<sockaddr_in>();
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = addr.host;
    sock_addr.sin_port = htons(addr.port);
    MPKMIX_THROW_IF_MINUS_ONE(
        ::connect(fd_, sockaddr_ptr(sock_addr), sizeof(sock_addr)));
}

auto Socket::bind(const SockAddrLite& addr) -> void
{
    auto sock_addr = zero_filled<sockaddr_in>();
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = addr.host;
    sock_addr.sin_port = htons(addr.port);
    MPKMIX_THROW_IF_MINUS_ONE(
        ::bind(fd_, sockaddr_ptr(sock_addr), sizeof(sock_addr)));
}

auto Socket::listen(int backlog) -> void
{
    MPKMIX_THROW_IF_MINUS_ONE(::listen(fd_, backlog));
}

auto Socket::accept() -> std::pair<Socket, SockAddrLite>
{
    auto sock_addr = zero_filled<sockaddr_in>();
    socklen_t addrlen = sizeof(sockaddr_in);
    auto fd = MPKMIX_THROW_IF_MINUS_ONE(
        ::accept(fd_, sockaddr_ptr(sock_addr), &addrlen));
    assert(fd > 0);

    auto sock = Socket{};
    sock.attach(Unsafe, fd);

    auto addr = addr_from_sockaddr_in(sock_addr, addrlen);

    return {std::move(sock), std::move(addr)};
}

auto Socket::addr() const -> SockAddrLite
{
    auto sock_addr = zero_filled<sockaddr_in>();
    socklen_t addrlen = sizeof(sockaddr_in);
    MPKMIX_THROW_IF_MINUS_ONE(
        ::getsockname(fd_, sockaddr_ptr(sock_addr), &addrlen));
    return addr_from_sockaddr_in(sock_addr, addrlen);
}

auto Socket::recvfrom(ByteSpan buf) -> std::pair<size_t, SockAddrLite>
{
    auto sock_addr = zero_filled<sockaddr_in>();
    socklen_t addrlen = sizeof(sockaddr_in);
    auto received = MPKMIX_THROW_IF_MINUS_ONE(::recvfrom(
        fd_,
        buf.data(),
        buf.size(),
        0,
        sockaddr_ptr(sock_addr),
        &addrlen));
    return {received, addr_from_sockaddr_in(sock_addr, addrlen)};
}

auto Socket::sendto(ConstByteSpan buf, const SockAddrLite& addr) -> size_t
{
    auto sock_addr = zero_filled<sockaddr_in>();
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = addr.host;
    sock_addr.sin_port = htons(addr.port);
    return MPKMIX_THROW_IF_MINUS_ONE(::sendto(
        fd_,
        buf.data(),
        buf.size(),
        0,
        sockaddr_ptr(sock_addr),
        sizeof(sock_addr)));
}

auto Socket::read_size() -> size_t
{
    int size{};
    MPKMIX_THROW_IF_MINUS_ONE(ioctl(fd_, FIONREAD, &size));
    return size;
}

auto Socket::try_read_size() -> std::pair<size_t, int>
{
    int size{};
    if (ioctl(fd_, FIONREAD, &size) == -1)
        return {size, errno};

    return {size, 0};
}

auto Socket::socket(int domain, int type, int protocol) -> Socket
{
    auto fd = MPKMIX_THROW_IF_MINUS_ONE(::socket(domain, type, protocol));
    return {Unsafe, fd};
}

auto Socket::socketpair(int domain, int type, int protocol) -> SocketPair
{
    int fd[2];
    MPKMIX_THROW_IF_MINUS_ONE(::socketpair(domain, type, protocol, fd));
    return {Socket{Unsafe, fd[0]}, Socket{Unsafe, fd[1]}};
}

} // namespace mpk::mix::wrap_c
