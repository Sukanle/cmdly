/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 * Copyright 2026 Sukanle
 *
 * SPDX-License-Identifier: BSD-3-Clause AND Apache-2.0
 */

#include <cmdly/terminal.h>
#include <fmt/format.h>

using namespace cmdly;

Terminal::Terminal(const std::shared_ptr<IO> &io, const std::shared_ptr<History> &history,
    const std::shared_ptr<Completion> &completion)
    : _io(io)
    , _history(history)
    , _completion(completion) {
    registerDefaultKeyListeners();
    registerDefaultLineEnteredListeners();
}

const std::shared_ptr<IO> &Terminal::io() { return _io; }

const std::shared_ptr<History> &Terminal::history() { return _history; }

const std::shared_ptr<Completion> &Terminal::completion() { return _completion; }

void Terminal::onKeyPressed(const Key &key, KeyPressed::FunctionType handler) {
    addKeyPressed(key, std::make_shared<KeyPressedFnWrap>(std::move(handler)));
}

void Terminal::addKeyPressed(const Key &key, const std::shared_ptr<KeyPressed> &listener) {
    _key_pressed_listeners[key].push_back(listener);
}

void Terminal::removeKeyPressed(const Key &key) {
#if __cplusplus >= 202'002L
    if (!_key_pressed_listeners.contains(key)) return;
#else
    if (_key_pressed_listeners.find(key) == _key_pressed_listeners.end()) return;
#endif
    _key_pressed_listeners[key].clear();
}

void Terminal::onLineChanged(LineChanged::FunctionType handler) {
    addLineChanged(std::make_shared<LineChangedFnWrap>(std::move(handler)));
}

void Terminal::addLineChanged(const std::shared_ptr<LineChanged> &listener) {
    _line_changed_listeners.push_back(listener);
}

void Terminal::removeLineChanged() { _line_changed_listeners.clear(); }

void Terminal::onLineEntered(LineEntered::FunctionType handler) {
    addLineEntered(std::make_shared<LineEnteredFnWrap>(std::move(handler)));
}

void Terminal::addLineEntered(const std::shared_ptr<LineEntered> &listener) {
    _line_entered_listeners.push_back(listener);
}

void Terminal::removeLineEntered() { _line_entered_listeners.clear(); }

void Terminal::setPromptStyle(const TextStyle &style) { _prompt_style = style; }

const TextStyle &Terminal::getPromptStyle() const { return _prompt_style; }

void Terminal::setLineStyle(const TextStyle &style) { _line_style = style; }

const TextStyle &Terminal::getLineStyle() const { return _line_style; }

void Terminal::setCursorStyle(const CursorStyle &style) {
    _cursor_style = style;
    *_io << _cursor_style.str();
}

const CursorStyle &Terminal::getCursorStyle() { return _cursor_style; }

void Terminal::resetStyle() {
    setPromptStyle(TextStyle::Default);
    setLineStyle(TextStyle::Default);
    setCursorStyle(CursorStyle::Default);
}

void Terminal::run(const std::string &prompt) {
    while (true) {
        auto content = readLine(prompt);

        auto start = content.find_first_not_of(" \t\n\r\f\v");
        if (start == std::string_view::npos)
            content = {};
        else {
            auto end = content.find_last_not_of(" \t\n\r\f\v");
            content = content.substr(start, end - start + 1);
        }

        auto cmd_status = handleLineEntered(content);
        if (cmd_status == LineEntered::Status::CONTINUE) continue;
        if (cmd_status == LineEntered::Status::BREAK) break;
    }
}

std::string Terminal::readLine(const std::string &prompt) {
    Line line(prompt, _prompt_style, _line_style, _io);
    Cursor cursor(line, _io);
    std::string content;

    for (;;) {
        auto key = _io->getKey();

        auto key_status = handleKeyPressed(key, line, cursor);
        if (key_status == KeyPressed::Status::CONTINUE) continue;
        if (key_status == KeyPressed::Status::BREAK) break;

        if (key.isPrintable()) {
            cursor.putChar(key.code());
            if (_history->isManipulated()) _history->rewind();
        }

        if (content == line.content()) continue;

        content = line.content();
        auto line_status = handleLineChanged(content, line, cursor);
        if (line_status == LineChanged::Status::CONTINUE) continue;
        if (line_status == LineChanged::Status::BREAK) break;
    }

    content = line.content();
    _history->insert(content);
    _history->rewind();

    return content;
}

void Terminal::writeText(const std::string &text, const TextStyle &text_style) {
    if (text_style == TextStyle::Default) {
        *_io << text;
        return;
    }
    *_io << text_style.str() << text << "\033[m";
}

void Terminal::bell() { *_io << "\a"; }

void Terminal::clearCurrentLine() { *_io << "\033[K"; }

