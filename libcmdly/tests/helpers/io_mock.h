/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 * Copyright 2026 Sukanle
 *
 * SPDX-License-Identifier: BSD-3-Clause AND Apache-2.0
 */

#ifndef CMDLY_TESTS_HELPERS_IO_MOCK_H
#define CMDLY_TESTS_HELPERS_IO_MOCK_H

#include <gmock/gmock.h>
#include "cmdly/io.h"

class IOMock : public cmdly::IO {
public:
    MOCK_METHOD(cmdly::Key, getKey, (), (override));
    MOCK_METHOD(char, getChar, (), (override));
    MOCK_METHOD(void, write, (const std::string &), (override, const));
    MOCK_METHOD(void, getWindowSize, (size_t *, size_t *), (override, const));
    MOCK_METHOD(void, die, ());
    ~IOMock() override { die(); }
}; /* End of class IOMock */

#endif /* !CMDLY_TESTS_HELPERS_IO_MOCK_H */
