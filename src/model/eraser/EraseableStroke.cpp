#include "EraseableStroke.h"

#include <cmath>

#include "model/Stroke.h"

#include "LoopUtil.h"
#include "Range.h"
#include "UnionOfIntervals.h"

#ifdef EXTRA_CAREFUL
#include <iostream>
#include <sstream>
#endif

EraseableStroke::EraseableStroke(Stroke* stroke): stroke(stroke) {}

////////////////////////////////////////////////////////////////////////////////
// This is done in a Thread, every thing else in the main loop /////////////////
////////////////////////////////////////////////////////////////////////////////

void EraseableStroke::draw(cairo_t* cr) {  // no pressure
    std::vector<Interval<Spline::Parameter>> sections;
    {  // lock_guard scope
        std::lock_guard<std::mutex> lock(sectionsMutex);
        sections = remainingSections.cloneToIntervalVector();
    }  // Release the mutex

    cairo_set_line_width(cr, this->stroke->getWidth());
    
    Spline::SegmentIteratable segments = this->stroke->spline.segments();

    for (auto&& interval: sections) {
        // Similar to Spline::cloneSection (without the cloning)
        auto it = segments.iteratorAt(interval.min.index);

        if (interval.max.index == interval.min.index) {
            SplineSegment seg = it->getSubsegment(interval.min.t, interval.max.t);
            seg.draw(cr);
        } else {
            SplineSegment firstSegment = it->subdivide(interval.min.t).second;

            firstSegment.draw(cr);
            it++;
            auto itEnd = segments.iteratorAt(interval.max.index);
            for (; it != itEnd; it++) {
                cairo_curve_to(cr, it->firstControlPoint.x, it->firstControlPoint.y, it->secondControlPoint.x,
                               it->secondControlPoint.y, it->secondKnot.x, it->secondKnot.y);
            }

            SplineSegment seg = it->subdivide(interval.max.t).first;
            cairo_curve_to(cr, seg.firstControlPoint.x, seg.firstControlPoint.y, seg.secondControlPoint.x,
                           seg.secondControlPoint.y, seg.secondKnot.x, seg.secondKnot.y);
        }
        cairo_stroke(cr);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////


auto EraseableStroke::beginErasure(double x, double y, double halfEraserSize, Range* range) -> Range* {
    if (stroke->isSpline()) {
        const Spline& spline = stroke->getSpline();
        if (spline.size() != 0) {
            std::lock_guard<std::mutex> lock(sectionsMutex);
            remainingSections.unite(this->stroke->intersectionParameters);
            remainingSections.complement({0, 0.0}, {spline.size() - 1, 1.0});
            this->stroke->intersectionParameters = {};
        }
    }
    addRepaintRect(x - halfEraserSize, y - halfEraserSize, 2 * halfEraserSize, 2 * halfEraserSize);
    return this->repaintRect;
}

/**
 * The only public method
 */
auto EraseableStroke::erase(double x, double y, double halfEraserSize, Range* range) -> Range* {
    if (stroke->isSpline()) {
        const Spline& spline = stroke->getSpline();
        if (spline.size() != 0) {
            std::vector<Interval<Spline::Parameter>> sections;
            {  // lock_guard scope
                std::lock_guard<std::mutex> lock(sectionsMutex);
                sections = remainingSections.cloneToIntervalVector();
            }  // Release the mutex

            if (sections.empty()) {
                /** Nothing left to erase! **/
                return range;
            } else {
                Rectangle<double> eraserBox(x - halfEraserSize, y - halfEraserSize, 2 * halfEraserSize,
                                            2 * halfEraserSize);

                std::vector<Spline::Parameter> inter = spline.intersectWithRectangle(eraserBox);
                UnionOfIntervals<Spline::Parameter> newErasedSections;

                /**
                 * Determine which stroke segments are still (partially) visible
                 */
                std::vector<Interval<size_t>> indexIntervals;

                for_first_then_each(
                        sections,
                        [&indexIntervals](const Interval<Spline::Parameter>& p) {
                            indexIntervals.emplace_back(p.min.index, p.max.index);
                        },
                        [&indexIntervals](const Interval<Spline::Parameter>& p) {
                            if (indexIntervals.back().max + 1 >= p.min.index) {
                                indexIntervals.back().envelop(p.max.index);
                            } else {
                                indexIntervals.emplace_back(p.min.index, p.max.index);
                            }
                        });

                for (auto&& i: indexIntervals) {
                    std::vector<Spline::Parameter> res = spline.intersectWithRectangle(eraserBox, i.min, i.max);
                    newErasedSections.unite(res);
                }

                if (!newErasedSections.empty()) {
#ifdef EXTRA_CAREFUL
                    if (newErasedSections.getData().size() % 2) {
                        g_warning("EraseableStroke::erase: newErasedSections of odd length. This is a bug.");
                        return this->repaintRect;
                    }
                    std::stringstream ss;
                    ss << "newErasedSections :";
                    bool even = true;
                    for (auto&& p: newErasedSections.getData()) {
                        ss << (even ? " [" : " -- ");
                        ss << "(" << p.index << " ; " << p.t << ")";
                        ss << (even ? "" : "] ");
                        even = !even;
                    }
                    ss << "\n  * Complement :";
                    newErasedSections.complement({0, 0.0}, {spline.size() - 1, 1.0});
                    even = true;
                    for (auto&& p: newErasedSections.getData()) {
                        ss << (even ? " [" : " -- ");
                        ss << "(" << p.index << " ; " << p.t << ")";
                        ss << (even ? "" : "] ");
                        even = !even;
                    }
                    ss << "\nremainingSections :";
                    for (auto&& sect: sections) {
                        ss << " [(" << sect.min.index << " ; " << sect.min.t << ") -";
                        ss << "- (" << sect.max.index << " ; " << sect.max.t << ")]";
                    }

                    {  // lock_guard scope
                        std::lock_guard<std::mutex> lock(sectionsMutex);
                        remainingSections.intersect(newErasedSections.getData());
                        ss << "\n  * After :";
                        even = true;
                        for (auto&& p: remainingSections.getData()) {
                            ss << (even ? " [" : " -- ");
                            ss << "(" << p.index << " ; " << p.t << ")";
                            ss << (even ? "" : "] ");
                            even = !even;
                        }
                    }  // Release the mutex
//                     std::cout << ss.str() << "\n";
#else
                    newErasedSections.complement({0, 0.0}, {spline.size() - 1, 1.0});
                    {  // lock_guard scope
                        std::lock_guard<std::mutex> lock(sectionsMutex);
                        remainingSections.intersect(newErasedSections.getData());
                    }  // Release the mutex
#endif
                }
            }
        }
    }

    this->repaintRect = range;
    addRepaintRect(x - halfEraserSize, y - halfEraserSize, 2 * halfEraserSize, 2 * halfEraserSize);

    return this->repaintRect;
}

void EraseableStroke::addRepaintRect(double x, double y, double width, double height) {
    if (this->repaintRect) {
        this->repaintRect->addPoint(x, y);
    } else {
        this->repaintRect = new Range(x, y);
    }

    this->repaintRect->addPoint(x + width, y + height);
}

#ifdef OBSOLETE
void EraseableStroke::erase(double x, double y, double halfEraserSize, EraseableStrokePart* part, PartList* list) {
    if (part->points->next == nullptr) {
        return;
    }

    Point eraser(x, y);

    auto* a = static_cast<Point*>(g_list_first(part->points)->data);
    auto* b = static_cast<Point*>(g_list_last(part->points)->data);

    if (eraser.lineLengthTo(*a) < halfEraserSize * 1.2 && eraser.lineLengthTo(*b) < halfEraserSize * 1.2) {
        list->data = g_list_remove(list->data, part);
        addRepaintRect(part->getX(), part->getY(), part->getElementWidth(), part->getElementHeight());

        delete part;
        return;
    }

    double x1 = x - halfEraserSize;
    double x2 = x + halfEraserSize;
    double y1 = y - halfEraserSize;
    double y2 = y + halfEraserSize;

    double aX = a->x;
    double aY = a->y;
    double bX = b->x;
    double bY = b->y;

    // check first point
    if (aX >= x1 && aY >= y1 && aX <= x2 && aY <= y2) {
        bool deleteAfter = false;

        if (erasePart(x, y, halfEraserSize, part, list, &deleteAfter)) {
            addRepaintRect(part->getX(), part->getY(), part->getElementWidth(), part->getElementHeight());
            part->calcSize();
        }

        if (deleteAfter) {
            delete part;
        }

        return;
    }

    // check last point
    if (bX >= x1 && bY >= y1 && bX <= x2 && bY <= y2) {
        bool deleteAfter = false;

        if (erasePart(x, y, halfEraserSize, part, list, &deleteAfter)) {
            addRepaintRect(part->getX(), part->getY(), part->getElementWidth(), part->getElementHeight());
            part->calcSize();
        }

        if (deleteAfter) {
            delete part;
        }

        return;
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

            if (erasePart(x, y, halfEraserSize, part, list, &deleteAfter)) {
                addRepaintRect(part->getX(), part->getY(), part->getElementWidth(), part->getElementHeight());
                part->calcSize();
            }

            if (deleteAfter) {
                delete part;
            }

            return;
        }
    }
}

auto EraseableStroke::erasePart(double x, double y, double halfEraserSize, EraseableStrokePart* part, PartList* list,
                                bool* deleteStrokeAfter) -> bool {
    bool changed = false;

    part->splitFor(halfEraserSize);

    double x1 = x - halfEraserSize;
    double x2 = x + halfEraserSize;
    double y1 = y - halfEraserSize;
    double y2 = y + halfEraserSize;

    /**
     * erase the beginning
     */
    for (GList* l = part->getPoints(); l != nullptr;) {
        auto* p = static_cast<Point*>(l->data);
        l = l->next;
        if (p->x >= x1 && p->y >= y1 && p->x <= x2 && p->y <= y2) {
            part->points = g_list_remove(part->points, p);
            delete p;
            changed = true;
        } else {
            // only the beginning is handled here
            break;
        }
    }

    /**
     * erase the end
     */
    for (GList* l = g_list_last(part->getPoints()); l != nullptr;) {
        auto* p = static_cast<Point*>(l->data);
        l = l->prev;
        if (p->x >= x1 && p->y >= y1 && p->x <= x2 && p->y <= y2) {
            part->points = g_list_remove(part->points, p);
            delete p;
            changed = true;
        } else {
            // only the end is handled here
            break;
        }
    }

    /**
     * handle the rest
     */

    GList* lists = nullptr;
    GList* current = nullptr;

    for (GList* l = part->points; l != nullptr;) {
        auto* p = static_cast<Point*>(l->data);
        l = l->next;
        if (p->x >= x1 && p->y >= y1 && p->x <= x2 && p->y <= y2) {
            part->points = g_list_remove(part->points, p);
            delete p;
            if (current) {
                lists = g_list_append(lists, current);
                current = nullptr;
            }
            changed = true;
        } else {
            current = g_list_append(current, p);
        }
    }

    if (current) {
        lists = g_list_append(lists, current);
        current = nullptr;
    }

    g_list_free(part->points);
    part->points = nullptr;
    if (lists) {
        part->points = static_cast<GList*>(lists->data);
        lists = g_list_delete_link(lists, lists);

        int pos = g_list_index(list->data, part) + 1;

        // create data structure for all new (splitted) parts
        for (GList* l = lists; l != nullptr; l = l->next) {
            auto* newPart = new EraseableStrokePart(part->width);
            newPart->points = static_cast<GList*>(l->data);
            list->data = g_list_insert(list->data, newPart, pos++);
        }

        g_list_free(lists);
    } else {
        // no parts, all deleted
        list->data = g_list_remove(list->data, part);
        *deleteStrokeAfter = true;
    }

    return changed;
}

auto EraseableStroke::getStroke(Stroke* original) -> GList* {
    GList* list = nullptr;

    Stroke* s = nullptr;
    Point lastPoint(NAN, NAN);
    for (GList* l = this->parts->data; l != nullptr; l = l->next) {
        auto* p = static_cast<EraseableStrokePart*>(l->data);
        GList* points = p->getPoints();
        if (g_list_length(points) < 2) {
            continue;
        }

        Point a = *(static_cast<Point*>(g_list_first(points)->data));
        Point b = *(static_cast<Point*>(g_list_last(points)->data));
        a.z = p->width;

        if (!lastPoint.equalsPos(a) || s == nullptr) {
            if (s) {
                s->addPoint(lastPoint);
            }
            s = new Stroke();
            s->setColor(original->getColor());
            s->setToolType(original->getToolType());
            s->setLineStyle(original->getLineStyle());
            s->setWidth(original->getWidth());
            list = g_list_append(list, s);
        }
        s->addPoint(a);
        lastPoint = b;
    }
    if (s) {
        s->addPoint(lastPoint);
    }

    return list;
}
#endif

auto EraseableStroke::getStrokes() -> std::vector<Stroke*> {
    std::vector<Stroke*> strokes;

    const std::list<Spline::Parameter>& data = remainingSections.getData();

    strokes.reserve(data.size() / 2);

    auto itLowerBound = data.begin();
    auto itUpperBound = itLowerBound;

    while (itLowerBound != data.end()) {
        itUpperBound = std::next(itLowerBound);
        Stroke* s = new Stroke();
        s->setColor(stroke->getColor());
        s->setToolType(stroke->getToolType());
        s->setLineStyle(stroke->getLineStyle());
        s->setWidth(stroke->getWidth());
        s->spline = stroke->spline.cloneSection(*itLowerBound, *itUpperBound);
        s->splineComputed = true;
        s->pointsFromSpline();
        strokes.push_back(s);
        itLowerBound = std::next(itUpperBound);
    }
    return strokes;
}
