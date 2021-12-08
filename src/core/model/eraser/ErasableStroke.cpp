#include "ErasableStroke.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <optional>

#include "model/Stroke.h"
#include "util/Point.h"
#include "util/Range.h"
#include "util/Rectangle.h"
#include "util/safe_casts.h"

namespace {

// Todo (performance): replace this with:
// std::pair<ErasableStrokeEdge, ErasableStrokeEdge> splitAt(Rectangle<double> eraserRect);

[[nodiscard]] std::vector<Point> splitFor(ErasableStrokeEdge edge, double halfEraserSize) {
    Point const& a = edge.p1;
    Point const& b = edge.p2;

    // nothing to do, the size is enough small
    auto distance = a.lineLengthTo(b);
    if (distance <= halfEraserSize) {
        return {a, b};
    }
    double cnt = std::ceil(distance / (halfEraserSize / 2));
    size_t n = static_cast<size_t>(cnt);
    std::vector<Point> points;
    points.reserve(n);
    auto dx = (b.x - a.x) / cnt;
    auto dy = (b.y - a.y) / cnt;
    for (size_t i = 0; i < n; ++i) { points.emplace_back(a.x + dx * double(i), a.y + dy * double(i)); }
    return points;
}

}  // namespace


ErasableStroke::ErasableStroke(Stroke* stroke): stroke(stroke) {
    // parts.reserve(stroke->getPointCount());
    for (int i = 1; i < stroke->getPointCount(); i++) {
        parts.emplace_back(stroke->getPoint(i - 1), stroke->getPoint(i));
    }
}

////////////////////////////////////////////////////////////////////////////////
// This is done in a Thread, every thing else in the main loop /////////////////
////////////////////////////////////////////////////////////////////////////////

