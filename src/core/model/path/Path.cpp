#include "Path.h"

#include <algorithm>  // for transform
#include <cmath>      // for sqrt, abs
#include <memory>

#include <glib.h>  // for g_warning

#include "model/Point.h"
#include "model/eraser/PaddedBox.h"
#include "util/Assert.h"
#include "util/Interval.h"
#include "util/SmallVector.h"

using xoj::util::Rectangle;

const std::vector<Point>& Path::getData() const { return data; }

const Point& Path::getLastKnot() const {
    xoj_assert(!this->data.empty());
    return this->data.back();
}

const Point& Path::getFirstKnot() const {
    xoj_assert(!this->data.empty());
    return this->data.front();
}

void Path::setFirstKnotPressure(double pressure) {
    xoj_assert(!this->data.empty());
    this->data.front().z = pressure;
}

std::vector<double> Path::getPressureValues() const {
    std::vector<double> result;
    result.reserve(this->data.size());
    std::transform(this->data.begin(), this->data.end(), std::back_inserter(result),
                   [](const Point& p) { return p.z; });
    return result;
}

void Path::setPressureValues(const std::vector<double>& pressures) {
    if (pressures.size() != this->data.size()) {
        g_warning("invalid pressure point count: %zu, expected %zu", pressures.size(), this->data.size());

        if (pressures.size() > this->data.size()) {
            std::transform(this->data.cbegin(), this->data.cend(), pressures.cbegin(), this->data.begin(),
                           [](Point pt, double p) { return Point(pt.x, pt.y, p); });
            return;
        }
    }

    std::transform(pressures.cbegin(), pressures.cend(), this->data.cbegin(), this->data.begin(),
                   [](double p, Point pt) { return Point(pt.x, pt.y, p); });
}

auto Path::intersectWithPaddedBox(const PaddedBox& box) const -> IntersectionParametersContainer {
    if (this->nbSegments() == 0) {
        if (!this->data.empty() == 1 && this->data.back().isInside(box.getInnerRectangle())) {
            IntersectionParametersContainer result;
            result.emplace_back(0U, 0.0);
            result.emplace_back(0U, 0.0);
            return result;
        }
        return {};
    }
    return this->intersectWithPaddedBox(box, 0, this->nbSegments() - 1);
}

// std::vector<Path::Parameter> Path::intersectWithPaddedBox(const PaddedBox& box, size_t firstIndex,
//                                                                          size_t lastIndex) const {
//     xoj_assert(firstIndex <= lastIndex && lastIndex < this->nbSegments());
//
//     const auto innerBox = box.getInnerRectangle();
//     const auto outerBox = box.getOuterRectangle();
//
//     IntersecterContext context;
//     context.index = firstIndex;
//
//
//     /**
//     * Initialize with the half tangent at the first point
//     */
//     {  // Scope for initialization at first point
//     auto [firstPoint, controlPoint] = this->getHalfTangent(firstIndex, LEFT);
//     context.isInsideOuter = firstPoint.isInside(outerBox);
//
//     if (firstPoint.isInside(innerBox)) {
//         /**
//         * The stroke starts in the eraser box
//         * OR
//         * The half line starting from the second point and passing by the first point intersects the eraser
//         * and the first point is in the padded eraser box
//         *
//         * Erasing the tip of the stroke. Add a fake intersection parameter
//         */
//         context.result.emplace_back(firstIndex, 0.0);
//         context.wentInsideInner = true;
//     } else if (context.isInsideOuter) {
//         std::optional<Interval<double>> innerLineIntersections = intersectLineWithRectangle(firstPoint, controlPoint,
//         innerBox); if (innerLineIntersections && innerLineIntersections.value().max <= 0.0) {
//             /**
//             * The half tangent at the first point intersects the eraser and the first point is in the padding.
//             *
//             * Erasing the tip of the stroke. Add a fake intersection parameter
//             */
//             context.result.emplace_back(firstIndex, 0.0);
//             context.wentInsideInner = true;
//         }
//     }
//     }  // end of scope
//
//     this->intersectWithPaddedBoxMainLoop(context, firstIndex, lastIndex, innerBox, outerBox);
//
//     if (context.result.size() % 2) {
//         // Same as with the first point
//         auto [controlPoint,lastPoint] = this->getHalfTangent(lastIndex, RIGHT);
//
//         if (lastPoint.isInside(innerBox)) {
//             /**
//              * The stroke ends in the eraser box
//              *
//              * Erasing the tip of the stroke. Add a fake intersection parameter
//              */
//             context.result.emplace_back(context.index - 1, 1.0);
//             context.wentInsideInner = true;
//         } else {
//             xoj_assert(lastPoint.isInside(outerBox));
//             // The last point has to be in the padding!
//             std::optional<Interval<double>> innerLineIntersections = intersectLineWithRectangle(controlPoint,
//             lastPoint, innerBox); if (innerLineIntersections && innerLineIntersections.value().min > 1.0) {
//                 /**
//                  * The half tangent at the last point intersects the eraser and the last point is in the padding.
//                  *
//                  * Erasing the tip of the stroke. Add a fake intersection parameter
//                  */
//                 context.result.emplace_back(context.index - 1, 1.0);
//                 context.wentInsideInner = true;
//             } else {
//                 /**
//                  * The stroke ends in the padding but its prolongation does not hit the eraser box
//                  *
//                  * Drop the last intersection (corresponding to going in the padding).
//                  */
//                 context.result.pop_back();
//             }
//         }
//     }
//     return context.result;
// }
//
// void Path::IntersecterContext::appendSegmentIntersection(const std::vector<double>& innerIntersections, const
// std::vector<double>& outerIntersections) {
//     auto itInner = innerIntersections.begin();
//     auto itInnerEnd = innerIntersections.end();
//     auto itOuter = outerIntersections.begin();
//     auto itOuterEnd = outerIntersections.end();
//     while (itOuter != itOuterEnd) {
//         if (itInner != itInnerEnd && *itInner < *itOuter) {
//             wentInsideInner = true;
//             ++itInner;
//         } else {
//             if (isInsideOuter) {
//                 // Leaving the padded box
//                 if (wentInsideInner) {
//                     /**
//                      * The stroke went in and out of the padded box and touched the eraser box.
//                      * Erase this section
//                      */
//                     result.emplace_back(index, *itOuter);
//                     wentInsideInner = false;
//                 } else {
//                     /**
//                      * The stroke went in and out of the padding without touching the eraser box.
//                      * This section should not be erased
//                      */
//                     if (!result.empty()) {
//                         result.pop_back();
//                     }
//                 }
//                 isInsideOuter = false;
//             } else {
//                 // Going in the padded box
//                 result.emplace_back(index, *itOuter);
//                 isInsideOuter = true;
//             }
//             ++itOuter;
//         }
//     }
//     if (itInner != itInnerEnd) {
//         wentInsideInner = true;
//     }
//     ++index;
// }
//
//
// std::vector<Path::Parameter> Path::intersectWithRectangle(const Rectangle<double>& rectangle) const {
//     return this->intersectWithRectangle(rectangle, 0, this->nbSegments() - 1);
// }

