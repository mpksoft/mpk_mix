/** @file
 * @brief POSIX signal handler registration and stop-source integration
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/log.hpp"

#include <atomic>
#include <concepts>
#include <csignal>
#include <stop_token>
#include <string_view>
#include <vector>

namespace mpk::mix
{

struct SignalHandler
{
public:
    template <std::same_as<int>... Ints>
    explicit SignalHandler(
        std::stop_source& stop, std::string_view logger_name, Ints... signals) :
      SignalHandler{&stop, logger_name, {signals...}}
    {
    }

    template <std::same_as<int>... Ints>
    explicit SignalHandler(std::stop_source& stop, Ints... signals) :
      SignalHandler{&stop, "", {signals...}}
    {
    }

    template <std::same_as<int>... Ints>
    explicit SignalHandler(std::string_view logger_name, Ints... signals) :
      SignalHandler{nullptr, logger_name, {signals...}}
    {
    }

    template <std::same_as<int>... Ints>
    explicit SignalHandler(Ints... signals) :
      SignalHandler{nullptr, "", {signals...}}
    {
    }

    ~SignalHandler();

private:
    SignalHandler(
        std::stop_source* stop,
        std::string_view logger_name,
        std::initializer_list<int> signals);

    static auto signal_handler(int signal) -> void;

    std::stop_source* stop_;
    Logger* logger_;
    std::vector<int> signals_;

    static std::atomic<SignalHandler*> the_instance_;
};

} // namespace mpk::mix
