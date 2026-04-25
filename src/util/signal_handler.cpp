/** @file
 * @brief Implementation of POSIX signal handler
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/util/signal_handler.hpp"

#include "mpk/mix/util/throw.hpp"

#include <csignal>
#include <span>

namespace mpk::mix
{

namespace
{

auto set_default_signal_handlers(std::span<int const> signals) -> void
{
    for (auto signal : signals)
        std::signal(signal, SIG_DFL);
}

} // anonymous namespace

std::atomic<SignalHandler*> SignalHandler::the_instance_{};

SignalHandler::SignalHandler(
    std::stop_source* stop,
    std::string_view logger_name,
    std::initializer_list<int> signals)
{
    SignalHandler* expected = nullptr;
    if (!the_instance_.compare_exchange_strong(expected, this))
        throw_("SignalHandler construction failed: another instance already exists");

    if (logger_name.empty())
        logger_ = default_logger();
    else
        logger_ = get_logger(logger_name);

    stop_ = stop;
    signals_.assign(signals.begin(), signals.end());

    for (auto signal : signals_)
        std::signal(signal, &SignalHandler::signal_handler);
}

SignalHandler::~SignalHandler()
{
    set_default_signal_handlers(signals_);
    the_instance_.store(nullptr);
}

auto SignalHandler::signal_handler([[maybe_unused]] int signal) -> void
{
    auto* self = the_instance_.load();

    MPKMIX_LOG_INFO(
        self->logger_,
        "Received signal: {}. Requesting shutdown (Ctrl+C to terminate)...",
        signal);

    if (self->stop_)
        self->stop_->request_stop();

    set_default_signal_handlers(self->signals_);
}

} // namespace mpk::mix
