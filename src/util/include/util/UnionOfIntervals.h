/*
 * Xournal++
 *
 * A data structure for unions of intervals of a numeric data type.
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <iterator>
#include <vector>

#include "Interval.h"

/**
 * @brief A data structure for unions of intervals of a numeric data type
 *
 * The intervals are stored as a sorted list of their bounds. The list must be of even length.
 * This data structure makes the computation of the union/intersection/complement of those intervals very simple.
 * The downside is: it makes iterating through the structure a bit weird. See cloneToIntervalVector below for an example
 */
template <class T>
class UnionOfIntervals final {
public:
    UnionOfIntervals() = default;

    /**
     * @brief Reset the union of intervals to be the single interval [lowerBound, upperBound).
     * Assumes lowerBound < upperBound
     * @param lowerBound The lower bound
     * @param upperBound The upper bound
     */
    [[maybe_unused]] void set(const T& lowerBound, const T& upperBound) { data = {lowerBound, upperBound}; }

    /**
     * @brief Add the intervals of other to this
     * @param other Any forward-iteratable sorted container of type T of **even length**
     * @return True if changes were made, false otherwise
     */
    template <class C>
    [[maybe_unused]] bool unite(const C& other) {
        bool changesMade = false;
        bool insideThis = false;
        bool insideOther = false;
        auto itThis = this->data.begin();
        auto itOther = other.cbegin();
        while (itThis != this->data.end() && itOther != other.cend()) {
            if (*itOther == *itThis) {
                if (insideThis != insideOther) {
                    // This bound is no longer needed
                    // Remove and increment
                    itThis = this->data.erase(itThis);
                    changesMade = true;
                } else {
                    ++itThis;
                }
                itOther++;
                insideThis = !insideThis;
                insideOther = !insideOther;
            } else if (*itOther < *itThis) {
                insideOther = !insideOther;
                if (!insideThis) {
                    // Open or close an interval (depending on the value of insideOther)
                    itThis = std::next(this->data.insert(itThis, *itOther));
                    changesMade = true;
                }
                ++itOther;
            } else {
                insideThis = !insideThis;
                if (insideOther) {
                    // This bound is no longer needed
                    // Remove and increment
                    itThis = this->data.erase(itThis);
                    changesMade = true;
                } else {
                    ++itThis;
                }
            }
        }
        if (itOther == other.cend()) {
            return changesMade;
        }
        std::copy(itOther, other.cend(), std::back_inserter(this->data));
        assert(this->data.size() % 2 == 0);
        return true;
    }

    /**
     * @brief Intersect the intervals of this with those of other
     * @param other Any forward-iteratable sorted container of type T of **even length**
     * @return True if changes were made, false otherwise
     */
    template <class C>
    [[maybe_unused]] bool intersect(const C& other) {
        bool changesMade = false;
        bool insideThis = false;
        bool insideOther = false;
        auto itThis = this->data.begin();
        auto itOther = other.cbegin();
        while (itThis != this->data.end() && itOther != other.cend()) {
            if (*itOther == *itThis) {
                if (insideThis != insideOther) {
                    // This bound is no longer needed
                    // Remove and increment
                    itThis = this->data.erase(itThis);
                    changesMade = true;
                } else {
                    ++itThis;
                }
                ++itOther;
                insideThis = !insideThis;
                insideOther = !insideOther;
            } else if (*itOther < *itThis) {
                insideOther = !insideOther;
                if (insideThis) {
                    // Open or close an interval (depending on the value of insideOther)
                    itThis = std::next(this->data.insert(itThis, *itOther));
                    changesMade = true;
                }
                itOther++;
            } else {
                insideThis = !insideThis;
                if (!insideOther) {
                    // This bound is no longer needed
                    // Remove and increment
                    itThis = this->data.erase(itThis);
                    changesMade = true;
                } else {
                    ++itThis;
                }
            }
        }
        data.erase(itThis, data.end());
        assert(this->data.size() % 2 == 0);
        return changesMade;
    }

    /**
     * @brief Replace the union of intervals by its complement inside [lowerBound, upperBound].
     * Assumes all values lie in this interval.
     * @param lowerBound The lower bound
     * @param upperBound The upper bound
     */
    void complement(const T& lowerBound, const T& upperBound) {
        if (data.empty()) {
            data = {lowerBound, upperBound};
            return;
        }

        if (data.front() == lowerBound) {
            data.erase(data.begin());
        } else {
            data.insert(data.begin(), lowerBound);
        }
        if (data.back() == upperBound) {
            data.pop_back();
        } else {
            data.push_back(upperBound);
        }
        assert(this->data.size() % 2 == 0);
    }

    /**
     * @brief Copy the data into a vector of type Interval
     * @return The copied vector
     */
    std::vector<Interval<T>> cloneToIntervalVector() const {
        std::vector<Interval<T>> result;
        assert(this->data.size() % 2 == 0);
        result.reserve(data.size() / 2);

        auto itLowerBound = data.cbegin();
        auto itUpperBound = itLowerBound;
        auto itEnd = data.cend();
        while (itLowerBound != itEnd) {
            itUpperBound = std::next(itLowerBound);
            result.emplace_back(*itLowerBound, *itUpperBound);
            itLowerBound = std::next(itUpperBound);
        }
        return result;
    }

    /**
     * @brief Determine if the union of intervals is empty or not
     * @return true if it is empty, false otherwise
     */
    inline bool empty() const { return data.empty(); }

    /**
     * @brief Get a reference to the data
     */
    const std::vector<T>& getData() const { return data; }

    /**
     * @brief Swap the content with that of other
     */
    void swap(UnionOfIntervals<T>& other) { data.swap(other.data); }

    /**
     * @brief Append raw data to the union's data
     * @param container Container with the added data
     *
     * This assumes:
     *   * container is a container of type T of even length
     *   * container is sorted
     *   * container.front() > data.back()
     */
    template <typename container_type>
    void appendData(const container_type& container) {
        assert(container.size() % 2 == 0);
        std::copy(container.begin(), container.end(), std::back_inserter(data));
    }

private:
    std::vector<T> data;
};
