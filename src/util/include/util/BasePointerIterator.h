/*
 * Xournal++
 *
 * Basic iterator based on pointer arithmetics
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>
#include <iterator>

#include "TypeIfThenElse.h"

template <typename T, bool is_const>
class BasePointerIterator {
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using qualified_value_type = typename const_if<T, is_const>::type;
    using pointer = qualified_value_type*;
    using reference = qualified_value_type&;
    using difference_type = std::ptrdiff_t;
    BasePointerIterator() = default;
    ~BasePointerIterator() = default;
    BasePointerIterator(const BasePointerIterator&) = default;
    BasePointerIterator(pointer p): p(p) {}

    reference operator*() const { return *p; }
    pointer operator->() const { return p; }

    BasePointerIterator& operator++() {
        ++p;
        return *this;
    }
    BasePointerIterator operator++(int) {
        auto tmp = *this;
        ++p;
        return tmp;
    }
    BasePointerIterator& operator--() {
        --p;
        return *this;
    }
    BasePointerIterator operator--(int) {
        auto tmp = *this;
        --p;
        return tmp;
    }
    BasePointerIterator& operator+=(difference_type n) {
        p += n;
        return *this;
    }
    BasePointerIterator& operator-=(difference_type n) {
        p -= n;
        return *this;
    }

    BasePointerIterator operator+(difference_type n) const { return BasePointerIterator(p + n); }
    BasePointerIterator operator-(difference_type n) const { return BasePointerIterator(p - n); }
    difference_type operator-(const BasePointerIterator& other) const { return p - other.p; }

    reference operator[](difference_type n) { return *(p + n); }

    bool operator==(const BasePointerIterator& other) const { return p == other.p; }
    bool operator!=(const BasePointerIterator& other) const { return p != other.p; }

    bool operator<(const BasePointerIterator& other) const { return p < other.p; }
    bool operator>(const BasePointerIterator& other) const { return other < *this; }
    bool operator<=(const BasePointerIterator& other) const { return !(other < *this); }
    bool operator>=(const BasePointerIterator& other) const { return !(*this < other); }

    friend BasePointerIterator operator+(difference_type n, BasePointerIterator& it) { return it + n; }

private:
    pointer p = nullptr;
};
