/** @file
 * @brief Implementation of POSIX read/write wrappers
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/wrap_c/io.hpp"

#include "mpk/mix/util/throw.hpp"
#include "mpk/mix/wrap_c/check_c_call.hpp"
#include "mpk/mix/wrap_c/zero_filled.hpp"

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
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

auto shutdown(Fd const& fd, int how) -> void
{
    assert(!fd.empty());
    MPKMIX_THROW_IF_MINUS_ONE(::shutdown(fd.fd(), how));
}

auto try_shutdown(Fd const& fd, int how) -> bool
{
    assert(!fd.empty());
    return MPKMIX_LOG_IF_MINUS_ONE(::shutdown(fd.fd(), how)) == 0;
}

auto shutdown_ignore_enotconn(Fd const& fd, int how) -> void
{
    assert(!fd.empty());
    if (::shutdown(fd.fd(), how) == -1)
    {
        auto err = errno;
        if (err == ENOTCONN)
            return;
        throw_("{}:{}: {}", __FILE__, __LINE__, strerror(errno));
    }
}

auto try_read(Fd const& fd, ByteSpan<> buf) noexcept -> ssize_t
{
    return ::read(fd.fd(), buf.data(), buf.size());
}

auto read(Fd const& fd, ByteSpan<> buf) -> ByteSpan<>
{
    auto bytes_read =
        MPKMIX_THROW_IF_MINUS_ONE(::read(fd.fd(), buf.data(), buf.size()));
    return buf.subspan(0, bytes_read);
}

auto read_all(Fd const& fd, ByteSpan<> buf) -> void
{
    auto read_total = size_t{};
    while (!buf.empty())
    {
        auto n_read = read(fd, buf).size();

        if (n_read == 0)
            throw_<std::runtime_error>(
                "Read {} of {} bytes and reached end of file",
                read_total,
                read_total + buf.size());

        buf = buf.subspan(n_read);
        read_total += n_read;
    }
}

auto write(Fd const& fd, ConstByteSpan<> buf) -> size_t
{
    auto result =
        MPKMIX_THROW_IF_MINUS_ONE(::write(fd.fd(), buf.data(), buf.size()));
    return result;
}

auto write_all(Fd const& fd, ConstByteSpan<> buf) -> void
{
    auto write_total = size_t{};
    while (!buf.empty())
    {
        auto n_written = write(fd, buf);
        if (n_written == 0)
            throw_<std::runtime_error>(
                "Wrote {} of {} bytes", write_total, write_total + buf.size());

        buf = buf.subspan(n_written);
        write_total += n_written;
    }
}

auto setblocking(Fd const& fd, bool blocking) -> void
{
    int flags = MPKMIX_THROW_IF_MINUS_ONE(fcntl(fd.fd(), F_GETFL, 0));
    if (!!(flags & O_NONBLOCK) == !blocking)
        return;
    flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    MPKMIX_THROW_IF_NONZERO(fcntl(fd.fd(), F_SETFL, flags));
}

template <typename T>
auto getsockopt(Fd const& fd, int level, int optname, Type_Tag<T>) -> T
{
    socklen_t optlen = sizeof(T);

    auto result = T{};
    MPKMIX_THROW_IF_MINUS_ONE(
        ::getsockopt(fd.fd(), level, optname, &result, &optlen));

    return result;
}

template auto getsockopt(Fd const&, int, int, Type_Tag<int>) -> int;

auto setsockopt(Fd const& fd, int level, int optname, int optval) -> void
{
    MPKMIX_THROW_IF_MINUS_ONE(
        ::setsockopt(fd.fd(), level, optname, &optval, sizeof(int)));
}

auto connect(Fd const& fd, SockAddrLite addr) -> void
{
    auto sock_addr = zero_filled<sockaddr_in>();
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = addr.host;
    sock_addr.sin_port = htons(addr.port);
    MPKMIX_THROW_IF_MINUS_ONE(
        ::connect(fd.fd(), sockaddr_ptr(sock_addr), sizeof(sock_addr)));
}

auto bind(Fd const& fd, SockAddrLite addr) -> void
{
    auto sock_addr = zero_filled<sockaddr_in>();
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = addr.host;
    sock_addr.sin_port = htons(addr.port);
    MPKMIX_THROW_IF_MINUS_ONE(
        ::bind(fd.fd(), sockaddr_ptr(sock_addr), sizeof(sock_addr)));
}

auto listen(Fd const& fd, int backlog) -> void
{
    MPKMIX_THROW_IF_MINUS_ONE(::listen(fd.fd(), backlog));
}

auto accept(Fd const& fd) -> std::pair<Fd, SockAddrLite>
{
    auto sock_addr = zero_filled<sockaddr_in>();
    socklen_t addrlen = sizeof(sockaddr_in);
    auto result_fd = MPKMIX_THROW_IF_MINUS_ONE(
        ::accept(fd.fd(), sockaddr_ptr(sock_addr), &addrlen));
    assert(result_fd > 0);

    auto result = Fd{Unsafe, result_fd};
    auto addr = addr_from_sockaddr_in(sock_addr, addrlen);

    return {std::move(result), std::move(addr)};
}

auto addr(Fd const& fd) -> SockAddrLite
{
    auto sock_addr = zero_filled<sockaddr_in>();
    socklen_t addrlen = sizeof(sockaddr_in);
    MPKMIX_THROW_IF_MINUS_ONE(
        ::getsockname(fd.fd(), sockaddr_ptr(sock_addr), &addrlen));
    return addr_from_sockaddr_in(sock_addr, addrlen);
}

auto recvfrom(Fd const& fd, ByteSpan<> buf) -> std::pair<size_t, SockAddrLite>
{
    auto sock_addr = zero_filled<sockaddr_in>();
    socklen_t addrlen = sizeof(sockaddr_in);
    auto received = MPKMIX_THROW_IF_MINUS_ONE(::recvfrom(
        fd.fd(),
        buf.data(),
        buf.size(),
        0,
        sockaddr_ptr(sock_addr),
        &addrlen));
    return {received, addr_from_sockaddr_in(sock_addr, addrlen)};
}

auto sendto(Fd const& fd, ConstByteSpan<> buf, SockAddrLite const& addr) -> size_t
{
    auto sock_addr = zero_filled<sockaddr_in>();
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = addr.host;
    sock_addr.sin_port = htons(addr.port);
    return MPKMIX_THROW_IF_MINUS_ONE(::sendto(
        fd.fd(),
        buf.data(),
        buf.size(),
        0,
        sockaddr_ptr(sock_addr),
        sizeof(sock_addr)));
}

auto read_size(Fd const& fd) -> size_t
{
    int size{};
    MPKMIX_THROW_IF_MINUS_ONE(ioctl(fd.fd(), FIONREAD, &size));
    return size;
}

auto try_read_size(Fd const& fd) -> std::pair<size_t, int>
{
    int size{};
    if (ioctl(fd.fd(), FIONREAD, &size) == -1)
        return {size, errno};

    return {size, 0};
}

auto socket(int domain, int type, int protocol) -> Fd
{
    auto fd = MPKMIX_THROW_IF_MINUS_ONE(::socket(domain, type, protocol));
    return {Unsafe, fd};
}

auto socketpair(int domain, int type, int protocol) -> std::array<Fd, 2>
{
    int fd[2];
    MPKMIX_THROW_IF_MINUS_ONE(::socketpair(domain, type, protocol, fd));
    return {Fd{Unsafe, fd[0]}, Fd{Unsafe, fd[1]}};
}

auto file(std::string const& path, int flags, mode_t mode) -> Fd
{
    auto fd = MPKMIX_THROW_IF_MINUS_ONE(::open(path.data(), flags, mode), path.data());
    return {Unsafe, fd};
}

auto ftruncate(Fd const& fd, off_t length) -> void
{
    MPKMIX_THROW_IF_MINUS_ONE(::ftruncate(fd.fd(), length));
}

auto mmap(
    void* ptr, size_t length, int prot, int flags, Fd const& fd, off_t offset)
    -> void*
{
    auto* result = ::mmap(ptr, length, prot, flags, fd.fd(), offset);
    if (result == MAP_FAILED)
        throw_("mmap failed: {}", strerror(errno));
    return result;
}

auto munmap(void* addr, size_t length) -> void
{
    MPKMIX_LOG_IF_MINUS_ONE(::munmap(addr, length));
}

} // namespace mpk::mix::wrap_c
