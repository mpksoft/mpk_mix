/** @file
 * @brief Format a sequence of items into a string, with separator.
 *
 * Copyright (C) 2025 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include <sstream>
#include <string>
#include <string_view>

namespace mpk::mix {

/** @brief Default element formatter: streams item via operator<<. */
struct DefaultFormatter
{
    template <typename T>
    auto operator()(std::ostream& s, const T& value) const -> void
    {
        s << value;
    }
};

/**
 * @brief Format a sequence of items into a string.
 *
 * @param seq            Any iterable sequence.
 * @param delim          Separator between items (default: ",").
 * @param element_formatter  Callable `(ostream&, item) -> void`. Defaults to streaming via operator<<.
 * @param head           Prefix before the sequence (default: "").
 * @param tail           Suffix after the sequence (default: "").
 */
template <typename Seq, typename ElementFormatter = DefaultFormatter>
auto format_seq(const Seq& seq,
                std::string_view delim = ",",
                const ElementFormatter& element_formatter = {},
                std::string_view head = {},
                std::string_view tail = {})
    -> std::string
{
    std::ostringstream s;
    s << head;
    auto current_delim = std::string_view{};
    for (const auto& item : seq)
    {
        s << current_delim;
        element_formatter(s, item);
        current_delim = delim;
    }
    s << tail;
    return s.str();
}

} // namespace mpk::mix
