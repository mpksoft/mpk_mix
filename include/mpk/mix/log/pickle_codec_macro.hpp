/** @file
 * @brief MPKMIX_LOG_PICKLE_CODEC macro for registering pickle-encoded quill codecs
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/log/defs.hpp"

#if MPKMIX_LOG_ENGINE == MPKMIX_LOG_NONE || MPKMIX_LOG_ENGINE == MPKMIX_LOG_CERR

#define MPKMIX_LOG_PICKLE_CODEC(T, ...) static_assert(true)

#elif MPKMIX_LOG_ENGINE == MPKMIX_LOG_QUILL

#include "quill/DeferredFormatCodec.h"

namespace mpk::mix::detail {

template <typename T>
concept NotVoid = !std::same_as<T, void>;

template <typename T>
concept DefaultPickleableType = requires(T const& value) {
    { pickle(value) } -> NotVoid;
};

template <DefaultPickleableType T>
struct DefaultPickleMaker
{
    auto operator()(T const& value) const
    {
        return pickle(value);
    }
};

template <typename T, typename PickleMaker = DefaultPickleMaker<T>>
struct PickleCodec
{
    static constexpr PickleMaker pickle{};
    using Pickle = decltype(pickle(std::declval<T>()));
    using InnerCodec = quill::DeferredFormatCodec<Pickle>;

    static size_t compute_encoded_size(
        quill::detail::SizeCacheVector& cache, T const& value) noexcept
    {
        return InnerCodec::compute_encoded_size(cache, pickle(value));
    }

    static void encode(
        std::byte*& buffer,
        quill::detail::SizeCacheVector const& cache,
        uint32_t& index,
        T const& value) noexcept
    {
        return InnerCodec::encode(buffer, cache, index, pickle(value));
    }

    static Pickle decode_arg(std::byte*& buffer)
    {
        return InnerCodec::decode_arg(buffer);
    }

    static void decode_and_store_arg(
        std::byte*& buffer, quill::DynamicFormatArgStore* args_store)
    {
        args_store->push_back(decode_arg(buffer));
    }
};

} // namespace mpk::mix::detail

/** MPKMIX_LOG_PICKLE_CODEC(T[, PickleMaker]) specializes quill::Codec<T> via
 *  PickleCodec, which calls pickle(value) on the hot path and formats the
 *  result using fmtquill::formatter for the pickle type.
 *  Expand in the global namespace. */
#define MPKMIX_LOG_PICKLE_CODEC(T, ...)                                    \
    template <>                                                            \
    struct quill::Codec<T>                                                 \
      : mpk::mix::detail::PickleCodec<T, ##__VA_ARGS__>                    \
    {                                                                      \
    }

#endif
