/** @file
 * @brief Extract the what() message from an exception or nested exceptions
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/detail/format.hpp"

#include <exception>
#include <string>

namespace mpk::mix
{

inline auto exception_message(std::exception_ptr error) -> std::string
{
    if (!error)
        return {};

    try
    {
        std::rethrow_exception(error);
    }
    catch (std::exception& e)
    {
        return mpk::mix::format("{}", e.what());
    }
    catch (...)
    {
        return "Unknown error";
    }
}

} // namespace mpk::mix
