/** @file
 * @brief Chrono time-point utilities and formatting helpers
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include <chrono>
#include <concepts>

namespace mpk::mix
{

template <typename T>
concept ChronoTimepointType =
    requires {
        typename T::clock;
        typename T::duration;
    }
    && std::same_as<
        T,
        std::chrono::time_point<typename T::clock, typename T::duration>>;

} // namespace mpk::mix
