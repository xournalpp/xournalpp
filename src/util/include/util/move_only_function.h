/*
 * Xournal++
 *
 * Rudimentary port of C++23's std::move_only_function
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

namespace xoj::util {
template <class T>
class move_only_function final {};

/**
 * Rudimentary implementation of C++23's std::move_only_function
 *
 * Specialization of std::swap and bool operator==(const move_only_function&, std::nullptr_t) are not implemented.
 * Add them if required.
 */
template <class R, class... Args>
class move_only_function<R(Args...)> final {
public:
    move_only_function() = default;
    move_only_function(move_only_function&& f) = default;
    move_only_function& operator=(move_only_function&& f) = default;
    move_only_function(const move_only_function&) = delete;
    move_only_function& operator=(const move_only_function&) = delete;
    move_only_function& operator=(std::nullptr_t) {
        invoke = nullptr;
        callable.reset();
    }

    operator bool() const { return static_cast<bool>(invoke) && static_cast<bool>(callable); }

    R operator()(Args... args) const {
        if (!invoke || !callable) {
            throw std::bad_function_call();
        }
        return invoke(callable.get(), std::forward<Args>(args)...);
    }

private:
    template <class F, class... FArgs>
    void bind(FArgs&&... fargs) {
        invoke = +[](void* p, Args... args) -> R { return (*static_cast<F*>(p))(std::forward<Args>(args)...); };
        callable = {new F(std::forward<FArgs>(fargs)...), [](void* pf) { delete static_cast<F*>(pf); }};
    };

public:
    template <class F, std::enable_if_t<!std::is_same_v<std::decay_t<F>, move_only_function>, bool> = true,
              std::enable_if_t<std::is_same_v<R, void> || std::is_convertible_v<std::invoke_result_t<F, Args...>, R>,
                               bool> = true>
    move_only_function(F&& f) {
        bind<std::decay_t<F>>(std::forward<F>(f));
    }

private:
    std::unique_ptr<void, void (*)(void*)> callable{nullptr, +[](void*) {}};
    void (*invoke)(void*, Args...) = nullptr;
};
}  // namespace xoj::util