Terminal::Size Terminal::getSize() {
    Terminal::Size terminal_size{};
    _io->getWindowSize(&terminal_size.cols, &terminal_size.rows);
    return terminal_size;
}

void Terminal::registerDefaultKeyListeners() {
    onKeyPressed(Key::Enter, [](const Key &, Line &, Cursor &, Terminal &terminal) {
        terminal.writeText("\n");
        return KeyPressed::Status::BREAK;
    });

    onKeyPressed(Key::Home, [](const Key &, Line &, Cursor &cursor, Terminal &) {
        cursor.moveToHome();
        return KeyPressed::Status::CONTINUE;
    });

    onKeyPressed(Key::End, [](const Key &, Line &, Cursor &cursor, Terminal &) {
        cursor.moveToEnd();
        return KeyPressed::Status::CONTINUE;
    });

    onKeyPressed(Key::ArrowLeft, [](const Key &, Line &, Cursor &cursor, Terminal &) {
        cursor.moveLeft();
        return KeyPressed::Status::CONTINUE;
    });

    onKeyPressed(Key::ArrowRight, [](const Key &, Line &, Cursor &cursor, Terminal &) {
        cursor.moveRight();
        return KeyPressed::Status::CONTINUE;
    });

    onKeyPressed(Key::ArrowUp, [](const Key &, Line &line, Cursor &cursor, Terminal &terminal) {
        auto history = terminal.history();
        if (!history->isManipulated()) {
            history->setTopLine(line.content());
        }
        auto content = history->next();
        line.setContent(content);
        line.update();
        cursor.moveToEnd();
        return KeyPressed::Status::CONTINUE;
    });

    onKeyPressed(Key::ArrowDown, [](const Key &, Line &line, Cursor &cursor, Terminal &terminal) {
        if (terminal.history()->isManipulated()) {
            auto content = terminal.history()->prev();
            line.setContent(content);
            line.update();
            cursor.moveToEnd();
        }
        return KeyPressed::Status::CONTINUE;
    });

    onKeyPressed(Key::Backspace, [](const Key &, Line &, Cursor &cursor, Terminal &) {
        cursor.eatChar();
        return KeyPressed::Status::OK;
    });

    auto exit_listener_handler = [](const Key &, Line &line, Cursor &, Terminal &terminal) {
        line.setContent("exit");
        terminal.writeText("\n");
        return KeyPressed::Status::BREAK;
    };
    onKeyPressed(Key::Ctrl('c'), exit_listener_handler);
    onKeyPressed(Key::Ctrl('d'), exit_listener_handler);
    addKeyPressed(Key::Tab, _completion);
}

void Terminal::registerDefaultLineEnteredListeners() {
    onLineEntered([](const std::string &content, Terminal &terminal) {
        if (content == "exit" || content == "quit") {
            terminal.writeText("bye!\n");
            return LineEntered::Status::BREAK;
        }

        if (content == "history") {
            uint32_t i = 0;
            for (const auto &line : terminal.history()->lines()) {
                if (i == 0) {
                    i++;
                    continue;
                }
                terminal.writeText(fmt::format("[{}] {}\n", i++, line));
            }
            return LineEntered::Status::CONTINUE;
        }

        return LineEntered::Status::OK;
    });

    _completion->insert({"exit", "history"});
}

KeyPressed::Status Terminal::handleKeyPressed(const Key &key, Line &line, Cursor &cursor) {

#if __cplusplus >= 202'002L
    if (_key_pressed_listeners.contains(key)) {
#else
    if (_key_pressed_listeners.find(key) != _key_pressed_listeners.end()) {
#endif
        for (auto &listener : _key_pressed_listeners[key]) {
            auto status = listener->invoke(key, line, cursor, *this);
            if (status != KeyPressed::Status::OK) return status;
        }
    }

#if __cplusplus >= 202'002L
    if (_key_pressed_listeners.contains(Key::Any)) {
#else
    if (_key_pressed_listeners.find(Key::Any) != _key_pressed_listeners.end()) {
#endif
        for (auto &listener : _key_pressed_listeners[Key::Any]) {
            auto status = listener->invoke(key, line, cursor, *this);
            if (status != KeyPressed::Status::OK) return status;
        }
    }

    return KeyPressed::Status::OK;
}

LineChanged::Status Terminal::handleLineChanged(const std::string &content, Line &line, Cursor &cursor) {
    for (auto &listener : _line_changed_listeners) {
        auto status = listener->invoke(content, line, cursor, *this);
        if (status != LineChanged::Status::OK) return status;
    }

    return LineChanged::Status::OK;
}

LineEntered::Status Terminal::handleLineEntered(const std::string &content) {
    for (auto &listener : _line_entered_listeners) {
        auto status = listener->invoke(content, *this);
        if (status != LineEntered::Status::OK) return status;
    }
    return LineEntered::Status::OK;
}
