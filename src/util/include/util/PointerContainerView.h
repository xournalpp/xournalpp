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

#include "util/ViewIteratorBase.h"
#include "util/safe_casts.h"

namespace xoj::util {
template <typename container_type>
class PointerContainerView {
public:
    using value_type = const typename std::pointer_traits<typename container_type::value_type>::element_type*;
    using reference = const value_type&;
    using pointer = const value_type*;
    using difference_type = typename container_type::difference_type;
    using size_type = typename container_type::size_type;

    template <typename base_it>
    class iterator_impl: public ViewIteratorBase<iterator_impl<base_it>, base_it> {
    public:
        using value_type = const typename std::pointer_traits<typename container_type::value_type>::element_type*;
        using reference = const value_type&;
        using pointer = const value_type*;

        iterator_impl() = default;
        iterator_impl(const iterator_impl& o) = default;
        iterator_impl(iterator_impl&& o) = default;
        iterator_impl& operator=(const iterator_impl& o) = default;
        iterator_impl& operator=(iterator_impl&& o) = default;

        iterator_impl(base_it it): ViewIteratorBase<iterator_impl<base_it>, base_it>(it) {}

        // We must return by value
        value_type operator*() const { return std::to_address(*ViewIteratorBase<iterator_impl<base_it>, base_it>::it); }
        value_type operator[](difference_type n) const {
            return std::to_address(ViewIteratorBase<iterator_impl<base_it>, base_it>::it[n]);
        }

        /*
         * operator->() is not required by std::random_access_iterator but Clang's libc++20 seems to rely on
         * std::to_address to implement the constructor std::vector::vector(It b, It e). So we add it.
         *
         * Also, operator*() returns by value, so we cannot only return a value_type*
         */
        std::unique_ptr<value_type> operator->() const { return std::make_unique<value_type>(operator*()); }
    };
    using iterator = iterator_impl<typename container_type::const_iterator>;
    using const_iterator = iterator;
    using reverse_iterator = std::reverse_iterator<iterator>;

    PointerContainerView(const container_type& container): b(container.cbegin()), e(container.cend()) {}

    iterator begin() const { return b; }
    iterator end() const { return e; }
    reverse_iterator rbegin() const { return reverse_iterator(e); }
    reverse_iterator rend() const { return reverse_iterator(b); }
    value_type operator[](size_type n) const { return b[static_cast<typename iterator::difference_type>(n)]; }
    value_type front() const { return *b; }
    value_type back() const { return e[-1]; }

    size_t size() const { return as_unsigned(std::distance(b, e)); }

    /// Clones the view into a vector. The pointed elements are not cloned.
    std::vector<value_type> clone() const { return std::vector<value_type>(begin(), end()); }

private:
    const iterator b;
    const iterator e;
};

static_assert(std::ranges::random_access_range<PointerContainerView<std::vector<std::unique_ptr<const int>>>>);
static_assert(std::random_access_iterator<PointerContainerView<std::vector<std::unique_ptr<const int>>>::iterator>);
static_assert(std::is_same_v<PointerContainerView<std::vector<int*>>::value_type, const int*>);
static_assert(std::is_same_v<PointerContainerView<std::vector<const int*>>::value_type, const int*>);
static_assert(std::is_same_v<PointerContainerView<std::vector<std::unique_ptr<int>>>::value_type, const int*>);
static_assert(std::is_same_v<PointerContainerView<std::vector<std::unique_ptr<const int>>>::value_type, const int*>);
static_assert(std::is_same_v<PointerContainerView<std::vector<std::shared_ptr<int>>>::value_type, const int*>);
static_assert(std::is_same_v<PointerContainerView<std::vector<std::shared_ptr<const int>>>::value_type, const int*>);
}  // namespace xoj::util
