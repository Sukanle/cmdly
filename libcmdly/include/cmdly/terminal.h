/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#ifndef CMDLY_TERMINAL_H
#define CMDLY_TERMINAL_H

#include <iostream>

#include <map>
#include <vector>
#include <memory>
#include <cmdly/style.h>
#include <cmdly/line.h>
#include <cmdly/cursor.h>
#include <cmdly/listener.h>
#include <cmdly/history.h>
#include <cmdly/completion.h>
#include <cmdly/io.h>

namespace cmdly {

class Terminal {
public:
    struct Size {
        std::size_t cols, rows;
    };
    explicit Terminal(const std::shared_ptr<IO> &io = std::make_shared<StandardIO>(),
        const std::shared_ptr<History> &history = std::make_shared<MemoryHistory>(),
        const std::shared_ptr<Completion> &completion = std::make_shared<Completion>());

    const std::shared_ptr<IO> &io();
    const std::shared_ptr<History> &history();
    const std::shared_ptr<Completion> &completion();

    void onKeyPressed(const Key &key, KeyPressed::FunctionType handler);
    void addKeyPressed(const Key &key, const std::shared_ptr<KeyPressed> &listener);
    void removeKeyPressed(const Key &key);

    void onLineChanged(LineChanged::FunctionType handler);
    void addLineChanged(const std::shared_ptr<LineChanged> &listener);
    void removeLineChanged();

    void onLineEntered(LineEntered::FunctionType handler);
    void addLineEntered(const std::shared_ptr<LineEntered> &listener);
    void removeLineEntered();

    void setPromptStyle(const TextStyle &style);
    void setLineStyle(const TextStyle &style);
    void setCursorStyle(const CursorStyle &style);

    [[nodiscard]] const TextStyle &getPromptStyle() const;
    [[nodiscard]] const TextStyle &getLineStyle() const;
    [[nodiscard]] const CursorStyle &getCursorStyle();

    void resetStyle();

    void run(const std::string &prompt);
    std::string readLine(const std::string &prompt);
    void writeText(const std::string &text, const TextStyle &text_style = TextStyle::Default);

    void bell();
    void clearCurrentLine();
    Terminal::Size getSize();

protected:
    std::shared_ptr<IO> _io;
    std::shared_ptr<History> _history;
    std::shared_ptr<Completion> _completion;
    std::map<Key, std::vector<std::shared_ptr<KeyPressed>>> _key_pressed_listeners;
    std::vector<std::shared_ptr<LineChanged>> _line_changed_listeners;
    std::vector<std::shared_ptr<LineEntered>> _line_entered_listeners;
    TextStyle _prompt_style;
    TextStyle _line_style;
    CursorStyle _cursor_style;

    void registerDefaultKeyListeners();
    void registerDefaultLineEnteredListeners();

    KeyPressed::Status handleKeyPressed(const Key &key, Line &line, Cursor &cursor);
    LineChanged::Status handleLineChanged(const std::string &content, Line &line, Cursor &cursor);
    LineEntered::Status handleLineEntered(const std::string &content);
}; /* End of class Terminal */

} /* End of namespace cmdly */

#endif /* !CMDLY_TERMINAL_H */
