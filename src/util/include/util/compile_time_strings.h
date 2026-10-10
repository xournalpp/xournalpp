/*
 * Xournal++
 *
 * Configuration data for generated backgrounds
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

// Copied from:
// https://github.com/lluisalemanypuig/lluisalemanypuig.github.io/blob/master/blog/2026/03/14/code_advanced.cpp
// More info in:
// https://lluisalemanypuig.github.io/blog/2026/03/14/

#include <type_traits>

template <typename type_t>
concept is_char_like =
        std::is_same_v<type_t, char> or std::is_same_v<type_t, unsigned char> or std::is_same_v<type_t, signed char> or
        std::is_same_v<type_t, char8_t> or std::is_same_v<type_t, char16_t> or std::is_same_v<type_t, char32_t>;

template <is_char_like char_t, auto... Ts>
constexpr inline const char str[] = {char_t(Ts)..., char_t('\0')};

template <is_char_like char_t, auto... chars>
struct literal_range {

    [[nodiscard]] constexpr const char_t* c_str() const noexcept { return str<char_t, chars...>; }
    [[nodiscard]] constexpr std::string_view strvw() const noexcept { return std::string_view{c_str()}; }
};

template <typename char_t, char_t... chars>
[[nodiscard]] constexpr auto operator""_cts() noexcept {
    return literal_range<char_t, chars...>{};
}

template <is_char_like T, auto... Ts, is_char_like U, auto... Us>
[[nodiscard]] constexpr auto operator+(literal_range<T, Ts...>, literal_range<U, Us...>) noexcept {
    // std::common_type_t allows dangerous implicit conversions, and perhaps
    // a different trait should be used.
    using common_type = std::common_type_t<T, U>;
    return literal_range<common_type, Ts..., Us...>{};
}
