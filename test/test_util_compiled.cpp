/** @file
 * @brief TODO: Brief docstring.
 *
 * TODO: More documentation here
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/util/command_line_args.hpp"
#include "mpk/mix/util/scratch_dir.hpp"
#include "mpk/mix/util/signal_handler.hpp"

#include <gtest/gtest.h>

#include <csignal>
#include <filesystem>

namespace mpk::mix
{

TEST(ScratchDir, CreateAndDestroy)
{
    std::filesystem::path p;
    {
        auto dir = ScratchDir{std::string_view{"mpkmix_test"}};
        EXPECT_FALSE(dir.empty());
        p = dir.path();
        EXPECT_TRUE(std::filesystem::exists(p));
        EXPECT_TRUE(std::filesystem::is_directory(p));
    }
    EXPECT_FALSE(std::filesystem::exists(p));
}

TEST(ScratchDir, Detach)
{
    std::filesystem::path p;
    {
        auto dir = ScratchDir{std::string_view{"mpkmix_test"}};
        p = dir.path();
        dir.detach();
        EXPECT_TRUE(dir.empty());
    }
    EXPECT_TRUE(std::filesystem::exists(p));
    std::filesystem::remove(p);
}

TEST(ArgParser, BasicPositionalAndOption)
{
    std::string name;
    int count = 0;
    ArgParser parser{"Test parser"};
    parser.arg("name", name, "Name")
        .arg("--count,-c", count, 42, "Count");

    const char* argv[] = {"prog", "hello", "--count", "7"};
    parser.run(4, const_cast<char**>(argv));
    parser.validate();

    EXPECT_EQ(name, "hello");
    EXPECT_EQ(count, 7);
}

TEST(ArgParser, DefaultValues)
{
    std::string name;
    int count = 0;
    ArgParser parser{"Test parser"};
    parser.arg("name", name, "default_name", "Name")
        .arg("--count,-c", count, 5, "Count");

    const char* argv[] = {"prog"};
    parser.run(1, const_cast<char**>(argv));
    parser.validate();

    EXPECT_EQ(name, "default_name");
    EXPECT_EQ(count, 5);
}

TEST(ArgParser, BoolFlag)
{
    bool verbose = false;
    ArgParser parser{"Test"};
    parser.arg("--verbose,-v", verbose, verbose, "Verbose");

    const char* argv[] = {"prog", "-v"};
    parser.run(2, const_cast<char**>(argv));
    parser.validate();

    EXPECT_TRUE(verbose);
}

TEST(ArgParser, MissingMandatoryOption)
{
    std::string name;
    ArgParser parser{"Test"};
    parser.arg("--name", name, "Name");

    const char* argv[] = {"prog"};
    parser.run(1, const_cast<char**>(argv));
    EXPECT_THROW(parser.validate(), std::invalid_argument);
}

TEST(SignalHandler, InstallAndRemove)
{
    // Verify construction succeeds and destructor restores default handlers.
    {
        SignalHandler handler{SIGUSR1};
        // If we get here without throwing, the singleton was set up successfully.
        SUCCEED();
    }
    // After destruction, SIGUSR1 handler should be SIG_DFL again.
    EXPECT_EQ(std::signal(SIGUSR1, SIG_DFL), SIG_DFL);
}

TEST(SignalHandler, StopSourceIntegration)
{
    std::stop_source src;
    {
        SignalHandler handler{src, SIGUSR1};
        std::raise(SIGUSR1);
    }
    EXPECT_TRUE(src.stop_requested());
}

} // namespace mpk::mix
