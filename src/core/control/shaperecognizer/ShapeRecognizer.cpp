#include "ShapeRecognizer.h"

#include <cmath>     // for fabs, M_PI
#include <iterator>  // for begin, next
#include <memory>    // for allocator...
#include <utility>   // for move
#include <vector>    // for vector

#include <glib.h>  // for g_message

#include "control/shaperecognizer/RecoSegment.h"            // for RecoSegment
#include "control/shaperecognizer/ShapeRecognizerConfig.h"  // for RDEBUG
#include "model/Point.h"                                    // for Point
#include "model/Stroke.h"                                   // for Stroke

#include "CircleRecognizer.h"  // for CircleRec...
#include "Inertia.h"           // for Inertia
#include "config-debug.h"      // for DEBUG_REC...

ShapeRecognizer::ShapeRecognizer() {
    resetRecognizer();
    this->stroke = nullptr;
    this->queueLength = 0;
}

ShapeRecognizer::~ShapeRecognizer() { resetRecognizer(); }

void ShapeRecognizer::resetRecognizer() {
    RDEBUG("reset");
    this->queue = {};
    this->queueLength = 0;
}

/**
 *  Test if segments form standard shapes
 */
auto ShapeRecognizer::tryRectangle() -> Stroke* {
    // first, we need whole strokes to combine to 4 segments...
    if (this->queueLength < 4) {
        return nullptr;
    }

    RecoSegment* rs = &this->queue[this->queueLength - 4];
    if (rs->startpt != 0) {
        return nullptr;
    }

    // check edges make angles ~= Pi/2 and vertices roughly match
    double avgAngle = 0.;
    for (int i = 0; i <= 3; i++) {
        RecoSegment* r1 = &rs[i];
        RecoSegment* r2 = &rs[(i + 1) % 4];
        if (fabs(fabs(r1->angle - r2->angle) - M_PI / 2) > RECTANGLE_ANGLE_TOLERANCE) {
            return nullptr;
        }
        avgAngle += r1->angle;
        if (r2->angle > r1->angle) {
            avgAngle += (i + 1) * M_PI / 2;
        } else {
            avgAngle -= (i + 1) * M_PI / 2;
        }

        // test if r1 points away from r2 rather than towards it
        r1->reversed =
                ((r1->x2 - r1->x1) * (r2->xcenter - r1->xcenter) + (r1->y2 - r1->y1) * (r2->ycenter - r1->ycenter)) < 0;
    }
    for (int i = 0; i <= 3; i++) {
        RecoSegment* r1 = &rs[i];
        RecoSegment* r2 = &rs[(i + 1) % 4];
        double dist = hypot((r1->reversed ? r1->x1 : r1->x2) - (r2->reversed ? r2->x2 : r2->x1),
                            (r1->reversed ? r1->y1 : r1->y2) - (r2->reversed ? r2->y2 : r2->y1));
        if (dist > RECTANGLE_LINEAR_TOLERANCE * (r1->radius + r2->radius)) {
            return nullptr;
        }
    }

    // make a rectangle of the correct size and slope
    avgAngle = avgAngle / 4;
    if (fabs(avgAngle) < SLANT_TOLERANCE) {
        avgAngle = 0.;
    }

    if (fabs(avgAngle) > M_PI / 2 - SLANT_TOLERANCE) {
        avgAngle = M_PI / 2;
    }

    auto* s = new Stroke();
    s->applyStyleFrom(this->stroke);

    for (int i = 0; i <= 3; i++) {
        rs[i].angle = avgAngle + i * M_PI / 2;
    }

    for (int i = 0; i <= 3; i++) {
        Point p = rs[i].calcEdgeIsect(&rs[(i + 1) % 4]);
        s->addPoint(p);
    }

    s->addPoint(s->getPoint(0));

    return s;
}

/*
 * check if something is a polygonal line with at most nsides sides
 */
auto ShapeRecognizer::findPolygonal(const Point* pt, int start, int end, int nsides, int* breaks, Inertia* ss) -> int {
    Inertia s;
    int i1 = 0, i2 = 0, n1 = 0, n2 = 0;

    if (end == start) {
        return 0;  // no way
    }

    if (nsides <= 0) {
        return 0;
    }

    if (end - start < 5) {
        nsides = 1;  // too small for a polygon
    }

    // look for a linear piece that's big enough
    int k = 0;
    for (; k < nsides; k++) {
        i1 = start + (k * (end - start)) / nsides;
        i2 = start + ((k + 1) * (end - start)) / nsides;
        s.calc(pt, i1, i2);
        if (s.det() < LINE_MAX_DET) {
            break;
        }
    }
    if (k == nsides) {
        return 0;  // failed!
    }

    double det1{};
    double det2{};
    Inertia s1;
    Inertia s2;

    // grow the linear piece we found
    while (true) {
        if (i1 > start) {
            s1 = s;
            s1.increase(pt[i1 - 1], pt[i1], 1);
            det1 = s1.det();
        } else {
            det1 = 1.0;
        }

        if (i2 < end) {
            s2 = s;
            s2.increase(pt[i2], pt[i2 + 1], 1);
            det2 = s2.det();
        } else {
            det2 = 1.0;
        }

        if (det1 < det2 && det1 < LINE_MAX_DET) {
            i1--;
            s = s1;
        } else if (det2 < det1 && det2 < LINE_MAX_DET) {
            i2++;
            s = s2;
        } else {
            break;
        }
    }

    if (i1 > start) {
        n1 = findPolygonal(pt, start, i1, (i2 == end) ? (nsides - 1) : (nsides - 2), breaks, ss);
        if (n1 == 0) {
            return 0;  // it doesn't work
        }
    } else {
        n1 = 0;
    }

    breaks[n1] = i1;
    breaks[n1 + 1] = i2;
    ss[n1] = s;

    if (i2 < end) {
        n2 = findPolygonal(pt, i2, end, nsides - n1 - 1, breaks + n1 + 1, ss + n1 + 1);
        if (n2 == 0) {
            return 0;
        }
    } else {
        n2 = 0;
    }

    return n1 + n2 + 1;
}

