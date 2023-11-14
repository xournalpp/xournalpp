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

#include <type_traits>

template <typename container_type>
class PointerContainerView {
public:
    static_assert(std::is_pointer<typename container_type::value_type>());
    using value_type = const typename std::remove_pointer<typename container_type::value_type>::type*;
    class iterator;
    using const_iterator = iterator;
    using reference = const value_type&;
    using pointer = const value_type*;
    using difference_type = typename container_type::difference_type;
    using size_type = typename container_type::size_type;

    PointerContainerView(const container_type& container): b(container.cbegin()), e(container.cend()) {}

    iterator begin() const { return b; }
    iterator end() const { return e; }
    reference operator[](size_type n) const { return b[static_cast<typename iterator::difference_type>(n)]; }
    reference front() const { return *b; }
    reference back() const { return e[-1]; }

private:
    const iterator b;
    const iterator e;
};

template <typename container_type>
class PointerContainerView<container_type>::iterator {
public:
    using iterator_category = typename container_type::const_iterator::iterator_category;
    using value_type = const typename std::remove_pointer<typename container_type::value_type>::type*;
    using reference = const value_type&;
    using pointer = const value_type*;
    using difference_type = typename container_type::const_iterator::difference_type;

    iterator() = default;
    ~iterator() = default;
    iterator(typename container_type::const_iterator it): it(it) {}

    reference operator*() const { return *it; }
    pointer operator->() const { return it.operator->(); }

    iterator& operator++() {
        ++it;
        return *this;
    }
    iterator operator++(int) {
        auto tmp = *this;
        ++it;
        return tmp;
    }
    iterator& operator--() {
        --it;
        return *this;
    }
    iterator operator--(int) {
        auto tmp = *this;
        --it;
        return tmp;
    }
    iterator& operator+=(difference_type n) {
        it += n;
        return *this;
    }
    iterator& operator-=(difference_type n) {
        it -= n;
        return *this;
    }

    iterator operator+(difference_type n) const { return iterator(it + n); }
    iterator operator-(difference_type n) const { return iterator(it - n); }
    difference_type operator-(const iterator& other) const { return it - other.it; }

    reference operator[](difference_type n) const { return it[n]; }

    bool operator==(const iterator& other) const { return it == other.it; }
    bool operator!=(const iterator& other) const { return it != other.it; }

    bool operator<(const iterator& other) const { return it < other.it; }
    bool operator>(const iterator& other) const { return other < *this; }
    bool operator<=(const iterator& other) const { return !(other < *this); }
    bool operator>=(const iterator& other) const { return !(*this < other); }

    friend iterator operator+(difference_type n, const iterator& it) { return it + n; }

private:
    typename container_type::const_iterator it;
};
