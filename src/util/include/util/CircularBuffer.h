/*
 * Xournal++
 *
 * A rudimentary circular buffer
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <cstddef>
#include <vector>

template <class T>
class CircularBuffer: private std::vector<T> {
public:
    CircularBuffer(size_t length): std::vector<T>(length > 1 ? length : 1), length(length > 1 ? length : 1) {}
    ~CircularBuffer() = default;
    T front() { return (*this)[head]; }
    T back() {
        if (head == 0) {
            return (*this)[length - 1];
        }
        return (*this)[head - 1];
    }
    void push_front(const T& ev) {
        head++;
        head %= length;
        (*this)[head] = ev;
    }
    void assign(const T& ev) {
        for (T& e: *this) { e = ev; }
    }
    size_t size() { return length; }

    /**
     * Beware: begin and end are not related to the position of the head
     */
    using std::vector<T>::cbegin;
    using std::vector<T>::begin;
    using std::vector<T>::cend;
    using std::vector<T>::end;

private:
    const size_t length;
    size_t head = length - 1;
};
