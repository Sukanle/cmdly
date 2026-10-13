# Cmdly

Cmdly is an C++ project designed for creating a versatile line editor, reminiscent of popular tools like bash's readline or zsh's zle. This project is crafted to offer a seamless and interactive command-line editing experience. It supports C++17 or newer standards, ensuring compatibility with modern development practices.

* `Commercial-Ready`: Cmdly is open-source and free for use in commercial projects, making it an ideal choice for both personal and business applications.

* `Easy Integration`: Integration with existing projects is easy, thanks to its well-structured and adaptable design. You can seamlessly incorporate Cmdly into your software stack without extensive modifications.

* `Simple API`: Cmdly exposes a robust API that helps developers to create and customize interfaces with ease. This API is designed to facilitate the development of feature-rich command-line interfaces for your applications.

Cmdly empowers developers to build dynamic, customizable, command-line interfaces, ensuring a user-friendly and interactive experience for your software users.

## Features
* key reading (ascii sequence codes) and key binding to function
* line reading and line editing
* history (in-memory or file-based)
* basic auto-completion
* event emitting such as key-pressed, line-changed, line-entered
* support colourful prompt (text style, cursor style)

## Planned Features 
* bash-style search <CTRL+R>
* word highlighting
* some more default key-bindings (CTRL+B, CTRL+F, etc.)
* support for internal environment variables

Something is missing here? Let me know!

## Requirements

| | |
|---|---|
| C++ standard | C++17 or newer |
| CMake | 4.0 or newer |
| Compiler | any C++17 capable compiler (GCC 8+, Clang 7+, MSVC 19.14+) |
| Build system | any CMake supported generator (Makefiles, Ninja, ...) |

## Dependencies

| Package | Version | Required | Used for | Install |
|---|---|---|---|---|
| [fmt](https://fmt.dev) | 5.0 or newer | yes | string formatting everywhere | `brew install fmt`, `apt install libfmt-dev`, `vcpkg install fmt` |
| [googletest](https://github.com/google/googletest) | 1.10 or newer | tests only | unit test framework and mocks | `brew install googletest`, `apt install libgtest-dev libgmock-dev` |

The fmt 5.0 floor is what the code actually needs: `fmt::format`, `fmt::format_to`,
`fmt::to_string`, `fmt::print`, `fmt::memory_buffer` and
`fmt::formatter<std::string>` have all been available since fmt 5.0. Three newer
APIs are deliberately avoided:

- `fmt::println()` (fmt 9.1) - the examples write the `\n` into the format string instead
- `fmt::ostream_formatter` (fmt 9.0) - `cmdly::StringLikeFormatter` in
  `libcmdly/include/cmdly/formatter.h` formats the `str()` member directly, which
  is what every cmdly escape-sequence type renders itself through
- `fmt::appender` (fmt 8.1) - `memory_buffer` is no longer an output iterator, so
  `std::back_inserter` is used instead, which works on every fmt release

The googletest 1.10 minimum comes from the `MOCK_METHOD(..., (override, const))`
form used by `libcmdly/tests/helpers/io_mock.h`.

`fmt` is located through `find_package(fmt)`, so either install it into a
standard prefix or point CMake at an existing installation:

```sh
$ cmake .. -DFMT_ROOT=/opt/homebrew/opt/fmt
$ cmake .. -DCMAKE_PREFIX_PATH=/opt/homebrew
```

If you build with `BUILD_TESTING=1`, googletest and googlemock have to be
discoverable by CMake as well (a `GTestConfig.cmake` / `libgmock.a` in a prefix
listed in `CMAKE_PREFIX_PATH`).

## Building
```sh
$ git clone https://github.com/lpodkalicki/cmdly.git
$ cd cmdly
$ mkdir build
$ cd build
$ cmake ../ -DBUILD_TESTING=1 -DBUILD_EXAMPLES=1
$ make
```

Build options:

| Option | Default | Description |
|---|---|---|
| `BUILD_TESTING` | `OFF` | build the unit tests, requires googletest/googlemock |
| `BUILD_EXAMPLES` | `OFF` | build the programs from `examples/` |

## Key Reading Example

```cpp
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
```

## Basic Terminal Example

```cpp
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
```

## More Advanced Example

```cpp
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
        // std::string::starts_with is C++20, this project targets C++17
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
```

## License

Licensed under the [Apache License, Version 2.0](LICENSE).

This project is a fork of [lpodkalicki/cmdly](https://github.com/lpodkalicki/cmdly).
Portions of the original code remain under the BSD 3-Clause License
(see [LICENSE-BSD](LICENSE-BSD)), Copyright (c) 2023, Łukasz Podkalicki.

