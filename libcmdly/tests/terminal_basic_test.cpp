/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 * Copyright 2026 Sukanle
 *
 * SPDX-License-Identifier: BSD-3-Clause AND Apache-2.0
 */

#include <gtest/gtest.h>
#include "cmdly/terminal.h"
#include "helpers/io_mock.h"

TEST(TerminalTest, checkConstructorWithNonDefaultArgumentIsOk) {
    auto io = std::make_shared<IOMock>();
    auto terminal = std::make_unique<cmdly::Terminal>(io);
    auto *io_mock = reinterpret_cast<IOMock *>(io.get());

    EXPECT_CALL(*io_mock, die()).Times(1);
}
