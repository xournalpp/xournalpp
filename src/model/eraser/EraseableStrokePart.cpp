#include "EraseableStrokePart.h"

EraseableStrokePart::EraseableStrokePart(Point a, Point b) {
    addPoint(a);
    addPoint(b);
    this->width = a.z;

    this->splitSize = 0;

    calcSize();
}

EraseableStrokePart::EraseableStrokePart(double width) {
    this->width = width;
    this->splitSize = 0;

    calcSize();
}

void EraseableStrokePart::calcSize() {
    if (points.empty()) {
        this->x = 0;
        this->y = 0;
        this->elementWidth = 0;
        this->elementHeight = 0;
        return;
    }

    double x1 = points.front().x;
    double y1 = points.front().y;
    double x2 = x1;
    double y2 = y1;

    for (const Point& p: points) {
        x1 = std::min(x1, p.x);
        x2 = std::max(x2, p.x);
        y1 = std::min(y1, p.y);
        y2 = std::max(y2, p.y);
    }

    this->x = x1;
    this->y = y1;
    this->elementWidth = x2 - x1;
    this->elementHeight = y2 - y1;
}

auto EraseableStrokePart::getX() const -> double { return this->x; }

auto EraseableStrokePart::getY() const -> double { return this->y; }

auto EraseableStrokePart::getElementWidth() const -> double { return this->elementWidth; }

auto EraseableStrokePart::getElementHeight() const -> double { return this->elementHeight; }

void EraseableStrokePart::addPoint(Point p) {
    calcSize();

    points.emplace_back(std::move(p));
}

auto EraseableStrokePart::getWidth() const -> double { return this->width; }

auto EraseableStrokePart::getPoints() -> std::vector<Point>& { return points; }
auto EraseableStrokePart::getPoints() const -> std::vector<Point> const& { return points; }

void EraseableStrokePart::clearSplitData() {
    points.erase(points.begin() + 1, points.end() - 1);  // deletes everything but endpoints
}

void EraseableStrokePart::splitFor(double halfEraserSize) {
    if (halfEraserSize == this->splitSize) {
        return;
    }

    this->splitSize = halfEraserSize;

    Point a = points.front();
    Point b = points.back();

    // nothing to do, the size is enough small
    if (a.lineLengthTo(b) <= halfEraserSize) {
        return;
    }

    clearSplitData();

    double len = a.lineLengthTo(b);
    halfEraserSize /= 2;

    while (len > halfEraserSize) {
        points.emplace(points.begin() + 1, std::move(a.lineTo(b, len)));
        len -= halfEraserSize;
    }
}
