/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 * Copyright 2026 Sukanle
 *
 * SPDX-License-Identifier: BSD-3-Clause AND Apache-2.0
 */

#include <memory>
#include <cmdly/terminal.h>

using namespace cmdly;

int main() {
    fmt::println("Welcome to Basic  Console!");
    fmt::println("To quit type \"exit\" or press <CTRL+C>");

    auto terminal = std::make_unique<Terminal>();

    while (true) {
        auto line = terminal->readLine("console> ");
        fmt::println("line={}", line);
        if (line == "exit") {
            fmt::println("bye!");
            break;
        }
    }

    return 0;
}
