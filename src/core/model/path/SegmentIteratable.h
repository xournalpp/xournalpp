/**
 * Xournal++
 *
 * Iteratable adaptor for segment-based iterations on paths
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "util/Assert.h"
#include "util/TypeIfThenElse.h"

#include "Path.h"

template <class T>
class Path::SegmentIteratable {
private:
    template <class U>
    struct pttype {
        using type = typename U::point_t;
    };
    template <class U>
    struct pttype<const U> {
        using type = const typename U::point_t;
    };

public:
    using value_type = T;
    using point_type = typename pttype<value_type>::type;

    SegmentIteratable(point_type* begin, point_type* end):
            beginIt(reinterpret_cast<value_type*>(begin)), endIt(reinterpret_cast<value_type*>(end)) {}
    ~SegmentIteratable() = default;

    class Iterator {
    public:
        using value_type = Path::SegmentIteratable<T>::value_type;
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        static_assert(sizeof(value_type) % sizeof(point_type) == 0);
        static constexpr difference_type shift = sizeof(value_type) / sizeof(point_type) - 1;

        [[maybe_unused]] Iterator() = default;
        [[maybe_unused]] Iterator(const Iterator& it) = default;
        [[maybe_unused]] Iterator(pointer ptr): ptr(ptr) {}
        [[maybe_unused]] ~Iterator() = default;

        [[maybe_unused]] Iterator& operator=(const Iterator& other) = default;

        /**
         * @brief Increment/Decrement the iterator
         *
         * A SplineSegment consists of 4 Points, but its last Point (its second knot) is the next segment's first knot.
         * This shared knot is stored only once, so two consecutive SplineSegments overlap in memory (by 1 Point).
         * This gives the following formulae
         */
        [[maybe_unused]] Iterator operator++(int) {  // it++
            Iterator it = *this;
            ptr = (pointer)((point_type*)ptr + shift);
            return it;
        }
        [[maybe_unused]] Iterator& operator++() {  // ++it
            ptr = (pointer)((point_type*)ptr + shift);
            return *this;
        }
        [[maybe_unused]] Iterator operator--(int) {  // it--
            Iterator it = *this;
            ptr = (pointer)((point_type*)ptr - shift);
            return it;
        }
        [[maybe_unused]] Iterator& operator--() {  // --it
            ptr = (pointer)((point_type*)ptr - shift);
            return *this;
        }
        [[maybe_unused]] Iterator& operator+=(difference_type n) {
            ptr = (pointer)((point_type*)ptr + shift * n);
            return *this;
        }
        [[maybe_unused]] Iterator& operator-=(difference_type n) {
            ptr = (pointer)((point_type*)ptr - shift * n);
            return *this;
        }

        [[maybe_unused]] Iterator operator+(difference_type n) const {
            return Iterator((pointer)((point_type*)ptr + shift * n));
        }
        [[maybe_unused]] Iterator operator-(difference_type n) const {
            return Iterator((pointer)((point_type*)ptr - shift * n));
        }

        [[maybe_unused]] difference_type operator-(const Iterator& other) const {
            xoj_assert(((point_type*)this->ptr - (point_type*)other.ptr) % shift == 0);
            return ((point_type*)this->ptr - (point_type*)other.ptr) / shift;
        }

        [[maybe_unused]] bool operator==(const Iterator& other) const { return this->ptr == other.ptr; }
        [[maybe_unused]] bool operator!=(const Iterator& other) const { return this->ptr != other.ptr; }

        [[maybe_unused]] reference operator*() { return *ptr; }
        [[maybe_unused]] pointer operator->() { return ptr; }
        [[maybe_unused]] reference operator[](difference_type n) { return *(pointer)((point_type*)ptr + shift * n); }

        [[maybe_unused]] bool operator<(const Iterator& other) const { return this->ptr < other.ptr; }
        [[maybe_unused]] bool operator<=(const Iterator& other) const { return this->ptr <= other.ptr; }
        [[maybe_unused]] bool operator>(const Iterator& other) const { return this->ptr > other.ptr; }
        [[maybe_unused]] bool operator>=(const Iterator& other) const { return this->ptr >= other.ptr; }

    private:
        pointer ptr = nullptr;
    };

    Iterator begin() const { return beginIt; }
    Iterator end() const { return endIt; }

    Iterator iteratorAt(size_t i) { return beginIt + static_cast<typename Iterator::difference_type>(i); }

    value_type& operator[](size_t i) { return *(beginIt + static_cast<typename Iterator::difference_type>(i)); }
    const value_type& operator[](size_t i) const {
        return *(beginIt + static_cast<typename Iterator::difference_type>(i));
    }

    size_t size() const { return static_cast<size_t>(endIt - beginIt); }

    Iterator beginIt;
    Iterator endIt;
};

template <class value_type>
[[maybe_unused]] typename Path::SegmentIteratable<value_type>::Iterator operator+(
        typename Path::SegmentIteratable<value_type>::Iterator::difference_type n,
        typename Path::SegmentIteratable<value_type>::Iterator it) {
    return it + n;
}
