/*
 * Xournal++
 *
 * C++20-like view to iterate on pairs of consecutive elements
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <array>
#include <iterator>
#include <type_traits>

#include "TypeIfThenElse.h"

/**
 * @brief view to iterate on pairs of consecutive elements. The pairs overlap:
 * On an array x[N], you'll get the pairs (x[0],x[1]), (x[1],x[2]), ... , (x[n-2],x[n-1])
 * @param contiguous_container_type Must be a contiguous container
 */
template <typename contiguous_container_type>
class PairView {
    // C++ 20
    // static_assert(is_same<typename std::iterator_traits<typename
    // contiguous_container_type::iterator>::iterator_category, contiguous_iterator_tag>::value, "">);
public:
    using sub_value_type = typename contiguous_container_type::value_type;
    static_assert(!std::is_same<sub_value_type, bool>::value, "bool arrays are not contiguous");
    using value_type = std::array<sub_value_type, 2>;
    PairView(contiguous_container_type& c): container(c){};

private:
    contiguous_container_type& container;

    template <bool is_const>
    class BaseIterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = typename PairView::value_type;
        using qualified_value_type = typename const_if<value_type, is_const>::type;
        using qualified_sub_value_type = typename const_if<sub_value_type, is_const>::type;
        using pointer = qualified_value_type*;
        using reference = qualified_value_type&;
        using difference_type = std::ptrdiff_t;
        using container_iterator_type =
                typename type_or_type<typename contiguous_container_type::const_iterator,
                                      typename contiguous_container_type::iterator, is_const>::type;

        BaseIterator() = default;
        ~BaseIterator() = default;
        BaseIterator(const BaseIterator&) = default;
        BaseIterator(container_iterator_type it): it(it) {}

        reference operator*() const { return *reinterpret_cast<pointer>(&*it); }
        qualified_sub_value_type& first() const { return *it; }
        qualified_sub_value_type& second() const { return it[1]; }

        BaseIterator& operator++() {
            ++it;
            return *this;
        }
        BaseIterator operator++(int) {
            auto tmp = *this;
            ++it;
            return tmp;
        }
        BaseIterator& operator--() {
            --it;
            return *this;
        }
        BaseIterator operator--(int) {
            auto tmp = *this;
            --it;
            return tmp;
        }
        BaseIterator& operator+=(difference_type n) {
            it += n;
            return *this;
        }
        BaseIterator& operator-=(difference_type n) {
            it -= n;
            return *this;
        }

        BaseIterator operator+(difference_type n) const { return BaseIterator(std::next(it, n)); }
        BaseIterator operator-(difference_type n) const { return BaseIterator(std::prev(it, n)); }
        difference_type operator-(const BaseIterator& other) const { return std::distance(it, other.it); }

        reference operator[](difference_type n) const { return *(*this + n); }

        bool operator==(const BaseIterator& other) const { return it == other.it; }
        bool operator!=(const BaseIterator& other) const { return it != other.it; }

        bool operator<(const BaseIterator& other) const { return it < other.it; }
        bool operator>(const BaseIterator& other) const { return other < *this; }
        bool operator<=(const BaseIterator& other) const { return !(other < *this); }
        bool operator>=(const BaseIterator& other) const { return !(*this < other); }

        friend BaseIterator operator+(difference_type n, BaseIterator& it) { return it + n; }

    private:
        container_iterator_type it;
    };

    using const_iterator = BaseIterator<true>;
    using iterator = BaseIterator<false>;

public:
    const_iterator begin() const { return const_iterator(container.begin()); }

    template <class T = contiguous_container_type, std::enable_if_t<!std::is_const<T>::value, int> = 0>
    iterator begin() {
        return iterator(container.begin());
    }

    const_iterator end() const {
        if (container.empty()) {
            return begin();
        }
        return const_iterator(std::prev(container.end()));
    }

    template <class T = contiguous_container_type, std::enable_if_t<!std::is_const<T>::value, int> = 0>
    iterator end() {
        if (container.empty()) {
            return begin();
        }
        return iterator(std::prev(container.end()));
    }
};
