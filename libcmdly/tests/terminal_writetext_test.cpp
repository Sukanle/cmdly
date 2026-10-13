/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 * Copyright 2026 Sukanle
 *
 * SPDX-License-Identifier: BSD-3-Clause AND Apache-2.0
 */

#include <gtest/gtest.h>
#include "cmdly/terminal.h"
#include "helpers/io_mock.h"

TEST(TerminalTest, checkWriteWithPlainTextWorksCorrectly) {
    auto io = std::make_shared<IOMock>();
    auto terminal = std::make_unique<cmdly::Terminal>(io);
    auto *io_mock = reinterpret_cast<IOMock *>(io.get());

    EXPECT_CALL(*io_mock, write("some text")).Times(1);

    terminal->writeText("some text");
}

TEST(TerminalTest, checkWriteWithCodeSequenceWorksCorrectly) {
    auto io = std::make_shared<IOMock>();
    auto terminal = std::make_unique<cmdly::Terminal>(io);
    auto *io_mock = reinterpret_cast<IOMock *>(io.get());

    EXPECT_CALL(*io_mock, write("\033[C")).Times(1);

    terminal->writeText(cmdly::Key::ArrowRight.str());
}

TEST(TerminalTest, checkWriteWithStyledTextWorksCorrectly) {
    auto io = std::make_shared<IOMock>();
    auto terminal = std::make_unique<cmdly::Terminal>(io);
    auto *io_mock = reinterpret_cast<IOMock *>(io.get());

    testing::Sequence seq;
    EXPECT_CALL(*io_mock, write("\033[38;5;1m")).InSequence(seq);
    EXPECT_CALL(*io_mock, write("some text")).InSequence(seq);
    EXPECT_CALL(*io_mock, write("\033[m")).InSequence(seq);

    terminal->writeText("some text", cmdly::TextStyle(cmdly::Color::Red));
}
