/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#ifndef CMDLY_LISTENER_H
#define CMDLY_LISTENER_H
#include <version>
#ifndef __cpp_lib_copyable_function
#  include <boost/compat/move_only_function.hpp>
#else
#  include <functional>
#endif
#include <cmdly/cursor.h>
#include <cmdly/line.h>
#include <string>

namespace cmdly {

class Terminal;

template<typename F>
#ifndef __cpp_lib_copyable_function
using MoveOnlyFunction = boost::compat::move_only_function<F>;
#else
using MoveOnlyFunction = std::move_only_function<F>;
#endif

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
                                                  && std::is_constructible_v<FunctionType, F>>>
#endif
    explicit ListenerFnWrap(F &&handler)
        : _handler(std::forward<F>(handler)) {
    }

    typename Base::Status invoke(Args... args) override { return _handler(std::forward<Args>(args)...); }

private:
    Base::FunctionType _handler;
};

using KeyPressed = Listener<const Key &, Line &, Cursor &, Terminal &>;
using KeyPressedFnWrap = ListenerFnWrap<const Key &, Line &, Cursor &, Terminal &>;
using LineChanged = Listener<const std::string &, Line &, Cursor &, Terminal &>;
using LineChangedFnWrap = ListenerFnWrap<const std::string &, Line &, Cursor &, Terminal &>;
using LineEntered = Listener<const std::string &, Terminal &>;
using LineEnteredFnWrap = ListenerFnWrap<const std::string &, Terminal &>;

} /* End of namespace cmdly */

#endif /* !CMDLY_LISTENER_H */
