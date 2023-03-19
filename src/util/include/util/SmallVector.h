/*
 * Xournal++
 *
 * A small-vector-optimized data structure, with data in the stack when it's small enough, in the heap otherwise
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <utility>
#include <vector>

#include "BasePointerIterator.h"
#include "TinyVector.h"

/**
 * @brief A small-vector-optimized data structure, with data in the stack when it's small enough, in the heap otherwise
 * If the size of the container often passes from > N to <= N and vice-versa, don't use this class (or pick a better N)
 * This would result in a lot of unnecessary data moves between stack and heap, thus poorer performance.
 */
template <class T, size_t N>
class SmallVector {
public:
    using size_type = size_t;
    using value_type = T;
    using reference = T&;
    using iterator = BasePointerIterator<T, false>;
    using const_iterator = BasePointerIterator<T, true>;
    using difference_type = ptrdiff_t;

    SmallVector() = default;
    ~SmallVector() {
        if (nb <= N) {
            dataArrayClear();
        }
    }
    SmallVector(const SmallVector& other) {
        if (other.size() > N) {
            // Avoid moving from stack to heap
            dataVector = other.dataVector;
        } else {
            std::uninitialized_copy(other.begin(), other.end(), dataArray.data());
        }
        nb = other.size();
    }
    SmallVector(SmallVector&& other) {
        if (other.size() > N) {
            dataVector = std::move(other.dataVector);
            nb = other.size();
            other.nb = 0;
        } else {
            std::uninitialized_move(other.begin(), other.end(), dataArray.data());
            nb = other.size();
            other.clear();
        }
    }
    SmallVector& operator=(const SmallVector& other) {
        if (this == &other) {
            return *this;
        }
        clear();
        if (other.size() > N) {
            // Avoid moving from stack to heap
            dataVector = other.dataVector;
        } else {
            std::uninitialized_copy(other.begin(), other.end(), dataArray.data());
        }
        nb = other.size();
        return *this;
    }
    SmallVector& operator=(SmallVector&& other) {
        if (this == &other) {
            return *this;
        }
        clear();
        if (other.size() > N) {
            dataVector = std::move(other.dataVector);
            nb = other.size();
            other.nb = 0;
        } else {
            std::uninitialized_move(other.begin(), other.end(), dataArray.data());
            nb = other.size();
            other.clear();
        }
        return *this;
    }

    SmallVector(std::initializer_list<T> list) {
        nb = static_cast<size_type>(list.size());
        if (nb <= N) {
            std::uninitialized_move(list.begin(), list.end(), dataArray.data());
        } else {
            dataVector = list;  //{list.begin(), list.end()};
        }
    }
    SmallVector& operator=(std::initializer_list<T> list) {
        clear();
        nb = static_cast<size_type>(list.size());
        if (nb <= N) {
            std::uninitialized_move(list.begin(), list.end(), dataArray.data());
        } else {
            dataVector = list;  //{list.begin(), list.end()};
        }
        return *this;
    }

    size_type size() const { return nb; }
    bool empty() const { return nb == 0; }

    T& operator[](size_type pos) { return nb <= N ? dataArray[pos] : dataVector[pos]; }
    const T& operator[](size_type pos) const { return nb <= N ? dataArray[pos] : dataVector[pos]; }
    T& front() { return nb <= N ? *dataArray.data() : dataVector.front(); }
    const T& front() const { return nb <= N ? *dataArray.data() : dataVector.front(); }
    T& back() { return nb <= N ? *(dataArray.data() + nb - 1) : dataVector.back(); }
    const T& back() const { return nb <= N ? *(dataArray.data() + nb - 1) : dataVector.back(); }

    T* data() { return nb <= N ? dataArray.data() : dataVector.data(); }
    const T* data() const { return nb <= N ? dataArray.data() : dataVector.data(); }

    iterator begin() { return nb <= N ? dataArray.data() : dataVector.data(); }
    const_iterator begin() const { return nb <= N ? dataArray.data() : dataVector.data(); }
    iterator end() { return nb <= N ? dataArray.data() + nb : dataVector.data() + nb; }
    const_iterator end() const { return nb <= N ? dataArray.data() + nb : dataVector.data() + nb; }

    template <class... Args>
    void emplace_back(Args&&... args) {
        if (nb < N) {
            ::new (&dataArray[nb]) T(std::forward<Args>(args)...);
        } else if (nb == N) {
            assert(end() == begin() + N);
            dataVector.reserve(N + 1);
            std::move(dataArrayBegin(), dataArrayEnd(), std::back_inserter(dataVector));
            dataArrayClear();
            dataVector.emplace_back(std::forward<Args>(args)...);
        } else {
            dataVector.emplace_back(std::forward<Args>(args)...);
        }
        ++nb;
    }
    void push_back(const T& value) { emplace_back(value); }
    void push_back(T&& value) { emplace_back(std::move(value)); }

    void pop_back() {
        if (nb > N) {
            if (nb == N + 1) {
                std::uninitialized_move(dataVector.begin(), std::prev(dataVector.end()), dataArray.data());
                dataVector = std::vector<value_type>(0);
            } else {
                dataVector.pop_back();
            }
        } else {
            assert(nb != 0);
            std::destroy_at(dataArray.data() + nb - 1);
        }
        --nb;
    }
    void clear() {
        if (nb > N) {
            dataVector = std::vector<value_type>(0);
        } else {
            dataArrayClear();
        }
        nb = 0;
    }

    void swap(SmallVector& other) {
        if (nb > N) {
            dataVector.swap(other.dataVector);
            if (other.nb <= N) {  // Handle stack data
                std::uninitialized_move(other.dataArrayBegin(), other.dataArrayEnd(), dataArray.data());
                other.dataArrayClear();
            }
            std::swap(nb, other.nb);
            return;
        }

        if (other.nb > N) {
            dataVector.swap(other.dataVector);
            std::uninitialized_move(dataArrayBegin(), dataArrayEnd(), other.dataArray.data());
            dataArrayClear();
            std::swap(nb, other.nb);
            return;
        }

        // Both in stack
        if (nb < other.nb) {
            iterator it = std::swap_ranges(dataArrayBegin(), dataArrayEnd(), other.dataArray.data());
            std::uninitialized_move(it, other.dataArrayEnd(), dataArrayEnd());
            std::destroy(it, other.dataArrayEnd());
        } else {
            iterator it = std::swap_ranges(other.dataArrayBegin(), other.dataArrayEnd(), dataArray.data());
            std::uninitialized_move(it, dataArrayEnd(), other.dataArrayEnd());
            std::destroy(it, dataArrayEnd());
        }
        std::swap(nb, other.nb);
    }

private:
    iterator dataArrayBegin() { return dataArray.data(); }
    iterator dataArrayEnd() {
        assert(nb <= N);
        return dataArray.data() + nb;
    }
    void dataArrayClear() { std::destroy(dataArrayBegin(), dataArrayEnd()); }

    UninitializedStorage<T, N> dataArray{};
    std::vector<T> dataVector{};
    size_type nb = 0;
};

namespace std {
template <typename T, size_t N>
void swap(SmallVector<T, N>& a, SmallVector<T, N>& b) {
    a.swap(b);
}
}  // namespace std
