/** @file
 * @brief TODO: Brief docstring.
 *
 * TODO: More documentation here
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/wrap_c/check_c_call.hpp"
#include "mpk/mix/wrap_c/fd.hpp"
#include "mpk/mix/wrap_c/host_addr.hpp"
#include "mpk/mix/wrap_c/io.hpp"
#include "mpk/mix/wrap_c/sock_addr.hpp"
#include "mpk/mix/wrap_c/socket.hpp"
#include "mpk/mix/wrap_c/zero_filled.hpp"

#include <gtest/gtest.h>

#include <sys/socket.h>

#include <cstring>
#include <sstream>
#include <string>
#include <string_view>

using namespace mpk::mix;
using namespace mpk::mix::wrap_c;
using namespace std::string_view_literals;

TEST(MpkMix_WrapC_ZeroFilled, Basic)
{
    struct Foo
    {
        int a;
        int b;
    };
    auto f = zero_filled<Foo>();
    EXPECT_EQ(f.a, 0);
    EXPECT_EQ(f.b, 0);
}

TEST(MpkMix_WrapC_CheckCCall, ThrowIfMinusOne_Throws)
{
    EXPECT_THROW(MPKMIX_THROW_IF_MINUS_ONE(-1), std::runtime_error);
}

TEST(MpkMix_WrapC_CheckCCall, ThrowIfMinusOne_PassThrough)
{
    EXPECT_EQ(MPKMIX_THROW_IF_MINUS_ONE(42), 42);
}

TEST(MpkMix_WrapC_CheckCCall, ThrowIfNonzero_Throws)
{
    EXPECT_THROW(MPKMIX_THROW_IF_NONZERO(1), std::runtime_error);
}

TEST(MpkMix_WrapC_CheckCCall, ThrowIfZero_Throws)
{
    EXPECT_THROW(MPKMIX_THROW_IF_ZERO(0), std::runtime_error);
}

TEST(MpkMix_WrapC_Fd, DefaultEmpty)
{
    Fd fd;
    EXPECT_TRUE(fd.empty());
}

TEST(MpkMix_WrapC_Fd, SocketpairLifecycle)
{
    auto fds = wrap_c::socketpair(AF_UNIX, SOCK_STREAM, 0);
    EXPECT_FALSE(fds[0].empty());
    EXPECT_FALSE(fds[1].empty());

    std::string_view msg = "hello";
    write_all(fds[0], msg);

    char buf[5];
    read_all(fds[1], buf);
    EXPECT_EQ(std::string_view(buf, 5), msg);
}

TEST(MpkMix_WrapC_HostAddr, FromString)
{
    auto h = HostAddr{"127.0.0.1"sv};
    EXPECT_FALSE(h.empty());
    EXPECT_EQ(h.as_string(), "127.0.0.1");
    EXPECT_NE(h.as_uint32(), 0u);
}

TEST(MpkMix_WrapC_HostAddr, FromUint32)
{
    auto h = HostAddr{0x0100007Fu};  // 127.0.0.1 in network byte order
    EXPECT_FALSE(h.empty());
}

TEST(MpkMix_WrapC_HostAddr, DefaultEmpty)
{
    HostAddr h;
    EXPECT_TRUE(h.empty());
}

TEST(MpkMix_WrapC_HostAddr, OstreamFormat)
{
    auto h = HostAddr{"127.0.0.1"sv};
    std::ostringstream oss;
    oss << h;
    EXPECT_EQ(oss.str(), "127.0.0.1");
}

TEST(MpkMix_WrapC_SockAddrLite, CompareAndHash)
{
    auto a = SockAddrLite{0x0100007Fu, 8080};
    auto b = SockAddrLite{0x0100007Fu, 8080};
    auto c = SockAddrLite{0x0100007Fu, 9090};

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_EQ(a.hash(), b.hash());

    SockAddrLite::Hash h;
    EXPECT_EQ(h(a), h(b));
}

TEST(MpkMix_WrapC_SockAddrLite, OstreamFormat)
{
    auto addr = SockAddr{HostAddr{"127.0.0.1"sv}, 8080};
    std::ostringstream oss;
    oss << addr;
    EXPECT_EQ(oss.str(), "127.0.0.1:8080");
}

TEST(MpkMix_WrapC_Socket, SocketpairReadWrite)
{
    auto socks = Socket::socketpair(AF_UNIX, SOCK_STREAM, 0);

    socks[0].write_all("hello"sv);

    char read_buf[5];
    socks[1].read_all(read_buf);
    EXPECT_EQ(std::string_view(read_buf, 5), "hello");
}

TEST(MpkMix_WrapC_Socket, LoopbackEcho)
{
    auto server = Socket::socket(AF_INET, SOCK_STREAM, 0);
    server.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1);

    // Bind to loopback on ephemeral port
    auto bind_addr = SockAddrLite{0x0100007Fu, 0};
    server.bind(bind_addr);
    server.listen();

    auto bound = server.addr();
    EXPECT_NE(bound.port, 0);

    auto client = Socket::socket(AF_INET, SOCK_STREAM, 0);
    client.connect(bound);

    auto [accepted, peer_addr] = server.accept();
    EXPECT_FALSE(accepted.empty());

    std::string_view msg = "echo";
    client.write_all(msg);

    char buf[4];
    accepted.read_all(buf);
    EXPECT_EQ(std::string_view(buf, 4), msg);
}
