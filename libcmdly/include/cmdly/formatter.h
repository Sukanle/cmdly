/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 * Copyright 2026 Sukanle
 *
 * SPDX-License-Identifier: BSD-3-Clause AND Apache-2.0
 */

#ifndef CMDLY_FORMATTER_H
#define CMDLY_FORMATTER_H

#include <fmt/format.h>

#include <string>

namespace cmdly {

/**
 * Drop-in replacement for fmt::ostream_formatter for the cmdly types.
 *
 * fmt::ostream_formatter only appeared in fmt 9.0 and routes the value
 * through a temporary std::ostream. Every cmdly type that needs formatting
 * already renders itself through a str() member returning std::string, so
 * formatting that member directly gives the same bytes while keeping the
 * requirement at fmt 5.x.
 *
 * Usage:
 *   template<> struct fmt::formatter<cmdly::MyType> : cmdly::StringLikeFormatter<cmdly::MyType> {};
 */
template<typename T>
struct StringLikeFormatter : fmt::formatter<std::string> {
    template<typename FormatContext>
    auto format(const T &value, FormatContext &ctx) const {
        return fmt::formatter<std::string>::format(value.str(), ctx);
    }
}; /* End of struct StringLikeFormatter */

} /* End of namespace cmdly */

#endif /* !CMDLY_FORMATTER_H */