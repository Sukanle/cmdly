/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#ifndef CMDLY_CURSOR_H
#define CMDLY_CURSOR_H

#include <stdint.h>
#include <memory>

#include "cmdly/exception.h"
#include "cmdly/line.h"
#include "cmdly/io.h"

namespace cmdly {

class Cursor {
public:
    explicit Cursor(Line &line, const std::shared_ptr<IO> &io)
        : _line(line)
        , _io(io) {
        init();
    }

    void init() {
        readPosition();
        _col = static_cast<uint32_t>(_line.prompt().length() + 1);
        updatePosition();
    }

    [[nodiscard]] uint32_t row() const { return _row; }

    [[nodiscard]] uint32_t col() const { return _col; }

    void moveLeft() {
        if (_col > (_line.prompt().length() + 1)) {
            _col--;
            updatePosition();
        }
    }

    void moveRight() {
        if (_col <= _line.length()) {
            _col++;
            updatePosition();
        }
    }

    void moveToHome() {
        _col = _line.prompt().length() + 1;
        updatePosition();
    }

    void moveToEnd() {
        _col = _line.length() + 1;
        updatePosition();
    }

    void putChar(char c) {
        _line.insert(static_cast<int>(_col - 1), c);
        _line.update();
        _col++;
        updatePosition();
    }

    void eatChar() {
        if (_col > (_line.prompt().length() + 1)) {
            _line.remove(static_cast<int>(_col - 2));
            _line.update();
            _col--;
            updatePosition();
        }
    }

private:
    Line &_line;
    std::shared_ptr<IO> _io;
    uint32_t _row = 0;
    uint32_t _col = 0;

    void readPosition() {
        *_io << "\033[6n";
        fflush(stdout);
        char data[32] = {0};
        for (size_t i = 0; i < sizeof(data) - 1; ++i) {
            data[i] = _io->getChar();
            if (data[i] < 0 || data[i] == 'R') break;
        }
        if (sscanf(data, "\033[%u;%uR", &_row, &_col) != 2) throw CursorError("could not read position");
    }

    void updatePosition() { *_io << "\033[" << _row << ";" << _col << "H"; }
}; /* End of class Cursor */

} /* End of namespace cmdly */

#endif /* !CMDLY_CURSOR_H */
