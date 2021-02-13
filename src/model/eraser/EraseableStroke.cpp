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
    std::vector<Interval<Path::Parameter>> sections;
    {  // lock_guard scope
        std::lock_guard<std::mutex> lock(sectionsMutex);
        sections = remainingSections.cloneToIntervalVector();
    }  // Release the mutex

    cairo_set_line_width(cr, this->stroke->getWidth());

    if (stroke->path->getType() == Path::PIECEWISE_LINEAR) {
        const std::vector<Point>& data = stroke->path->getData();
        for (auto&& interval: sections) {
            Point p = stroke->path->getPoint(interval.min);
            cairo_move_to(cr, p.x, p.y);

            auto endIt = data.cbegin() + (std::ptrdiff_t)interval.max.index + 1;
            for (auto it = data.cbegin() + (std::ptrdiff_t)interval.min.index + 1; it != endIt; ++it) {
                cairo_line_to(cr, it->x, it->y);
            }

            Point q = stroke->path->getPoint(interval.max);
            cairo_line_to(cr, q.x, q.y);
            cairo_stroke(cr);
        }
    } else {  //  SPLINE
        Spline::SegmentIteratable segments = this->stroke->getSpline().segments();

        for (auto&& interval: sections) {
            // Similar to Spline::cloneSection (without the cloning)
            auto it = segments.iteratorAt(interval.min.index);

            if (interval.max.index == interval.min.index) {
                SplineSegment seg = it->getSubsegment(interval.min.t, interval.max.t);
                seg.draw(cr);
            } else {
                SplineSegment firstSegment = it->subdivide(interval.min.t).second;

                firstSegment.draw(cr);
                ++it;
                auto endIt = segments.iteratorAt(interval.max.index);
                for (; it != endIt; ++it) {
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
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////


auto EraseableStroke::beginErasure(double x, double y, double halfEraserSize, Range* range) -> Range* {
    this->repaintRect = range;
    if (this->stroke->path->nbSegments() != 0) {
        std::lock_guard<std::mutex> lock(this->sectionsMutex);
        this->remainingSections.unite(this->stroke->intersectionParameters);
        this->remainingSections.complement({0, 0.0}, {this->stroke->path->nbSegments() - 1, 1.0});
        this->stroke->intersectionParameters = {};
    }
    addRepaintRect(x - halfEraserSize, y - halfEraserSize, 2 * halfEraserSize, 2 * halfEraserSize);
    return this->repaintRect;
}

/**
 * The only public method
 */
auto EraseableStroke::erase(double x, double y, double halfEraserSize, Range* range) -> Range* {
    auto& path = this->stroke->path;
    if (path->nbSegments() != 0) {
        std::vector<Interval<Path::Parameter>> sections;
        {  // lock_guard scope
            std::lock_guard<std::mutex> lock(sectionsMutex);
            sections = remainingSections.cloneToIntervalVector();
        }  // Release the mutex

        if (sections.empty()) {
            /** Nothing left to erase! **/
            return range;
        } else {
            Rectangle<double> eraserBox(x - halfEraserSize, y - halfEraserSize, 2 * halfEraserSize, 2 * halfEraserSize);

            UnionOfIntervals<Path::Parameter> newErasedSections;

            /**
             * Determine which stroke segments are still (partially) visible
             */
            std::vector<Interval<size_t>> indexIntervals;

            for_first_then_each(
                    sections,
                    [&indexIntervals](const Interval<Path::Parameter>& p) {
                        indexIntervals.emplace_back(p.min.index, p.max.index);
                    },
                    [&indexIntervals](const Interval<Path::Parameter>& p) {
                        if (indexIntervals.back().max + 1 >= p.min.index) {
                            indexIntervals.back().envelop(p.max.index);
                        } else {
                            indexIntervals.emplace_back(p.min.index, p.max.index);
                        }
                    });

            for (auto&& i: indexIntervals) {
                std::vector<Path::Parameter> res = path->intersectWithRectangle(eraserBox, i.min, i.max);
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
                newErasedSections.complement({0, 0.0}, {path->nbSegments() - 1, 1.0});
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
                newErasedSections.complement({0, 0.0}, {path->nbSegments() - 1, 1.0});
                {  // lock_guard scope
                    std::lock_guard<std::mutex> lock(sectionsMutex);
                    remainingSections.intersect(newErasedSections.getData());
                }  // Release the mutex
#endif
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

auto EraseableStroke::getStrokes() -> std::vector<Stroke*> {
    std::vector<Stroke*> strokes;

    std::vector<Interval<Path::Parameter>> sections;
    {  // lock_guard scope
        std::lock_guard<std::mutex> lock(sectionsMutex);
        sections = remainingSections.cloneToIntervalVector();
    }  // Release the mutex

    strokes.reserve(sections.size());
    for (auto&& interval: sections) {
        Stroke* s = new Stroke();
        s->setColor(stroke->getColor());
        s->setToolType(stroke->getToolType());
        s->setLineStyle(stroke->getLineStyle());
        s->setWidth(stroke->getWidth());
        s->path = stroke->path->cloneSection(interval.min, interval.max);
        strokes.push_back(s);
    }

    return strokes;
}
