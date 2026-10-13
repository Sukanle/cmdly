/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 * Copyright 2026 Sukanle
 *
 * SPDX-License-Identifier: BSD-3-Clause AND Apache-2.0
 */

#include <cmdly/io.h>

using namespace cmdly;

int main() {
    fmt::println("Welcome to STDIO Reader!");
    fmt::println("Press various key combinations to see result");
    fmt::println("To quit press <CTRL+C>");

    auto io = std::make_unique<StandardIO>();

    while (true) {
        auto key = io->getKey();
        fmt::println("{}", key.info());
        if (key == Key::Ctrl('c')) break;
    }

    return 0;
}
