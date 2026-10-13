/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 * Copyright 2026 Sukanle
 *
 * SPDX-License-Identifier: BSD-3-Clause AND Apache-2.0
 */

#include <deque>
#include <gtest/gtest.h>
#include "cmdly/terminal.h"
#include "helpers/io_mock.h"

class CustomIOMock : public IOMock {
public:
    std::deque<cmdly::Key> keys = {cmdly::Key('a'), cmdly::Key('b'), cmdly::Key('c'), cmdly::Key::Enter};
    std::deque<char> chars = {'\033', '[', '1', ';', '1', 'R'};

    cmdly::Key getKey() override {
        auto k = keys.front();
        keys.pop_front();
        return k;
    }

    char getChar() override {
        auto c = chars.front();
        chars.pop_front();
        return c;
    }
};

TEST(TerminalTest, checkReadLineReturnsValidCommand) {
    auto io = std::make_shared<CustomIOMock>();
    auto terminal = std::make_unique<cmdly::Terminal>(io);
    auto *io_mock = reinterpret_cast<CustomIOMock *>(io.get());

    EXPECT_CALL(*io_mock, write(::testing::_)).Times(::testing::AtLeast(1));

    auto line = terminal->readLine("prompt> ");
    EXPECT_EQ("abc", line);
}
