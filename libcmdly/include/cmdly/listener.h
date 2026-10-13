/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 * Copyright 2026 Sukanle
 *
 * SPDX-License-Identifier: BSD-3-Clause AND Apache-2.0
 */

#ifndef CMDLY_LISTENER_H
#define CMDLY_LISTENER_H
#include <version>
#ifdef __cpp_concepts
#  include <concepts>
#endif
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <cmdly/cursor.h>
#include <cmdly/line.h>
#include <string>

namespace cmdly {

class Terminal;

/**
 * A move-only, type-erased callable wrapper.
 *
 * Same role as std::move_only_function (C++23) / boost::compat::move_only_function,
 * but implemented directly so that it also works on C++17 toolchains. The
 * callable is stored type-erased behind one virtual call and is never copied:
 * handlers are free to capture move-only objects such as std::unique_ptr.
 *
 * Note: the callable is heap allocated, so registering a listener costs one
 * allocation.
 */
template<typename Signature>
class MoveOnlyFunction;

/* End of primary template declaration */

namespace detail {

// Declared ahead of MoveOnlyFunction because template parameter lists are not
// part of the complete-class context.
template<typename Callable, typename R, typename... Args>
struct IsHandlerFor : std::bool_constant<!std::is_same_v<std::decay_t<Callable>, MoveOnlyFunction<R(Args...)>>
                                         && std::is_invocable_r_v<R, std::decay_t<Callable> &, Args...>> {};

} /* End of namespace detail */

template<typename R, typename... Args>
class MoveOnlyFunction<R(Args...)> {
public:
    MoveOnlyFunction() noexcept = default;
    MoveOnlyFunction(std::nullptr_t) noexcept {}

    MoveOnlyFunction(MoveOnlyFunction &&other) noexcept = default;
    MoveOnlyFunction &operator=(MoveOnlyFunction &&other) noexcept = default;
    MoveOnlyFunction(const MoveOnlyFunction &) = delete;
    MoveOnlyFunction &operator=(const MoveOnlyFunction &) = delete;
    ~MoveOnlyFunction() = default;

    MoveOnlyFunction &operator=(std::nullptr_t) noexcept {
        _callable.reset();
        return *this;
    }

    // Not explicit on purpose: Terminal::onKeyPressed() and friends take the
    // wrapper by value, so handlers are converted implicitly.
    template<typename Callable, typename = std::enable_if_t<detail::IsHandlerFor<Callable, R, Args...>::value>>
    MoveOnlyFunction(Callable &&handler)
        : _callable(std::make_unique<Model<std::decay_t<Callable>>>(std::forward<Callable>(handler))) {}

    template<typename Callable, typename = std::enable_if_t<detail::IsHandlerFor<Callable, R, Args...>::value>>
    MoveOnlyFunction &operator=(Callable &&handler) {
        _callable = std::make_unique<Model<std::decay_t<Callable>>>(std::forward<Callable>(handler));
        return *this;
    }

    [[nodiscard]] explicit operator bool() const noexcept { return _callable != nullptr; }

    template<typename... A>
    R operator()(A &&...args) {
        if (!_callable) throw std::bad_function_call{};
        return _callable->invoke(std::forward<A>(args)...);
    }

private:
    struct TypeErased {
        virtual ~TypeErased() = default;
        virtual R invoke(Args &&...args) = 0;
    };

    template<typename Callable>
    struct Model final : TypeErased {
        explicit Model(Callable handler)
            : _handler(std::move(handler)) {}

        R invoke(Args &&...args) override { return std::invoke(_handler, std::forward<Args>(args)...); }

        Callable _handler;
    };

    std::unique_ptr<TypeErased> _callable;
}; /* End of class MoveOnlyFunction<R(Args...)> */

template<typename... Args>
class Listener {
public:
    enum class Status : uint8_t {
        OK,
        CONTINUE,
        BREAK,
    };

    Listener() noexcept = default;
    Listener(const Listener &) noexcept = delete;
    Listener &operator=(const Listener &) noexcept = delete;
    Listener(Listener &&) noexcept = default;
    Listener &operator=(Listener &&) noexcept = default;
    virtual ~Listener() noexcept = default;
    virtual Status invoke(Args... args) = 0;
    using FunctionType = MoveOnlyFunction<Status(Args... args)>;
}; /* End of class Listener */

template<typename... Args>
class ListenerFnWrap : public Listener<Args...> {
public:
    using Base = Listener<Args...>;

#ifdef __cpp_concepts
    template<class F>
        requires(!std::same_as<std::remove_cvref_t<F>, ListenerFnWrap>)
             && std::constructible_from<typename Base::FunctionType, F>
#else
    template<class F, typename = std::enable_if_t<!std::is_same_v<std::decay_t<F>, ListenerFnWrap>
                                                  && std::is_constructible_v<typename Base::FunctionType, F>>>
#endif
    explicit ListenerFnWrap(F &&handler)
        : _handler(std::forward<F>(handler)) {
    }

    typename Base::Status invoke(Args... args) override { return _handler(std::forward<Args>(args)...); }

private:
    typename Base::FunctionType _handler;
};

using KeyPressed = Listener<const Key &, Line &, Cursor &, Terminal &>;
using KeyPressedFnWrap = ListenerFnWrap<const Key &, Line &, Cursor &, Terminal &>;
using LineChanged = Listener<const std::string &, Line &, Cursor &, Terminal &>;
using LineChangedFnWrap = ListenerFnWrap<const std::string &, Line &, Cursor &, Terminal &>;
using LineEntered = Listener<const std::string &, Terminal &>;
using LineEnteredFnWrap = ListenerFnWrap<const std::string &, Terminal &>;

} /* End of namespace cmdly */

#endif /* !CMDLY_LISTENER_H */