/**
 * Improve on the polygon found by find_polygonal()
 */
void ShapeRecognizer::optimizePolygonal(const Point* pt, int nsides, int* breaks, Inertia* ss) {
    for (int i = 1; i < nsides; i++) {
        // optimize break between sides i and i+1
        double cost = ss[i - 1].det() * ss[i - 1].det() + ss[i].det() * ss[i].det();
        Inertia s1 = ss[i - 1];
        Inertia s2 = ss[i];
        bool improved = false;
        while (breaks[i] > breaks[i - 1] + 1) {
            // try moving the break to the left
            s1.increase(pt[breaks[i] - 1], pt[breaks[i] - 2], -1);
            s2.increase(pt[breaks[i] - 1], pt[breaks[i] - 2], 1);
            double newcost = s1.det() * s1.det() + s2.det() * s2.det();

            if (newcost >= cost) {
                break;
            }

            improved = true;
            cost = newcost;
            breaks[i]--;
            ss[i - 1] = s1;
            ss[i] = s2;
        }

        if (improved) {
            continue;
        }

        s1 = ss[i - 1];
        s2 = ss[i];
        while (breaks[i] < breaks[i + 1] - 1) {
            // try moving the break to the right
            s1.increase(pt[breaks[i]], pt[breaks[i] + 1], 1);
            s2.increase(pt[breaks[i]], pt[breaks[i] + 1], -1);

            double newcost = s1.det() * s1.det() + s2.det() * s2.det();
            if (newcost >= cost) {
                break;
            }

            cost = newcost;
            breaks[i]++;
            ss[i - 1] = s1;
            ss[i] = s2;
        }
    }
}

/**
 * Determine if a particular stroke is large enough as to make a shape out of it
 */
auto ShapeRecognizer::isStrokeLargeEnough(Stroke* stroke, double strokeMinSize) -> bool {
    if (stroke->getPointCount() < 3) {
        return false;
    }

    auto rect = stroke->getSnappedBounds();
    return std::hypot(rect.width, rect.height) >= strokeMinSize;
}

/**
 * The main pattern recognition function
 */
auto ShapeRecognizer::recognizePatterns(Stroke* stroke, double strokeMinSize) -> Stroke* {
    this->stroke = stroke;

    if (!isStrokeLargeEnough(stroke, strokeMinSize)) {
        return nullptr;
    }

    Inertia ss[4];
    int brk[5] = {0};

    // first see if it's a polygon
    int n = findPolygonal(stroke->getPoints(), 0, stroke->getPointCount() - 1, MAX_POLYGON_SIDES, brk, ss);
    if (n > 0) {
        optimizePolygonal(stroke->getPoints(), n, brk, ss);
#ifdef DEBUG_RECOGNIZER
        g_message("--");
        g_message("ShapeReco:: Polygon, %d edges:", n);
        for (int i = 0; i < n; i++) {
            g_message("ShapeReco::      %d-%d (M=%.0f, det=%.4f)", brk[i], brk[i + 1], ss[i].getMass(), ss[i].det());
        }
        g_message("--");
#endif
        // update recognizer segment queue (most recent at end)
        while (n + queueLength > MAX_POLYGON_SIDES) {
            // remove oldest polygonal stroke
            int i = 1;
            while (i < queueLength && queue[i].startpt != 0) {
                i++;
            }
            queueLength -= i;
            std::move(std::next(begin(queue), i), std::next(begin(queue), i + queueLength), begin(queue));
        }

        RDEBUG("Queue now has %i + %i edges", this->queueLength, n);

        RecoSegment* rs = &this->queue[this->queueLength];
        this->queueLength += n;

        for (int i = 0; i < n; i++) {
            rs[i].startpt = brk[i];
            rs[i].endpt = brk[i + 1];
            rs[i].calcSegmentGeometry(stroke->getPoints(), brk[i], brk[i + 1], ss + i);
        }

        if (Stroke* result = tryRectangle(); result != nullptr) {
            RDEBUG("return rectangle");
            return result;
        }

        // Removed complicated recognition in commit 5494bd002050182cde3af70bd1924f4062579be5

        if (n == 1)  // current stroke is a line
        {
            bool aligned = true;
            if (fabs(rs->angle) < SLANT_TOLERANCE)  // nearly horizontal
            {
                rs->angle = 0.0;
                rs->y1 = rs->y2 = rs->ycenter;
            } else if (fabs(rs->angle) > M_PI / 2 - SLANT_TOLERANCE) {  // nearly vertical
                rs->angle = (rs->angle > 0) ? (M_PI / 2) : (-M_PI / 2);
                rs->x1 = rs->x2 = rs->xcenter;
            } else {
                aligned = false;
            }

            auto* s = new Stroke();
            s->applyStyleFrom(this->stroke);

            if (aligned) {
                s->addPoint(Point(rs->x1, rs->y1));
                s->addPoint(Point(rs->x2, rs->y2));
            } else {
                auto points = stroke->getPointVector();
                s->addPoint(Point(points.front().x, points.front().y));
                s->addPoint(Point(points.back().x, points.back().y));
            }

            RDEBUG("return line");
            return s;
        }
    }

    // not a polygon: maybe a circle ?
    Stroke* s = CircleRecognizer::recognize(stroke);
    if (s) {
        RDEBUG("return circle");
        return s;
    }

    return nullptr;
}
