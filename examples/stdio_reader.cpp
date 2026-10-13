/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 * Copyright 2026 Sukanle
 *
 * SPDX-License-Identifier: BSD-3-Clause AND Apache-2.0
 */

#include "cmdly/io.h"

int main() {
    fmt::print("Welcome to STDIO Reader!\n");
    fmt::print("Press various key combinations to see result\n");
    fmt::print("To quit press <CTRL+C>\n");

    auto io = std::make_unique<cmdly::StandardIO>();

    while (true) {
        auto key = io->getKey();
        fmt::print("{}\n", key.info());
        if (key == cmdly::Key::Ctrl('c')) break;
    }

    return 0;
}
