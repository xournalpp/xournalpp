/*
 * Xournal++
 *
 * A C++20-style view for containers with values pointer
 * Such a view is not allowed to modify the pointed objects.
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>
#include <type_traits>

#include "util/safe_casts.h"

namespace xoj::util {
/**
 * template class for wrapping a base iterator into an iterator for a view
 *
 * Usage:
 * class MyIterator: public ViewIteratorBase<MyIterator, BaseIterator> {
 *    // Define value_type, pointer, reference member types
 *    // Define default constructor + other contructor as needed
 *    // Define operator*(), operator[](size_t n) (if random access), operator->()
 * };
 */
template <typename iterator, std::input_iterator base_it>
class ViewIteratorBase {
public:
    using iterator_category = std::iterator_traits<base_it>::iterator_category;
    using difference_type = std::iterator_traits<base_it>::difference_type;

    constexpr ViewIteratorBase() = default;
    constexpr ViewIteratorBase(const ViewIteratorBase& o) = default;
    constexpr ViewIteratorBase(ViewIteratorBase&& o) = default;
    constexpr ViewIteratorBase& operator=(const ViewIteratorBase& o) = default;
    constexpr ViewIteratorBase& operator=(ViewIteratorBase&& o) = default;

    constexpr ViewIteratorBase(base_it it): it(it) {}

    constexpr iterator& operator++() {
        ++it;
        return static_cast<iterator&>(*this);
    }
    constexpr iterator operator++(int) { return it++; }
    constexpr iterator& operator--() {
        --it;
        return static_cast<iterator&>(*this);
    }
    constexpr iterator operator--(int) { return it--; }
    constexpr iterator& operator+=(difference_type n) {
        it += n;
        return static_cast<iterator&>(*this);
    }
    constexpr iterator operator+(difference_type n) const { return it + n; }
    friend constexpr iterator operator+(difference_type n, const iterator& self) { return self + n; }
    constexpr iterator& operator-=(difference_type n) {
        it -= n;
        return static_cast<iterator&>(*this);
    }
    constexpr iterator operator-(difference_type n) const { return it - n; }
    constexpr difference_type operator-(const ViewIteratorBase& o) const { return it - o.it; }

    constexpr auto operator==(iterator const& other) const -> bool { return it == other.it; }
    constexpr auto operator<(iterator const& other) const -> bool { return it < other.it; }
    constexpr auto operator>(iterator const& other) const -> bool { return it > other.it; }
    constexpr auto operator<=(iterator const& other) const -> bool { return it <= other.it; }
    constexpr auto operator>=(iterator const& other) const -> bool { return it >= other.it; }
    constexpr auto operator!=(iterator const& other) const -> bool { return it != other.it; }

    base_it it;
};
}  // namespace xoj::util
