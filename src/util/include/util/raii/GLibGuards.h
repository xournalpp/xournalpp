/*
 * Xournal++
 *
 * Basic memory RAII Guards for GLib objects
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>
#include <type_traits>

#include <gtk/gtk.h>
namespace xoj::util {
inline namespace raii {

#define XOJ_GIO_GUARD_GENERATOR(class_name, type, deletion_func) \
    namespace detail {                                           \
    struct class_name##GuardDelete {                             \
        auto operator()(type* ptr) const -> void {               \
            if (ptr) {                                           \
                deletion_func(ptr);                              \
                ptr = nullptr;                                   \
            }                                                    \
        }                                                        \
    };                                                           \
    } /* namespace detail */                                     \
    using class_name##Guard = std::unique_ptr<type, detail::class_name##GuardDelete>

#define XOJ_GIO_GUARD_GENERATOR_TYPE(type, deletion_func) XOJ_GIO_GUARD_GENERATOR(type, type, deletion_func)

XOJ_GIO_GUARD_GENERATOR(GStrv, gchar*, g_strfreev);
XOJ_GIO_GUARD_GENERATOR_TYPE(GError, g_error_free);

template <typename Smart, typename Pointer /* , typename ... Tuple */>
struct out_ptr_t {
    out_ptr_t(Smart& smart /* , Tuple tuple */): smart(smart) /* , tuple(tuple) */ {}
    ~out_ptr_t() noexcept { smart.reset(ptr /*,  Todo: unpack tuble with indices trick if required*/); }
    out_ptr_t(out_ptr_t const&) = delete;
    auto operator=(out_ptr_t const&) -> out_ptr_t& = delete;

    operator Pointer*() noexcept { return &ptr; }

private:
    Smart& smart;
    Pointer ptr{};
    // Tuple tuple;
};


template <typename Smart, typename... A>
auto out_ptr(Smart& s /*, A&&... a */) noexcept -> out_ptr_t<Smart, typename Smart::pointer /* , std::tuple<A...> */> {
    return out_ptr_t<Smart, typename Smart::pointer /* , std::tuple<A...> */>  //
            (s /* , std::tuple<A...>(std::forward<A>(a)...) */);
}

}  // namespace raii
}  // namespace xoj::util