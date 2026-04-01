/*
 * Xournal++
 *
 * Helper structures to get various types depending on a boolean value
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once
/**
 * @brief Get `const T` if the boolean is true, `T` otherwise
 */
template <typename T, bool>
struct const_if {
    using type = T;
};
template <typename T>
struct const_if<T, true> {
    using type = const T;
};

/**
 * @brief Get `T` if the boolean is true, `U` otherwise
 */
template <typename T, typename U, bool>
struct type_or_type {
    using type = T;
};
template <typename T, typename U>
struct type_or_type<T, U, false> {
    using type = U;
};
