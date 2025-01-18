/*
 * Xournal++
 *
 * A interval data structure over a numeric data type.
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <algorithm>
#include <optional>

/**
 * @brief A templated class for (half open) intervals (a,b]
 */
template <class T>
class Interval {
public:
    /**
     * The interval bounds.
     *
     * Do not change the order: the class IntervalIteratable below relies on it.
     */
    T min{};
    T max{};

    Interval() = default;

    /**
     * @brief Initialize the interval to (min,max]
     * @param min the lower bound
     * @param max the upper bound
     */
    Interval(const T& min, const T& max): min(min), max(max) {}

    /**
     * @brief Get the interval (a,b] or (b,a], depending on which is greater
     * @param a One bound
     * @param b The other bound
     * @return The interval
     */
    static inline Interval getInterval(const T& a, const T& b) { return a < b ? Interval(a, b) : Interval(b, a); }

    /**
     * @brief Computes the convex envelop of this and the other interval
     * @param other The other interval
     */
    [[maybe_unused]] void envelop(const Interval& other) {
        min = std::min(min, other.min);
        max = std::max(max, other.max);
    }

    /**
     * @brief Computes the convex envelop of this and another value
     * @param t The other value
     */
    [[maybe_unused]] void envelop(const T& t) {
        max = std::max(max, t);
        min = std::min(min, t);
    }

    /**
     * @brief Test if this interval contains a value
     * @param t The value
     * @return true if t lies in *this, false otherwise
     */
    [[maybe_unused]] bool contains(const T& t) const { return t > min && t <= max; }

    /**
     * @brief Test if this interval is contained in another
     * @param other The other interval
     * @return true if *this lies inside other, false otherwise
     */
    [[maybe_unused]] bool isContainedIn(const Interval& other) const { return min >= other.min && max <= other.max; }

    /**
     * @brief Test if this interval intersects another
     * @param other The other interval
     * @return true if *this and other intersect, false otherwise
     */
    [[maybe_unused]] std::optional<Interval<T>> intersect(const Interval& other) const {
        double newMin = std::max(min, other.min);
        double newMax = std::min(max, other.max);
        return (newMin < newMax) ? std::optional(Interval(newMin, newMax)) : std::nullopt;
    }

    /**
     * @brief Compute the length of the interval
     * @return The length
     */
    [[maybe_unused]] T length() const { return max - min; }

    bool operator==(const Interval<T>& other) const { return min == other.min && max == other.max; }
    bool operator!=(const Interval<T>& other) const { return min != other.min || max != other.max; }
    /**
     * Lexicographic order for intervals
     */
    bool operator<=(const Interval<T>& other) const {
        return min < other.min || (min == other.min && max <= other.max);
    }
    bool operator>=(const Interval<T>& other) const { return other <= *this; }
    bool operator<(const Interval<T>& other) const { return !(other <= *this); }
    bool operator>(const Interval<T>& other) const { return !(*this <= other); }
};
