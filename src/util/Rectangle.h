/*
 * Xournal++
 *
 * A Rectangle<double> with double precision
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <vector>

#include "XournalType.h"

class Range;

template <class T>
class Rectangle {
public:
    Rectangle();
    explicit Rectangle(const Range& rect);
    Rectangle(T x, T y, T width, T height);

public:
    /**
     * Returns whether this rectangle intersects another
     *
     * @param other the other rectangle
     * @param dest  if this is not nullptr, the rectangle will be modified to contain the intersection
     *
     * @return whether the rectangles intersect
     */
    bool intersects(const Rectangle<T>& other, Rectangle<T>* dest = nullptr) const;

    /**
     * Computes the union of this rectangle with the one given by the parameters
     */
    void add(T x, T y, T width, T height);

    /**
     * Returns a new Rectangle with an offset specified
     * by the function arguments
     *
     */
    Rectangle<T> translated(T dx, T dy) const;

    /**
     * Same as the above, provided for convenience
     */
    void add(const Rectangle<T>& other);

    Rectangle<T> intersect(const Rectangle<T>& other) const;

    Rectangle<T>& operator*=(T factor);

    T area() const;

public:
    T x = 0;
    T y = 0;
    T width = 0;
    T height = 0;
};
