/*
 * Xournal++
 *
 * header for casting c++ callbacks into gtk callbacks
 * will be removed later
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstdint>
#include <tuple>
#include <type_traits>

#include <glib-object.h>
#include <glib.h>

namespace xoj::util {

namespace detail {

template <typename F>
struct FunctionTraits;

template <typename R, typename... Args>
struct FunctionTraits<R (&)(Args...)> {
    using Reference = R (&)(Args...);
    using RetType = R;
    using ArgTypes = std::tuple<Args...>;
    static constexpr std::size_t ArgCount = sizeof...(Args);
    template <std::size_t N>
    using NthArg = std::tuple_element_t<N, ArgTypes>;
    using FirstArg = NthArg<0>;
    using LastArg = NthArg<ArgCount - 1>;
};

template <typename R, typename... Args>
struct FunctionTraits<R (*)(Args...)> {
    using Pointer = R (*)(Args...);
    using RetType = R;
    using ArgTypes = std::tuple<Args...>;
    static constexpr std::size_t ArgCount = sizeof...(Args);
    template <std::size_t N>
    using NthArg = std::tuple_element_t<N, ArgTypes>;
    using FirstArg = NthArg<0>;
    using LastArg = NthArg<ArgCount - 1>;
};

template <typename F, size_t... Is>
constexpr auto indices_impl(F f, std::index_sequence<Is...>) {
    return f(std::integral_constant<size_t, Is>()...);
}

template <size_t N, typename F>
constexpr auto indices(F f) {
    return indices_impl(f, std::make_index_sequence<N>());
}

template <auto SrcFn, class ST, class... Args>
constexpr auto callback_once_impl(Args... args, void* data) -> gboolean {
    SrcFn(args..., static_cast<ST>(data));
    return G_SOURCE_REMOVE;
}

template <auto SrcFn, class R, class ST, class... Args>
constexpr auto wrap_impl(Args... args, void* data) -> std::conditional_t<std::is_same_v<bool, R>, gboolean, R> {
    if constexpr (std::is_same_v<void, R>) {
        SrcFn(args..., static_cast<ST>(data));
        return;
    } else if constexpr (std::is_same_v<bool, R>) {
        return SrcFn(args..., static_cast<ST>(data)) ? G_SOURCE_CONTINUE : G_SOURCE_REMOVE;
    } else {
        return SrcFn(args..., static_cast<ST>(data));
    }
}

}  // namespace detail

/**
 * wrap a C++ function into a C / glib compatible callback function
 * for single execution. Usable for API's like g_timeout_add / g_idle_add
 */
template <auto SrcFn>
constexpr auto wrap_for_once() -> auto {
    using namespace detail;
    using FTI = FunctionTraits<decltype(SrcFn)>;
    return indices<FTI::ArgCount - 1>([&](auto... Is) {
        return callback_once_impl<SrcFn, typename FTI::LastArg, std::tuple_element_t<Is, typename FTI::ArgTypes>...>;
    });
}

template <auto SrcFn>
constexpr inline auto wrap_for_once_v = wrap_for_once<SrcFn>();

/**
 * Generally wrap a C++ function into a C / glib compatible callback function.
 * Removes warnings for functionpointer casts due to UB without reinterpreting the functionpointer.
 * Preserves type checking behavior of the compiler.
 * Replaces bool return values with gboolean.
 * statically casts data pointer to the last argument type of the function.
 * Use this whenever you would cast a functionpointer to any callback function.
 */
template <auto SrcFn>
constexpr auto wrap() -> auto {
    using namespace detail;
    using FTI = FunctionTraits<decltype(SrcFn)>;
    return indices<FTI::ArgCount - 1>([&](auto... Is) {
        return wrap_impl<SrcFn, typename FTI::RetType, typename FTI::LastArg,
                         std::tuple_element_t<Is, typename FTI::ArgTypes>...>;
    });
}

template <auto SrcFn>
constexpr inline auto wrap_v = wrap<SrcFn>();

/**
 * wrap a C++ function into a C / glib compatible callback function, just like wrap,
 * but reinterpret casts the functionpointer to a GCallback, which can be used for dynamic callback dispatching.
 * For example all g_signal_connect*, functions.
 * This wrapper function must be used with caution, as the reinterpret cast removes all type checking capabilities of
 * the compiler. The signature of the wrapper, must match the signature of the callback function.
 * Possible improvements: add a template type parameter for the callback function signature, to check the signature of
 * the target callback.
 * Use this instead of G_CALLBACK
 */
template <auto SrcFn>
inline auto const wrap_for_g_callback_v = G_CALLBACK(wrap_v<SrcFn>);


template <typename T, std::enable_if_t<!std::is_same_v<T, void>, int> = 1>
constexpr void destroy_cb(gpointer data) {
    delete static_cast<T*>(data);
};

template <typename T, std::enable_if_t<!std::is_same_v<T, void>, int> = 1>
constexpr void closure_notify_cb(gpointer data, GClosure*) {
    delete static_cast<T*>(data);
};

#ifndef NDEBUG
constexpr auto tester1(int*, double*, char*) -> bool { return true; }
constexpr auto tester2(int*, double*, char*) -> uint64_t { return 1; }
static_assert(std::is_same_v<decltype(wrap_for_once<tester1>()), gboolean (*)(int*, double*, void*)>);
static_assert(std::is_same_v<decltype(wrap_for_once<tester2>()), gboolean (*)(int*, double*, void*)>);
static_assert(std::is_same_v<decltype(wrap<tester1>()), gboolean (*)(int*, double*, void*)>);
static_assert(std::is_same_v<decltype(wrap<tester2>()), uint64_t (*)(int*, double*, void*)>);
#endif

}  // namespace xoj::util
