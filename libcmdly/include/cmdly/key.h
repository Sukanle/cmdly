/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 * Copyright 2026 Sukanle
 *
 * SPDX-License-Identifier: BSD-3-Clause AND Apache-2.0
 */

#ifndef CMDLY_KEY_H
#define CMDLY_KEY_H

#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include <string>

#include <iterator>

#include <fmt/format.h>

#include <cmdly/formatter.h>

namespace cmdly {

class Key {
public:
    static Key Any;
    static Key Tab;
    static Key Enter;
    static Key Esc;
    static Key Backspace;
    static Key ArrowUp;
    static Key ArrowDown;
    static Key ArrowRight;
    static Key ArrowLeft;
    static Key End;
    static Key Home;
    static Key F1;
    static Key F2;
    static Key F3;
    static Key F4;

    static Key Ctrl(char c) noexcept { return Key(static_cast<char>(c - 'a' + 1)); }

    static Key Alt(char c) noexcept { return Key(fmt::format("\033[{}", static_cast<int>(c))); }

    explicit Key(char code) noexcept
        : _code(code)
        , _special(false) {}

    explicit Key(std::string sequence) noexcept
        : _code(0)
        , _special(true)
        , _sequence(std::move(sequence)) {
        if (_sequence[0] == '^') _sequence[0] = '\033';
    }

    [[nodiscard]] char code() const noexcept { return _code; }

    [[nodiscard]] std::string sequence() const noexcept { return _sequence; }

    [[nodiscard]] bool isSpecial() const noexcept { return _special; }

    [[nodiscard]] bool isPrintable() const noexcept { return !_special && std::isprint(_code); }

    [[nodiscard]] std::string str() const noexcept {
        if (_special) return _sequence;
        return std::to_string(_code);
    }


    [[nodiscard]] std::string info() const noexcept {
        fmt::memory_buffer buf;
        if (_special) {
            fmt::format_to(std::back_inserter(buf), R"(<Key sequence=")");
            for (unsigned char c : _sequence) {
                if (c == '\033')
                    fmt::format_to(std::back_inserter(buf), "^");
                else if (isprint(c))
                    fmt::format_to(std::back_inserter(buf), "{}", static_cast<char>(c));
                else
                    fmt::format_to(std::back_inserter(buf), "\\{}", static_cast<int>(c));
            }
            fmt::format_to(std::back_inserter(buf), "\">");
        } else if (isprint(static_cast<unsigned char>(_code)))
            fmt::format_to(std::back_inserter(buf), R"(<Key code="{}">)", _code);
        else
            fmt::format_to(std::back_inserter(buf), R"(<Key code="\{}">)", static_cast<int>(_code));
        return fmt::to_string(buf);
    }

    friend bool operator<(const Key &key1, const Key &key2) { return key1.str() < key2.str(); }

    friend bool operator==(const Key &key1, const Key &key2) {
        return (key1._code == key2._code) && (key1._special == key2._special) && (key1._sequence == key2._sequence);
    }

    friend bool operator!=(const Key &key1, const Key &key2) { return !(key1 == key2); }

    friend std::ostream &operator<<(std::ostream &out, const Key &key) {
        out << key.str();
        return out;
    }

private:
    char _code;
    bool _special;
    std::string _sequence;
}; /* End of class Key */

} /* End of namespace cmdly */

template<>
struct fmt::formatter<cmdly::Key> : cmdly::StringLikeFormatter<cmdly::Key> {};
#endif /* !CMDLY_KEY_H */
