/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 * Copyright 2026 Sukanle
 *
 * SPDX-License-Identifier: BSD-3-Clause AND Apache-2.0
 */

#include <memory>
#include "cmdly/terminal.h"

int main() {
    fmt::print("Welcome to Basic  Console!\n");
    fmt::print("To quit type \"exit\" or press <CTRL+C>\n");

    auto terminal = std::make_unique<cmdly::Terminal>();

    while (true) {
        auto line = terminal->readLine("console> ");
        fmt::print("line={}\n", line);
        if (line == "exit") {
            fmt::print("bye!\n");
            break;
        }
    }

    return 0;
}