void ErasableStroke::draw(cairo_t* cr) {
    this->partLock.lock();
    PartList tmpCopy = parts;
    this->partLock.unlock();

    double w = this->stroke->getWidth();

    for (auto const& part: tmpCopy) {
        if (part.p1.z == Point::NO_PRESSURE) {
            cairo_set_line_width(cr, w);
        } else {
            cairo_set_line_width(cr, part.p1.z);
        }
        cairo_move_to(cr, part.p1.x, part.p1.y);
        cairo_line_to(cr, part.p2.x, part.p2.y);
        cairo_stroke(cr);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * The only public method
 */
[[nodiscard]] auto ErasableStroke::erase(double x, double y, double halfEraserSize, Range range) -> Range {
    this->repaintRect = range;

    this->partLock.lock();
    PartList tmpCopy = parts;
    this->partLock.unlock();

    for (auto partIter = tmpCopy.begin(); partIter != tmpCopy.end();
         partIter = erase(x, y, halfEraserSize, partIter, tmpCopy)) {}

    this->partLock.lock();
    parts = tmpCopy;
    this->partLock.unlock();

    return *this->repaintRect;
}

void ErasableStroke::addRepaintRect(double x, double y, double width, double height) {
    if (this->repaintRect) {
        this->repaintRect->addPoint(x, y);
    } else {
        this->repaintRect.emplace(x, y);
    }

    this->repaintRect->addPoint(x + width, y + height);
}

[[nodiscard]] auto erasePart(double x, double y, double halfEraserSize, PartList::iterator const& partIter,
                             PartList& list) -> PartList {

    double x1 = x - halfEraserSize;
    double x2 = x + halfEraserSize;
    double y1 = y - halfEraserSize;
    double y2 = y + halfEraserSize;
    auto valid_point = [x1, x2, y1, y2](const Point& point) {
        return !(point.x >= x1 && point.y >= y1 && point.x <= x2 && point.y <= y2);
    };

    // Todo (optimize): fixup
    std::vector<Point> const& points = splitFor(*partIter, halfEraserSize);

    auto const begin_skip_iter =
            std::find_if(points.begin(), points.end(), [&](const Point& point) { return valid_point(point); });
    auto const end_skip_iter =
            std::find_if(points.rbegin(), std::reverse_iterator(begin_skip_iter), [&](const Point& point) {
                return valid_point(point);
            }).base();
    /**
     * handle the rest
     */
    PartList splitPoints{};
    if (begin_skip_iter == end_skip_iter) {
        return splitPoints;
    }
    // splitPoints.reserve(size_t(std::distance(begin_skip_iter, end_skip_iter) - 1));

    Point prevPoint = *begin_skip_iter;

    // The split is suboptimal, and can be calculated.
    for (auto pointIter = begin_skip_iter + 1; pointIter != end_skip_iter; prevPoint = *pointIter, ++pointIter) {
        if (auto newPoint = *pointIter; valid_point(prevPoint) && valid_point(newPoint)) {
            splitPoints.emplace_back(prevPoint, newPoint);
        }
    }
    // Todo (performance): fixup
    if (auto changed = points.size() - 1 != splitPoints.size(); changed) {
        return splitPoints;
    } else {
        return PartList{*partIter};
    }
}

[[nodiscard]] auto ErasableStroke::erase(double x, double y, double halfEraserSize, PartList::iterator const& partIter,
                                         PartList& list) -> PartList::iterator {
    utl::Point<double> const& eraser{x, y};
    utl::Point<double> const& a{partIter->p1.x, partIter->p1.y};
    utl::Point<double> const& b{partIter->p2.x, partIter->p2.y};
    Rectangle<double> eraserRect{x - halfEraserSize, y - halfEraserSize, 2 * halfEraserSize, 2 * halfEraserSize};
    Rectangle<double> lineRect{a.x, a.y, b.x - a.x, b.y - a.y};
    Rectangle<double> validRect{a.x - halfEraserSize, a.y - halfEraserSize,  //
                                b.x - a.x + 2 * halfEraserSize, b.y - a.y + 2 * halfEraserSize};

    bool firstPoint = eraserRect.contains(a);
    bool secondPoint = eraserRect.contains(b);

    if (eraser.distance(a) < halfEraserSize * 1.2 && eraser.distance(b) < halfEraserSize * 1.2) {
        addRepaintRect(a.x, a.y, b.x - a.x, b.y - a.y);
        return list.erase(partIter);
    }

    auto isMidPoint = [&]() {
        // The distance of the center of the eraser box to the line passing through (aX, aY) and (bX, bY)
        double distancePts = a.distance(b);
        double distanceToLine = std::abs((x - a.x) * (a.y - b.y) + (y - a.y) * (b.x - a.x)) / distancePts;
        auto intersection = lineRect.intersects(eraserRect);
        return intersection && distanceToLine <= halfEraserSize;
    };

    if (firstPoint || secondPoint || isMidPoint()) {
        addRepaintRect(a.x, a.y, b.x - a.x, b.y - a.y);
        auto parts = erasePart(x, y, halfEraserSize, partIter, list);
        if (!parts.empty()) {
            *partIter = parts.front();
            auto return_iter = list.insert(std::next(partIter), std::next(begin(parts)), end(parts));
            return std::next(return_iter, as_signed(parts.size() - 1));
        } else {
            return list.erase(partIter);
        }
    }
    return std::next(partIter);
}

auto ErasableStroke::getStroke(Stroke* original) -> std::vector<std::unique_ptr<Stroke>> {
    std::vector<std::unique_ptr<Stroke>> strokeList;

    Point lastPoint(NAN, NAN);
    for (auto& part: parts) {
        Point const& a = part.p1;
        Point const& b = part.p2;

        if (!lastPoint.equalsPos(a) || strokeList.empty()) {
            if (!strokeList.empty()) {
                strokeList.back()->addPoint(lastPoint);
            }
            auto& newStroke = strokeList.emplace_back(std::make_unique<Stroke>(parts.size() / 8));
            newStroke->setColor(original->getColor());
            newStroke->setToolType(original->getToolType());
            newStroke->setLineStyle(original->getLineStyle());
            newStroke->setWidth(original->getWidth());
        }
        strokeList.back()->addPoint(a);
        lastPoint = b;
    }
    if (!strokeList.empty()) {
        strokeList.back()->addPoint(lastPoint);
    }

    return strokeList;
}
