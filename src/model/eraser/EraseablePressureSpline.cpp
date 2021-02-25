#include "EraseablePressureSpline.h"

#include <cmath>

#include "model/Stroke.h"

#include "LoopUtil.h"
#include "UnionOfIntervals.h"

#ifdef EXTRA_CAREFUL
#include <iostream>
#include <sstream>
#endif

EraseablePressureSpline::EraseablePressureSpline(Stroke* stroke): EraseableStroke(stroke) {
    Spline spline = stroke->getSpline();
    this->pointCache.reserve(spline.nbSegments());
    for (auto&& seg: spline.segments()) {
        std::vector<ParametrizedPoint> pts;
        seg.toParametrizedPoints(pts);
        this->pointCache.push_back(std::move(pts));
    }
}

////////////////////////////////////////////////////////////////////////////////
// This is done in a Thread, every thing else in the main loop /////////////////
////////////////////////////////////////////////////////////////////////////////

void EraseablePressureSpline::draw(cairo_t* cr) {  // no pressure
    std::vector<Interval<Path::Parameter>> sections;
    {  // lock_guard scope
        std::lock_guard<std::mutex> lock(sectionsMutex);
        sections = remainingSections.cloneToIntervalVector();
    }  // Release the mutex

    for (auto&& interval: sections) {
        Point p = stroke->path->getPoint(interval.min);
        cairo_set_line_width(cr, p.z);
        cairo_move_to(cr, p.x, p.y);

        auto pointCacheIt = pointCache.cbegin() + static_cast<std::ptrdiff_t>(interval.min.index);

        // Iterator to the first point whose parameter is greater that interval.min.t
        auto ptIt = std::upper_bound(pointCacheIt->cbegin(), pointCacheIt->cend(), interval.min.t,
                                     [](const double& t, const ParametrizedPoint& pt) { return t < pt.t; });

        auto endPointCacheIt = pointCache.cbegin() + static_cast<std::ptrdiff_t>(interval.max.index);
        if (pointCacheIt != endPointCacheIt) {
            // The drawn interval spans more than one spline segment
            for (auto endPtIt = pointCacheIt->cend(); ptIt < endPtIt; ++ptIt) {
                // First (portion of a) segment
                cairo_line_to(cr, ptIt->x, ptIt->y);
                cairo_stroke(cr);
                cairo_set_line_width(cr, ptIt->z);
                cairo_move_to(cr, ptIt->x, ptIt->y);
            }
            ++pointCacheIt;
            for (; pointCacheIt != endPointCacheIt; ++pointCacheIt) {
                // Middle segments
                for (auto&& p: *pointCacheIt) {
                    cairo_line_to(cr, p.x, p.y);
                    cairo_stroke(cr);
                    cairo_set_line_width(cr, p.z);
                    cairo_move_to(cr, p.x, p.y);
                }
            }
            ptIt = pointCacheIt->cbegin();
        }
        // Iterator to the first point whose parameter it greater or equal to interval.max.t
        auto endPtIt = std::lower_bound(ptIt, pointCacheIt->cend(), interval.max.t,
                                        [](const ParametrizedPoint& pt, const double& t) { return pt.t < t; });

        for (; ptIt < endPtIt; ++ptIt) {
            // Last (portion of a) segment
            cairo_line_to(cr, ptIt->x, ptIt->y);
            cairo_stroke(cr);
            cairo_set_line_width(cr, ptIt->z);
            cairo_move_to(cr, ptIt->x, ptIt->y);
        }

        Point q = stroke->path->getPoint(interval.max);
        cairo_line_to(cr, q.x, q.y);
        cairo_stroke(cr);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
