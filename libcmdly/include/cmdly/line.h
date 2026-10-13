/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 * Copyright 2026 Sukanle
 *
 * SPDX-License-Identifier: BSD-3-Clause AND Apache-2.0
 */

#ifndef CMDLY_LINE_H
#define CMDLY_LINE_H

#include <memory>
#include <string>
#include <utility>

#include "cmdly/style.h"
#include "cmdly/io.h"

namespace cmdly {

class Line {
public:
    Line() = default;

    Line(const std::string &prompt, TextStyle prompt_style, TextStyle content_style, const std::shared_ptr<IO> &io)
        : _data(prompt)
        , _prompt(prompt)
        , _prompt_style(std::move(prompt_style))
        , _content_style(std::move(content_style))
        , _io(io) {
        update();
    }

    void setPromptStyle(const TextStyle &text_style) { _prompt_style = text_style; }

    void setContentStyle(const TextStyle &text_style) { _content_style = text_style; }

    const std::string &str() { return _data; }

    const std::string &prompt() { return _prompt; }

    std::string content() { return _data.substr(_prompt.size(), _data.size()); }

    size_t length() { return _data.size(); }

    void remove(int index) {
        if (index < static_cast<int>(_prompt.size()) || index >= static_cast<int>(_data.size())) return;
        _data.erase(_data.begin() + index);
    }

    void insert(int index, char c) {
        if (index < static_cast<int>(_prompt.size()) || index > static_cast<int>(_data.size())) return;
        _data.insert(_data.begin() + index, c);
    }

    void append(char c) { _data.push_back(c); }

    void append(const std::string &s) { _data += s; }

    void setContent(const std::string &content) { _data = _prompt + content; }

    void update() {
        *_io << "\r";
        *_io << _prompt_style.str() << _prompt << std::string(Color::TERMINATOR);
        *_io << _content_style.str() << content();
        if (_data.size() < _old_len)
            for (size_t i = 0; i < (_old_len - _data.size()); i++)
                *_io << " ";
        *_io << std::string(Color::TERMINATOR);
        _old_len = _data.size();
    }

protected:
    std::string _data;
    std::string _prompt;
    TextStyle _prompt_style;
    TextStyle _content_style;
    std::shared_ptr<IO> _io;
    size_t _old_len{0};
}; /* End of class Line */

} /* End of namespace cmdly */

#endif /* !CMDLY_LINE_H */