bool Path::empty() const { return data.empty(); }

void Path::clear() { data.clear(); }

void Path::freeUnusedPointItems() { data = {data.begin(), data.end()}; }

void Path::move(double dx, double dy) {
    for (Point& p: data) {
        p.x += dx;
        p.y += dy;
    }
}

cairo_matrix_t Path::rotate(double x0, double y0, double th) {
    cairo_matrix_t rotMatrix;
    cairo_matrix_init_identity(&rotMatrix);
    cairo_matrix_translate(&rotMatrix, x0, y0);
    cairo_matrix_rotate(&rotMatrix, th);
    cairo_matrix_translate(&rotMatrix, -x0, -y0);

    for (auto&& p: data) {
        cairo_matrix_transform_point(&rotMatrix, &p.x, &p.y);
    }

    return rotMatrix;
}

std::pair<cairo_matrix_t, double> Path::scale(double x0, double y0, double fx, double fy, double rotation,
                                              bool restoreLineWidth) {
    std::pair<cairo_matrix_t, double> result;
    cairo_matrix_t* scaleMatrix = &(result.first);
    cairo_matrix_init_identity(scaleMatrix);
    cairo_matrix_translate(scaleMatrix, x0, y0);
    cairo_matrix_rotate(scaleMatrix, rotation);
    cairo_matrix_scale(scaleMatrix, fx, fy);
    cairo_matrix_rotate(scaleMatrix, -rotation);
    cairo_matrix_translate(scaleMatrix, -x0, -y0);

    if (restoreLineWidth) {
        for (auto&& p: data) {
            cairo_matrix_transform_point(scaleMatrix, &p.x, &p.y);
        }
        result.second = 1.0;
    } else {
        double fz = std::sqrt(std::abs(fx * fy));
        for (auto&& p: data) {
            cairo_matrix_transform_point(scaleMatrix, &p.x, &p.y);
            p.z *= fz;
        }
        result.second = fz;
    }
    return result;
}

bool Path::isPointOnBoundary(const Point& p, const Rectangle<double>& r) {
    return ((p.x == r.x || p.x == r.x + r.width) && p.y >= r.y && p.y <= r.y + r.height) ||
           ((p.y == r.y || p.y == r.y + r.height) && p.x >= r.x && p.x <= r.x + r.width);
}


std::optional<Interval<double>> Path::intersectLineWithRectangle(const Point& p, const Point& q,
                                                                 const Rectangle<double>& rectangle) {
    if (p.x == q.x) {
        // Vertical segment
        double x = p.x;
        if (p.y == q.y || x <= rectangle.x || x >= rectangle.x + rectangle.width) {
            return std::nullopt;
        }
        double norm = 1.0 / (q.y - p.y);
        double tUp = (rectangle.y - p.y) * norm;
        double tBottom = tUp + rectangle.height * norm;
        return Interval<double>::getInterval(tUp, tBottom);
    }

    if (p.y == q.y) {
        // Horizontal segment
        double y = p.y;
        if (y <= rectangle.y || y >= rectangle.y + rectangle.height) {
            return std::nullopt;
        }
        double norm = 1.0 / (q.x - p.x);
        double tUp = (rectangle.x - p.x) * norm;
        double tBottom = tUp + rectangle.width * norm;
        return Interval<double>::getInterval(tUp, tBottom);
    }

    // Generic case
    double norm = 1.0 / (q.y - p.y);
    double tUp = (rectangle.y - p.y) * norm;
    double tBottom = tUp + rectangle.height * norm;
    Interval<double> verticalIntersections = Interval<double>::getInterval(tUp, tBottom);

    norm = 1.0 / (q.x - p.x);
    tUp = (rectangle.x - p.x) * norm;
    tBottom = tUp + rectangle.width * norm;
    Interval<double> horizontalIntersections = Interval<double>::getInterval(tUp, tBottom);

    return verticalIntersections.intersect(horizontalIntersections);
}
