/** @file
 * @brief Overloads{} helper for constructing multi-lambda std::visit visitors
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

// https://github.com/ytsaurus/ytsaurus/blob/main/util/generic/overloaded.h

namespace mpk::mix
{

template <class... Fs>
struct Overloads : Fs...
{
    using Fs::operator()...;
};

template <class... Fs>
Overloads(Fs...) -> Overloads<Fs...>;

} // namespace mpk::mix
