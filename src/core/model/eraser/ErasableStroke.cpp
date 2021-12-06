#include "ErasableStroke.h"

#include <cmath>

#include "model/Stroke.h"
#include "util/Range.h"

#include "ErasableStrokePart.h"

ErasableStroke::ErasableStroke(Stroke* stroke): stroke(stroke) {

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

    for (ErasableStrokePart& part: tmpCopy) {
        if (part.getWidth() == Point::NO_PRESSURE) {
            cairo_set_line_width(cr, w);
        } else {
            cairo_set_line_width(cr, part.getWidth());
        }

        std::vector<Point> const& pl = part.getPoints();
        cairo_move_to(cr, pl[0].x, pl[0].y);

        for (auto pointIter = pl.begin() + 1; pointIter != pl.end(); ++pointIter) {
            cairo_line_to(cr, pointIter->x, pointIter->y);
        }
        cairo_stroke(cr);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * The only public method
 */
auto ErasableStroke::erase(double x, double y, double halfEraserSize, Range* range) -> Range* {
    this->repaintRect = range;

    this->partLock.lock();
    PartList tmpCopy = parts;
    this->partLock.unlock();

    for (auto partIter = tmpCopy.begin(); partIter != tmpCopy.end();) {
        if (!erase(x, y, halfEraserSize, partIter, tmpCopy)) {
            ++partIter;
        }
    }

    this->partLock.lock();
    parts = tmpCopy;
    this->partLock.unlock();

    return this->repaintRect;
}

void ErasableStroke::addRepaintRect(double x, double y, double width, double height) {
    if (this->repaintRect) {
        this->repaintRect->addPoint(x, y);
    } else {
        this->repaintRect = new Range(x, y);
    }

    this->repaintRect->addPoint(x + width, y + height);
}

bool ErasableStroke::erase(double x, double y, double halfEraserSize, PartList::iterator& partIter, PartList& list) {
    if (partIter->getPoints().size() < 2) {
        return false;
    }

    Point eraser(x, y);

    Point a = partIter->getPoints().front();
    Point b = partIter->getPoints().back();

    if (eraser.lineLengthTo(a) < halfEraserSize * 1.2 && eraser.lineLengthTo(b) < halfEraserSize * 1.2) {
        addRepaintRect(partIter->getX(), partIter->getY(), partIter->getElementWidth(), partIter->getElementHeight());
        partIter = list.erase(partIter);

        return true;
    }

    double x1 = x - halfEraserSize;
    double x2 = x + halfEraserSize;
    double y1 = y - halfEraserSize;
    double y2 = y + halfEraserSize;

    double aX = a.x;
    double aY = a.y;
    double bX = b.x;
    double bY = b.y;

    // check first point
    if (aX >= x1 && aY >= y1 && aX <= x2 && aY <= y2) {
        bool deleteAfter = false;

        if (erasePart(x, y, halfEraserSize, partIter, list, &deleteAfter)) {
            addRepaintRect(partIter->getX(), partIter->getY(), partIter->getElementWidth(),
                           partIter->getElementHeight());
            partIter->calcSize();
        }

        if (deleteAfter) {
            partIter = list.erase(partIter);
            return true;
        }

        return false;
    }

    // check last point
    if (bX >= x1 && bY >= y1 && bX <= x2 && bY <= y2) {
        bool deleteAfter = false;

        if (erasePart(x, y, halfEraserSize, partIter, list, &deleteAfter)) {
            addRepaintRect(partIter->getX(), partIter->getY(), partIter->getElementWidth(),
                           partIter->getElementHeight());
            partIter->calcSize();
        }

        if (deleteAfter) {
            partIter = list.erase(partIter);
            return true;
        }

        return false;
    }

    double len = hypot(bX - aX, bY - aY);
    /**
     * The distance of the center of the eraser box to the line passing through (aX, aY) and (bX, bY)
     */
    double p = std::abs((x - aX) * (aY - bY) + (y - aY) * (bX - aX)) / len;

    // If the distance p of the center of the eraser box to the (full) line is in the range,
    // we check whether the eraser box is not too far from the line segment through the two points.

    if (p <= halfEraserSize) {
        double centerX = (aX + bX) / 2;
        double centerY = (aY + bY) / 2;
        double distance = hypot(x - centerX, y - centerY);

        // For the above check we imagine a circle whose center is the mid point of the two points of the stroke
        // and whose radius is half the length of the line segment plus half the diameter of the eraser box
        // plus some small padding
        // If the center of the eraser box lies within that circle then we consider it to be close enough

        distance -= halfEraserSize * std::sqrt(2);

        constexpr double PADDING = 0.1;

        if (distance <= len / 2 + PADDING) {
            bool deleteAfter = false;

            if (erasePart(x, y, halfEraserSize, partIter, list, &deleteAfter)) {
                addRepaintRect(partIter->getX(), partIter->getY(), partIter->getElementWidth(),
                               partIter->getElementHeight());
                partIter->calcSize();
            }

            if (deleteAfter) {
                partIter = list.erase(partIter);
                return true;
            }

            return false;
        }
    }

    return false;
}

auto ErasableStroke::erasePart(double x, double y, double halfEraserSize, PartList::iterator& partIter, PartList& list,
                               bool* deleteStrokeAfter) -> bool {
    bool changed = false;

    partIter->splitFor(halfEraserSize);

    double x1 = x - halfEraserSize;
    double x2 = x + halfEraserSize;
    double y1 = y - halfEraserSize;
    double y2 = y + halfEraserSize;

    /**
     * erase the beginning
     */

    std::vector<Point>& points = partIter->getPoints();

    for (auto pointIter = points.begin(); pointIter != points.end();) {
        if (pointIter->x >= x1 && pointIter->y >= y1 && pointIter->x <= x2 && pointIter->y <= y2) {
            pointIter = points.erase(pointIter);
            changed = true;
        } else {
            // only the beginning is handled here
            break;
        }
    }

    /**
     * erase the end
     */
    // ugly loop avoiding reverse_iterators
    for (auto pointIter = points.end() - 1; pointIter-- != points.begin();) {
        if (pointIter->x >= x1 && pointIter->y >= y1 && pointIter->x <= x2 && pointIter->y <= y2) {
            points.erase(pointIter);
            changed = true;
        } else {
            // only the end is handled here
            break;
        }
    }

    /**
     * handle the rest
     */

    std::vector<std::vector<Point>> splitPoints{{}};

    for (auto pointIter = points.begin(); pointIter != points.end(); ++pointIter) {
        if (pointIter->x >= x1 && pointIter->y >= y1 && pointIter->x <= x2 && pointIter->y <= y2) {
            if (!splitPoints.back().empty()) {
                splitPoints.push_back({});
            }
            changed = true;
        } else {
            splitPoints.back().push_back(*pointIter);
        }
    }
    if (splitPoints.back().empty()) {
        splitPoints.pop_back();
    }

    points.clear();
    if (!splitPoints.empty()) {
        // Replace the points of the current part with this first subpart
        points = std::move(splitPoints.front());

        PartList::iterator insertPos = std::next(partIter);

        // create data structure for all new (splitted) parts
        for (auto it = splitPoints.begin() + 1; it != splitPoints.end(); ++it) {
            // Push the other subparts
            PartList::iterator newPart = list.emplace(insertPos, partIter->getWidth());
            newPart->getPoints() = std::move(*it);
        }
    } else {
        // no parts, all deleted
        *deleteStrokeAfter = true;
    }

    return changed;
}

auto ErasableStroke::getStroke(Stroke* original) -> std::vector<std::unique_ptr<Stroke>> {
    std::vector<std::unique_ptr<Stroke>> strokeList;

    Point lastPoint(NAN, NAN);
    for (ErasableStrokePart& part: parts) {
        std::vector<Point> const& points = part.getPoints();
        if (points.size() < 2) {
            continue;
        }

        Point a = points.front();
        Point b = points.back();
        a.z = part.getWidth();

        if (!lastPoint.equalsPos(a) || strokeList.empty()) {
            if (!strokeList.empty()) {
                strokeList.back()->addPoint(lastPoint);
            }
            auto& newStroke = strokeList.emplace_back(std::make_unique<Stroke>());
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
