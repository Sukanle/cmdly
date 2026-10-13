/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 * Copyright 2026 Sukanle
 *
 * SPDX-License-Identifier: BSD-3-Clause AND Apache-2.0
 */

#ifndef CMDLY_COMPLETION_H
#define CMDLY_COMPLETION_H

#include <stdint.h>

#include <set>
#include <utility>
#include <initializer_list>

#include <cmdly/listener.h>

namespace cmdly {

class Terminal;

class Completion : public KeyPressed {
public:
    struct SearchResult {
        std::vector<std::string> words;
        size_t smallest_word_length = 0;
        size_t longest_word_length = 0;
        std::string longest_common_prefix;

        SearchResult() = default;

        explicit SearchResult(
            std::vector<std::string> &words_, size_t longest_word_length_, std::string longest_common_prefix_)
            : words(words_)
            , longest_word_length(longest_word_length_)
            , longest_common_prefix(std::move(longest_common_prefix_)) {}

        [[nodiscard]] size_t size() const { return words.size(); }

        [[nodiscard]] bool empty() const { return words.empty(); }
    }; /* End of SearchResult */

    Completion();
    void insert(const std::string &word);
    void insert(std::initializer_list<std::string> words);
    std::shared_ptr<Completion::SearchResult> search(const std::string &phrase);
    Completion::Status invoke(const Key &key, Line &line, Cursor &cursor, Terminal &terminal) override;

protected:
    std::set<std::string> _words;
    uint16_t _key_tab_counter;
    uint16_t _longest_word_length;

    static void showSearchResult(Terminal &terminal, std::shared_ptr<Completion::SearchResult> &result);
}; /* End of Completion */

} /* End of namespace cmdly */

#endif /* !CMDLY_COMPLETION_H */
