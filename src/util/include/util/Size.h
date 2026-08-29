/*
 * Xournal++
 *
 * A rectangular size
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

namespace xoj::util {
template <class T>
struct Size {
    constexpr bool operator==(const Size<T>&) const = default;
    T width{};
    T height{};
};
}  // namespace xoj::util
