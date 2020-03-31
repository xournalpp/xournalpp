#include "Rectangle.h"

#include <cmath>

#include "Range.h"

template <class T>
Rectangle<T>::Rectangle() = default;

template <class T>
Rectangle<T>::Rectangle(T x, T y, T width, T height): x(x), y(y), width(width), height(height) {}

template <class T>
Rectangle<T>::Rectangle(const Range& rect):
        x(rect.getX()), y(rect.getY()), width(rect.getWidth()), height(rect.getHeight()) {}

template <class T>
auto Rectangle<T>::intersects(const Rectangle<T>& other, Rectangle<T>* dest) const -> bool {
    T destX = NAN, destY = NAN;
    T destW = NAN, destH = NAN;

    bool returnVal = false;

    destX = std::max(this->x, other.x);
    destY = std::max(this->y, other.y);
    destW = std::min(this->x + this->width, other.x + other.width) - destX;
    destH = std::min(this->y + this->height, other.y + other.height) - destY;

    if (destW > 0 && destH > 0) {
        if (dest) {
            dest->x = destX;
            dest->y = destY;
            dest->width = destW;
            dest->height = destH;
        }
        returnVal = true;
    } else if (dest) {
        dest->width = 0;
        dest->height = 0;
    }

    return returnVal;
}

template <class T>
void Rectangle<T>::add(T x, T y, T width, T height) {
    if (width <= 0 || height <= 0) {
        return;
    }

    T x1 = std::min(this->x, x);
    T y1 = std::min(this->y, y);

    T x2 = std::max(this->x + this->width, x + width);
    T y2 = std::max(this->y + this->height, y + height);

    this->x = x1;
    this->y = y1;
    this->width = x2 - x1;
    this->height = y2 - y1;
}

template <class T>
void Rectangle<T>::add(const Rectangle<T>& other) {
    add(other.x, other.y, other.width, other.height);
}

template <class T>
auto Rectangle<T>::translated(T dx, T dy) const -> Rectangle<T> {
    return Rectangle<T>(this->x + dx, this->y + dy, this->width, this->height);
}

template <class T>
auto Rectangle<T>::intersect(const Rectangle<T>& other) const -> Rectangle<T> {
    double x1 = std::max(this->x, other.x);
    double y1 = std::max(this->y, other.y);

    double x2 = std::min(this->x + this->width, other.x + other.width);
    double y2 = std::min(this->y + this->height, other.y + other.height);

    return Rectangle(x1, y1, x2 - x1, y2 - y1);
}

template <class T>
auto Rectangle<T>::operator*=(T factor) -> Rectangle<T>& {
    x *= factor;
    y *= factor;

    width *= factor;
    height *= factor;

    return *this;
}

template <class T>
auto Rectangle<T>::area() const -> T {
    return width * height;
}

template class Rectangle<double>;
template class Rectangle<int>;
