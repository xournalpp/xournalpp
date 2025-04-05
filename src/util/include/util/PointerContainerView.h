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

// Todo(cpp20) use std::to_address instead
template <typename T>
T* to_address(T*& t) {
    return t;
}
template <typename T>
T* const to_address(T* const& t) {
    return t;
}

template <typename T>
T* to_address(std::unique_ptr<T>& t) {
    return t.get();
}

template <typename T>
const T* to_address(const std::unique_ptr<T>& t) {
    return t.get();
}

template <typename container_type>
class PointerContainerView {
public:
    using value_type = const typename std::pointer_traits<typename container_type::value_type>::element_type*;
    class iterator;
    using const_iterator = iterator;
    using reference = const value_type&;
    using pointer = const value_type*;
    using difference_type = typename container_type::difference_type;
    using size_type = typename container_type::size_type;

    PointerContainerView(const container_type& container): b(container.cbegin()), e(container.cend()) {}

    iterator begin() const { return b; }
    iterator end() const { return e; }
    std::reverse_iterator<iterator> rbegin() const { return std::reverse_iterator(e); }
    std::reverse_iterator<iterator> rend() const { return std::reverse_iterator(b); }
    value_type operator[](size_type n) const { return b[static_cast<typename iterator::difference_type>(n)]; }
    value_type front() const { return *b; }
    value_type back() const { return e[-1]; }

    size_t size() const { return as_unsigned(std::distance(b, e)); }

    /// Clones the view into a vector. The pointed elements are not cloned.
    std::vector<value_type> clone() const { return {begin(), end()}; }

private:
    const iterator b;
    const iterator e;
};

template <typename container_type>
class PointerContainerView<container_type>::iterator: public container_type::const_iterator {
public:
    using value_type = const typename std::pointer_traits<typename container_type::value_type>::element_type*;
    using reference = const value_type&;
    using pointer = const value_type*;

    iterator() = default;
    ~iterator() = default;
    iterator(typename container_type::const_iterator it): container_type::const_iterator(it) {}

    value_type operator*() const { return to_address(container_type::const_iterator::operator*()); }
    pointer operator->() const { return container_type::const_iterator::operator->(); }
    value_type operator[](difference_type n) const { return to_address(container_type::const_iterator::operator[](n)); }
};


static_assert(std::is_same_v<PointerContainerView<std::vector<int*>>::value_type, const int*>);
static_assert(std::is_same_v<PointerContainerView<std::vector<const int*>>::value_type, const int*>);
static_assert(std::is_same_v<PointerContainerView<std::vector<std::unique_ptr<int>>>::value_type, const int*>);
static_assert(std::is_same_v<PointerContainerView<std::vector<std::unique_ptr<const int>>>::value_type, const int*>);
static_assert(std::is_same_v<PointerContainerView<std::vector<std::shared_ptr<int>>>::value_type, const int*>);
static_assert(std::is_same_v<PointerContainerView<std::vector<std::shared_ptr<const int>>>::value_type, const int*>);
}  // namespace xoj::util
