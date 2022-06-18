/*
 * Xournal++
 *
 * A tiny vector stored in the stack, with a small maximal size
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <array>
#include <cassert>
#include <stdexcept>

#include "BasePointerIterator.h"

template <class T, size_t N>
class UninitializedStorage {
public:
    using size_type = size_t;
    using value_type = T;
    using reference = T&;

    T& operator[](size_type pos) { return *std::launder(reinterpret_cast<T*>(&dataArray[pos])); }
    const T& operator[](size_type pos) const { return *std::launder(reinterpret_cast<const T*>(&dataArray[pos])); }

    T* data() { return std::launder(reinterpret_cast<T*>(dataArray.data())); }
    const T* data() const { return std::launder(reinterpret_cast<const T*>(dataArray.data())); }

protected:
    std::array<typename std::aligned_storage<sizeof(T), alignof(T)>::type, N> dataArray;
};

template <class T, size_t N>
class TinyVector: public UninitializedStorage<T, N> {
public:
    using size_type = size_t;
    using value_type = T;
    using reference = T&;
    using iterator = BasePointerIterator<T, false>;
    using const_iterator = BasePointerIterator<T, true>;
    using difference_type = ptrdiff_t;

    TinyVector() = default;
    ~TinyVector() { std::destroy(begin(), end()); }
    TinyVector(const TinyVector& other) { std::copy(other.begin(), other.end(), std::back_inserter(*this)); }
    TinyVector(TinyVector&& other) {
        std::move(other.begin(), other.end(), std::back_inserter(*this));
        other.clear();
    }
    TinyVector& operator=(const TinyVector& other) {
        if (this == &other) {
            return *this;
        }
        clear();
        std::copy(other.begin(), other.end(), std::back_inserter(*this));
        return *this;
    }
    TinyVector& operator=(TinyVector&& other) {
        if (this == &other) {
            return *this;
        }
        clear();
        std::move(other.begin(), other.end(), std::back_inserter(*this));
        other.clear();
        return *this;
    }

    size_type size() const { return nb; }
    bool empty() const { return nb == 0; }

    T& front() { return *UninitializedStorage<T, N>::data(); }
    const T& front() const { return *UninitializedStorage<T, N>::data(); }
    T& back() { return *(UninitializedStorage<T, N>::data() + nb - 1); }
    const T& back() const { return *(UninitializedStorage<T, N>::data() + nb - 1); }

    template <class... Args>
    void emplace_back(Args&&... args) {
        assert(nb < N);
        ::new (&UninitializedStorage<T, N>::dataArray[nb++]) T(std::forward<Args>(args)...);
    }
    void push_back(const T& value) { emplace_back(value); }
    void push_back(T&& value) { emplace_back(std::move(value)); }
    void pop_back() {
        assert(nb != 0);
        std::destroy_at(&back());
        nb--;
    }
    void clear() {
        std::destroy(begin(), end());
        nb = 0;
    }

    void erase_from(const iterator& first) {
        std::destroy(first, end());
        nb = static_cast<size_type>(std::distance(begin(), first));
    }

    void swap(TinyVector& other) {
        if (nb < other.nb) {
            iterator it = std::swap_ranges(begin(), end(), other.begin());
            std::move(it, other.end(), std::back_inserter(*this));
            other.erase_from(it);
        } else {
            iterator it = std::swap_ranges(other.begin(), other.end(), begin());
            std::move(it, end(), std::back_inserter(other));
            erase_from(it);
        }
    }

    iterator begin() { return UninitializedStorage<T, N>::data(); }
    const_iterator begin() const { return UninitializedStorage<T, N>::data(); }
    iterator end() { return UninitializedStorage<T, N>::data() + nb; }
    const_iterator end() const { return UninitializedStorage<T, N>::data() + nb; }

private:
    size_type nb = 0;
};

namespace std {
template <typename T, size_t N>
void swap(TinyVector<T, N>& a, TinyVector<T, N>& b) {
    a.swap(b);
}
}  // namespace std
