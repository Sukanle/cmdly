/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 * Copyright 2026 Sukanle
 *
 * SPDX-License-Identifier: BSD-3-Clause AND Apache-2.0
 */

#include <gtest/gtest.h>
#include "cmdly/line.h"

TEST(LineTest, checkNewlyCreatedLineHasLengthZero) {
    cmdly::Line line;
    EXPECT_EQ(line.length(), 0);
    EXPECT_EQ(line.str(), "");
}

TEST(LineTest, checkAppendMethodWorksFine) {
    cmdly::Line line;
    line.append('a');
    EXPECT_EQ(line.str(), "a");
    EXPECT_EQ(line.length(), 1);
    line.append("abc");
    EXPECT_EQ(line.length(), 4);
    EXPECT_EQ(line.str(), "aabc");
}

TEST(LineTest, checkInsertMethodWorksFine) {
    cmdly::Line line;
    line.append("simple");
    EXPECT_EQ(line.length(), 6);
    line.insert(0, '$');
    EXPECT_EQ(line.length(), 7);
    EXPECT_EQ(line.str(), "$simple");
    line.insert(4, '$');
    EXPECT_EQ(line.length(), 8);
    EXPECT_EQ(line.str(), "$sim$ple");
    line.insert(8, '$');
    EXPECT_EQ(line.length(), 9);
    EXPECT_EQ(line.str(), "$sim$ple$");
}

TEST(LineTest, checkRemoveMethodWorksFine) {
    cmdly::Line line;
    line.append("simple");
    EXPECT_EQ(line.length(), 6);
    line.remove(0);
    EXPECT_EQ(line.length(), 5);
    EXPECT_EQ(line.str(), "imple");
    line.remove(4);
    EXPECT_EQ(line.length(), 4);
    EXPECT_EQ(line.str(), "impl");
}
