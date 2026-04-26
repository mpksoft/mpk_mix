/** @file
 * @brief StrongGrouped collection partitioned by a strong key type
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/strong/fwd.hpp"
#include "mpk/mix/strong/span.hpp"
#include "mpk/mix/util/format_seq.hpp"
#include "mpk/mix/util/format_streamable.hpp"
#include "mpk/mix/util/index_range.hpp"

#include <cassert>
#include <cstdint>
#include <limits>
#include <ostream>
#include <span>
#include <string_view>
#include <vector>


namespace mpk::mix {

template <typename T>
struct Grouped
{
    using value_type = T;

    std::vector<T>          values;
    std::vector<uint32_t>   groups{0};

    auto operator==(const Grouped&) const noexcept -> bool = default;
};

template <typename T, typename V>
requires std::convertible_to<V, T>
auto add_to_last_group(Grouped<T>& grouped, V&& value)
    -> void
{ grouped.values.emplace_back(std::forward<V>(value)); }

template <typename T>
auto next_group(Grouped<T>& grouped)
    -> void
{ grouped.groups.push_back(grouped.values.size()); }

template <typename T>
auto group_count(const Grouped<T>& grouped)
    -> uint32_t
{
    assert(!grouped.groups.empty());
    return grouped.groups.size() - 1;
}

template <typename T>
auto group_indices(const Grouped<T>& grouped)
    -> IndexRange<uint32_t>
{ return index_range(group_count(grouped)); }

template <typename T>
auto group(const Grouped<T>& grouped, uint32_t igroup)
    -> std::span<const T>
{
    assert(igroup + 1 < grouped.groups.size());
    const auto* indices = grouped.groups.data() + igroup;
    const auto* data = grouped.values.data();
    return { data+indices[0], data+indices[1] };
}

template <typename T>
auto group(Grouped<T>& grouped, uint32_t igroup)
    -> std::span<T>
{
    assert(igroup + 1 < grouped.groups.size());
    auto* indices = grouped.groups.data() + igroup;
    auto* data = grouped.values.data();
    return { data+indices[0], data+indices[1] };
}

template <typename T>
auto operator<<(std::ostream& s, const Grouped<T>& grouped)
    -> std::ostream&
{
    std::string_view delim = "";
    s << '[';
    for (uint32_t ig=0, ng=group_count(grouped); ig<ng; ++ig)
    {
        auto g = group(grouped, ig);
        s << delim << '(' << format_seq(g) << ')';
        delim = ", ";
    }
    s << ']';
    return s;
}


template <typename T, StrongIndexType OI, StrongIndexType II>
struct StrongGrouped
{
    using OuterIndex = OI;
    using OuterCount = typename OI::StrongDiff;
    using InnerIndex = II;
    using InnerCount = typename II::StrongDiff;
    using Weak = Grouped<T>;
    using value_type = T;

    Weak v;
};

template <typename>
constexpr inline auto is_strong_grouped_v = false;

template <typename T, StrongIndexType OI, StrongIndexType II>
constexpr inline auto is_strong_grouped_v<StrongGrouped<T, OI, II>> = true;

template <typename T>
concept StrongGroupedType =
    is_strong_grouped_v<T>;

template <StrongGroupedType SG, typename V>
requires std::convertible_to<V, typename SG::value_type>
auto add_to_last_group(SG& grouped, V&& value)
    -> void
{ add_to_last_group(grouped.v, std::forward<V>(value)); }

template <StrongGroupedType SG>
auto next_group(SG& grouped)
    -> void
{ next_group(grouped.v); }

template <StrongGroupedType SG>
auto group_count(const SG& grouped)
    -> typename SG::OuterCount
{
    auto raw = group_count(grouped.v);
    assert(raw <= std::numeric_limits<typename SG::OuterCount::Weak>::max());
    return typename SG::OuterCount(raw);
}

template <StrongGroupedType SG>
auto group_indices(const SG& grouped)
    -> IndexRange<typename SG::OuterIndex>
{
    using OuterIndex = typename SG::OuterIndex;
    using OuterCount = typename SG::OuterCount;
    auto raw = group_count(grouped.v);
    assert(raw <= std::numeric_limits<typename OuterCount::Weak>::max());
    return index_range<OuterIndex>(OuterCount(raw));
}

template <StrongGroupedType SG>
auto group(const SG& grouped, typename SG::OuterIndex igroup)
    -> StrongSpan<const typename SG::value_type, typename SG::InnerIndex>
{
    return StrongSpan<const typename SG::value_type, typename SG::InnerIndex>(
        group(grouped.v, igroup.v) );
}

template <StrongGroupedType SG>
auto group(SG& grouped, typename SG::OuterIndex igroup)
    -> StrongSpan<typename SG::value_type, typename SG::InnerIndex>
{
    return StrongSpan<typename SG::value_type, typename SG::InnerIndex>(
        group(grouped.v, igroup.v) );
}

template <StrongGroupedType SG>
auto operator<<(std::ostream& s, const SG& grouped)
    -> std::ostream&
{ return s << grouped.v; }

} // namespace mpk::mix


template <mpk::mix::StrongGroupedType SG>
struct MPKMIX_FORMAT_NS::formatter<SG> final : mpk::mix::OstreamFormatter<SG>
{};
