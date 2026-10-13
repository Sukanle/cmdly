/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#include <cmdly/completion.h>
#include <cmdly/cursor.h>
#include <cmdly/key.h>
#include <cmdly/line.h>
#include <cmdly/terminal.h>
#include <fmt/format.h>

using namespace cmdly;

Completion::Completion()
    : _key_tab_counter(0)
    , _longest_word_length(0) {}

void Completion::insert(const std::string &word) {
    _words.insert(word);
    if (word.size() > _longest_word_length) _longest_word_length = word.size();
}

void Completion::insert(std::initializer_list<std::string> words) {
    for (const auto &word : words)
        insert(word);
}

std::shared_ptr<Completion::SearchResult> Completion::search(const std::string &phrase) {
    // if nothing to search, then return all words we have
    if (phrase.empty()) {
        auto words = std::vector<std::string>(_words.begin(), _words.end());
        return std::make_shared<Completion::SearchResult>(words, _longest_word_length, "");
    }

    // make a search
    auto result = std::make_shared<SearchResult>();
    result->smallest_word_length = _longest_word_length;
    for (auto &word : _words) {
        if (word.starts_with(phrase)) {
            if (word.size() > result->longest_word_length) result->longest_word_length = word.size();
            if (word.size() < result->smallest_word_length) result->smallest_word_length = word.size();
            result->words.push_back(word);
        }
    }

    // if found 0 or 1 word, return result immediately
    if (result->size() < 2) return result;

    // find longest common prefix
    std::string first = result->words.front();
    std::string last = result->words.back();
    auto mismatch =
        std::mismatch(first.begin(), first.begin() + static_cast<ssize_t>(result->smallest_word_length), last.begin());
    result->longest_common_prefix = first.substr(0, mismatch.first - first.begin());

    return result;
}

Completion::Status Completion::invoke(const Key &key, Line &line, Cursor &cursor, Terminal &terminal) {
    if (key != Key::Tab) {
        _key_tab_counter = 0;
        return Status::OK;
    }

    auto phrase = line.content();
    _key_tab_counter++;
    if (_key_tab_counter == 1 && phrase.empty()) {
        terminal.bell();
        return Status::OK;
    }

    auto result = search(phrase);
    if (result->empty()) return Status::OK;

    if (result->size() == 1) {
        const std::string first_word = result->words.front();
        line.setContent(first_word);
    } else if (!result->longest_common_prefix.empty())
        line.setContent(result->longest_common_prefix);

    if (result->size() == 1)
        line.append(" ");
    else
        showSearchResult(terminal, result);

    _key_tab_counter = 0;
    line.update();
    cursor.moveToEnd();

    return Status::OK;
}

void Completion::showSearchResult(Terminal &terminal, std::shared_ptr<Completion::SearchResult> &result) {
    if (result->empty()) return;

    size_t line_size = 0;
    auto terminal_size = terminal.getSize();

    const size_t align = 2;
    const size_t word_max_length = result->longest_word_length + align;

    fmt::memory_buffer buf;
    auto out = std::back_inserter(buf);

    fmt::format_to(out, "\n");
    for (const auto &keyword : result->words) {
        if ((line_size + word_max_length) >= terminal_size.cols) {
            fmt::format_to(out, "\n");
            line_size = 0;
        }
        line_size += word_max_length;
        fmt::format_to(out, "{:<{}}", keyword, word_max_length);
    }
    fmt::format_to(out, "\n");

    terminal.writeText(fmt::to_string(buf));
}
