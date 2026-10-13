/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 * Copyright 2026 Sukanle
 *
 * SPDX-License-Identifier: BSD-3-Clause AND Apache-2.0
 */

#include "cmdly/terminal.h"
#include <memory>

int main() {
    fmt::print("Welcome to Advanced Console!\n");
    fmt::print("To quit type \"exit\" or press <CTRL+C>\n");

    auto io = std::make_shared<cmdly::StandardIO>();
    auto history = std::make_shared<cmdly::MemoryHistory>();
    auto completion = std::make_shared<cmdly::Completion>();
    auto terminal = std::make_unique<cmdly::Terminal>(io, history, completion);

    terminal->setPromptStyle(cmdly::TextStyle(cmdly::Color::Green));
    terminal->setLineStyle(cmdly::TextStyle(cmdly::Color::Blue));
    terminal->setCursorStyle(cmdly::CursorStyle(cmdly::RgbColor::Red));

    terminal->onKeyPressed(
        cmdly::Key::Ctrl('h'), [](const cmdly::Key &, cmdly::Line &line, cmdly::Cursor &, cmdly::Terminal &terminal) {
            terminal.writeText("\nhello!\n", cmdly::TextStyle(cmdly::Color::Red));
            line.update();
            return cmdly::KeyPressed::Status::CONTINUE;
        });

    terminal->onLineEntered([](const std::string &line, cmdly::Terminal &terminal) {
        if (line == "help") {
            terminal.writeText("Allowed commands:\n");
            terminal.writeText("  help - show this help\n");
            terminal.writeText("  echo - echo intput\n");
            terminal.writeText("  exit - quit terminal\n");
            return cmdly::LineEntered::Status::CONTINUE;
        }
        return cmdly::LineEntered::Status::OK;
    });
    completion->insert("help");

    terminal->onLineEntered([](const std::string &line, cmdly::Terminal &terminal) {

#ifdef __cpp_lib_starts_ends_with
        if (line.starts_with("echo")) {
#else
        if (!line.compare(0, 4, "echo")) {
#endif
            terminal.writeText(line.substr(5) + "\n");
            return cmdly::LineEntered::Status::CONTINUE;
        }
        return cmdly::LineEntered::Status::OK;
    });
    completion->insert("echo");

    terminal->onLineChanged([](const std::string &content, cmdly::Line &line, cmdly::Cursor &, cmdly::Terminal &) {
        if (content == "red") {
            line.setContentStyle(cmdly::TextStyle(cmdly::Color::Red));
            line.update();
        } else if (content == "blue") {
            line.setContentStyle(cmdly::TextStyle(cmdly::Color::Blue));
            line.update();
        } else if (content == "green") {
            line.setContentStyle(cmdly::TextStyle(cmdly::Color::Green));
            line.update();
        }
        return cmdly::LineChanged::Status::OK;
    });

    terminal->run("console> ");
    terminal->resetStyle();

    return 0;
}
