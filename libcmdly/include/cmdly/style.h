/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 * Copyright 2026 Sukanle
 *
 * SPDX-License-Identifier: BSD-3-Clause AND Apache-2.0
 */

#ifndef CMDLY_STYLE_H
#define CMDLY_STYLE_H

#include <cstdint>
#include <sstream>

#include <fmt/ostream.h>

namespace cmdly {

// RGB-color
struct RgbColor {
    static RgbColor Default;
    static RgbColor Black;
    static RgbColor Red;
    static RgbColor Green;
    static RgbColor Yellow;
    static RgbColor Blue;
    static RgbColor Magenta;
    static RgbColor Cyan;
    static RgbColor White;

    static constexpr std::string_view TERMINATOR = "\033[0m";

    uint8_t red;
    uint8_t green;
    uint8_t blue;
    bool is_default;

    explicit RgbColor()
        : red(0)
        , green(0)
        , blue(0)
        , is_default(true) {}

    explicit RgbColor(uint8_t red_, uint8_t green_, uint8_t blue_)
        : red(red_)
        , green(green_)
        , blue(blue_)
        , is_default(false) {}

    bool operator==(const RgbColor &o) const { return memcmp(this, &o, sizeof(RgbColor)); }

    bool operator!=(const RgbColor &color) const { return !(*this == color); }
}; /* End of struct RgbColor */

// 256-color mode
struct Color {
    static Color Default;
    static Color Black;
    static Color Red;
    static Color Green;
    static Color Yellow;
    static Color Blue;
    static Color Magenta;
    static Color Cyan;
    static Color White;

    static constexpr std::string_view TERMINATOR = "\033[0m";

    uint8_t code;
    bool is_default;

    explicit Color()
        : code(0)
        , is_default(true) {}

    explicit Color(uint8_t code_)
        : code(code_)
        , is_default(false) {}

    bool operator==(const Color &color) const { return code == color.code && is_default == color.is_default; }

    bool operator!=(const Color &color) const { return !(*this == color); }
}; /* End of struct Color */

enum class FontStyle : uint16_t {
    Default = 0,
    Normal = 1,
    Bold = 1 << 1,
    Faint = 1 << 2,
    Italic = 1 << 3,
    Underline = 1 << 4,
    Blink = 1 << 5,
    Invert = 1 << 7,
    Hide = 1 << 8,
    Strike = 1 << 9,
}; /* End of enum FontStyle */

constexpr size_t FONT_STYLE_NUM = 10;

inline uint16_t operator&(const FontStyle &s1, const FontStyle &s2) {
    return static_cast<uint16_t>(s1) & static_cast<uint16_t>(s2);
}

inline FontStyle operator|(const FontStyle &s1, const FontStyle &s2) {
    return static_cast<FontStyle>(static_cast<uint16_t>(s1) | static_cast<uint16_t>(s2));
}

class ColorConverter {
public:
    static std::string fontStyleToCodeSequence(const FontStyle &font_style) {
        fmt::memory_buffer buf;
        for (uint32_t i = 1; i < FONT_STYLE_NUM; ++i)
            if (static_cast<uint16_t>(font_style) & (1 << i)) fmt::format_to(fmt::appender(buf), "\033[{}m", i);

        return fmt::to_string(buf);
    }

    static std::string cursorRgbColorToCodeSequence(const RgbColor &color) {
        if (color.is_default) return "\033]112\007";

        return fmt::format("\033]12;#{:02x}{:02x}{:02x}\033\\", color.red, color.green, color.blue);
    }

    static std::string foregroundTextColorToCodeSequence(const Color &color) { return colorToCodeSequence(color, 38); }

    static std::string backgroundTextColorToCodeSequence(const Color &color) { return colorToCodeSequence(color, 48); }

private:
    static std::string colorToCodeSequence(const Color &color, uint32_t code) {
        if (color.is_default) return "";

        return fmt::format("\033[{};5;{}m", code, color.code);
    }

    static std::string rgbColorToCodeSequence(const RgbColor &color, uint32_t code) {
        if (color.is_default) return "";

        return fmt::format("\033[{};2;{};{};{}m", code, color.red, color.green, color.blue);
    }
};

class TextStyle {
public:
    static TextStyle Default;

    explicit TextStyle(Color foreground_color = Color::Default, Color background_color = Color::Default,
        FontStyle font_style = FontStyle::Default)
        : foreground_color_(foreground_color)
        , background_color_(background_color)
        , font_style_(font_style) {
        code_sequence_ = convertToCodeSequence();
    }

    [[nodiscard]] std::string str() const { return code_sequence_; }

    bool operator==(const TextStyle &text_style) const { return code_sequence_ == text_style.code_sequence_; }

    bool operator!=(const TextStyle &text_style) const { return !(*this == text_style); }

    friend std::ostream &operator<<(std::ostream &stream, const TextStyle &text_style) {
        stream << text_style.code_sequence_;
        return stream;
    }

private:
    Color foreground_color_;
    Color background_color_;
    FontStyle font_style_;
    std::string code_sequence_;

    std::string convertToCodeSequence() {
        if (foreground_color_ == Color::Default
            && background_color_ == Color::Default
            && font_style_ == FontStyle::Default) {
            return "";
        }
        std::ostringstream oss;
        oss << ColorConverter::foregroundTextColorToCodeSequence(foreground_color_);
        oss << ColorConverter::backgroundTextColorToCodeSequence(background_color_);
        oss << ColorConverter::fontStyleToCodeSequence(font_style_);
        return oss.str();
    }
}; /* End of class TextStyle */

class CursorStyle {
public:
    static CursorStyle Default;

    explicit CursorStyle(const RgbColor &color = RgbColor::Default)
        : color_(color) {
        if (color_ == RgbColor::Default)
            code_sequence_ = "\033]112\007";
        else
            code_sequence_ = ColorConverter::cursorRgbColorToCodeSequence(color);
    }

    [[nodiscard]] std::string str() const { return code_sequence_; }

    friend std::ostream &operator<<(std::ostream &stream, const CursorStyle &cursor_style) {
        stream << cursor_style.code_sequence_;
        return stream;
    }

private:
    RgbColor color_;
    std::string code_sequence_;
}; /* End of class CursorStyle */

} /* End of namespace cmdly */

template<>
struct fmt::formatter<cmdly::CursorStyle> : fmt::ostream_formatter {};
template<>
struct fmt::formatter<cmdly::TextStyle> : fmt::ostream_formatter {};

#endif /* !CMDLY_STYLE_H */
