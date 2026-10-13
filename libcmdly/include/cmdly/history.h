/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 * Copyright 2026 Sukanle
 *
 * SPDX-License-Identifier: BSD-3-Clause AND Apache-2.0
 */

#ifndef CMDLY_HISTORY_H
#define CMDLY_HISTORY_H

#include <stdint.h>

#include <version>
#include <deque>
#include <string>
#include <fstream>
#include <filesystem>
#include <utility>
#include <vector>

#ifdef __cpp_lib_ranges
#  include <ranges>
#else
#  include <boost/range/adaptor/reversed.hpp>
#endif

namespace cmdly {

class History {
public:
    explicit History(size_t limit)
        : _limit(limit)
        , _index{-1} {}

    virtual ~History() = default;

    void insert(const std::string &line) {
        if (line.empty()) {
            return;
        }

        if (!_lines.empty() && line == _lines.at(0)) {
            return;
        }

        for (size_t i = 0; i < _lines.size(); ++i) {
            if (_lines[i] == line) {
                _lines.erase(_lines.begin() + static_cast<int>(i));
                _lines.push_front(line);
                return;
            }
        }

        _lines.push_front(line);
        if (_lines.size() > _limit) _lines.pop_back();
    }

    const std::deque<std::string> &lines() { return _lines; }

    size_t length() { return _lines.size(); }

    void clear() { _lines.clear(); }

    void rewind() { _index = -1; }

    [[nodiscard]] bool isManipulated() const { return _index > -1; }

    void setTopLine(const std::string &line) { _top_line = line; }

    const std::string &currentLine() {
        if (_index == -1) {
            return _top_line;
        }
        return _lines.at(_index);
    }

    const std::string &next() {
        if (_index < (static_cast<int>(_lines.size()) - 1)) _index++;
        return currentLine();
    }

    const std::string &prev() {
        if (_index > -1) _index--;
        return currentLine();
    }

    virtual void load() = 0;
    virtual void save() = 0;

protected:
    size_t _limit;
    int _index;
    std::string _top_line;
    std::deque<std::string> _lines;
}; /* End of class History */

class MemoryHistory : public History {
public:
    explicit MemoryHistory(size_t limit = 128)
        : History(limit) {}

    void load() override {}

    void save() override {}
}; /* End of class MemoryHistory */

class FileHistory : public History {
public:
    explicit FileHistory(std::filesystem::path file_path, size_t limit = 128)
        : History(limit)
        , _file_path(std::move(file_path)) {}

    void load() override {
        std::ifstream fp(_file_path);
        std::vector<std::string> tmp;

        for (std::string line; std::getline(fp, line);)
            tmp.push_back(line);


#ifdef __cpp_lib_ranges
        for (auto &it : tmp | std::views::reverse)
            insert(it);
#else
        for (auto &it : tmp | boost::adaptors::reversed) {
            insert(it);
        }
#endif
    }


    void save() override {
        std::ofstream fp(_file_path);
        for (auto &line : _lines)
            fp << line << "\n";
    }

private:
    std::filesystem::path _file_path;
}; /* End of class MemoryHistory */

} /* End of namespace cmdly */

#endif /* !CMDLY_HISTORY_H */
