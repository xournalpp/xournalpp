#include "ShapeRecognizer.h"

#include <cmath>     // for fabs, M_PI
#include <iterator>  // for begin, next
#include <memory>    // for allocator...
#include <span>      // for span
#include <utility>   // for move
#include <vector>    // for vector
#include <numeric>   

#include <glib.h>  // for g_message

#include "control/shaperecognizer/RecoSegment.h"            // for RecoSegment
#include "control/shaperecognizer/ShapeRecognizerConfig.h"  // for RDEBUG
#include "model/Point.h"                                    // for Point
#include "model/Stroke.h"                                   // for Stroke
#include "util/safe_casts.h"                                // for as_unsigned

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

inline double dist2(const Point& P, const Point& Q) {
    const double dx = P.x - Q.x;
    const double dy = P.y - Q.y;
    return dx * dx + dy * dy;
}

void ShapeRecognizer::reduceVerticesToFour(std::vector<Corner>& cornerList) {

    if (cornerList.size() <= 5) {
        return;
    }
    //save the first and last corner as these stay as they are 
    Corner firstCorner = cornerList.front();
    Corner lastCorner = cornerList.back();
    
    //create an array of indices for all interior points
    std::vector<size_t> innerIndices(cornerList.size() - 2);
    std::iota(innerIndices.begin(), innerIndices.end(), 1);
    
    //find the 3 interior corners with the most error 
    std::nth_element(innerIndices.begin(), 
                     innerIndices.begin() + 3, 
                     innerIndices.end(),
                     [&cornerList](size_t a, size_t b) {
                         return cornerList[a].dmax > cornerList[b].dmax;
                     });

    innerIndices.resize(3);
    std::sort(innerIndices.begin(), innerIndices.end());

    std::vector<Corner> newCorners;
    newCorners.reserve(5);
    
    newCorners.push_back(firstCorner);         
    for (size_t idx : innerIndices) {
        newCorners.push_back(cornerList[idx]); 
    }
    newCorners.push_back(lastCorner); 

    cornerList = std::move(newCorners);
    
}

double ShapeRecognizer::calculateDynamicEpsilon(const Point* pt, int last_idx) {
    if (last_idx < 0)
        return 1.0;

    double minX = pt[0].x;
    double maxX = pt[0].x;
    double minY = pt[0].y;
    double maxY = pt[0].y;

    for (int i = 1; i <= last_idx; ++i) {
        minX = std::min(minX, pt[i].x);
        maxX = std::max(maxX, pt[i].x);
        minY = std::min(minY, pt[i].y);
        maxY = std::max(maxY, pt[i].y);
    }

    double strokeWidth = maxX - minX;
    double strokeHeight = maxY - minY;
    double shortestSide = std::min(strokeWidth, strokeHeight);

    return std::max(1.0, shortestSide * 0.05);
}

double ShapeRecognizer::perpendicularDistance(const Point& p, const Point& a, const Point& b) {
    const double dx = b.x - a.x;
    const double dy = b.y - a.y;
    const double magSq = dx * dx + dy * dy;

    if (magSq < 1e-10) {
        // a and b are effectively the same point.
        // Return Euclidean distance: sqrt((p.x-a.x)^2 + (p.y-a.y)^2)
        return std::hypot(p.x - a.x, p.y - a.y);
    }

    const double mag = std::sqrt(magSq);
    return std::abs(dy * p.x - dx * p.y + b.x * a.y - b.y * a.x) / mag;
}

auto ShapeRecognizer::tryTriangle() -> std::unique_ptr<Stroke> {
    // first, we need whole strokes to combine to 3 segments...
    if (this->queueLength < 3) {
        return nullptr;
    }

    RecoSegment* rs = &this->queue[as_unsigned(this->queueLength - 3)];
    if (rs->startpt != 0) {
        return nullptr;
    }

    /*
    Make segments be oriented so that, for every pair of neighbouring segments,
    the first segment points towards the second. This should make the polygon
    have all of its segments oriented either clockwise or counter-clockwise.

    The direction of any segment R is defined from P to Q where
    if R is not reversed then
        P is (x1,y1)
        Q is (x2,y2)
    else
        P is (x2,y2)
        Q is (x1,y1)
    */
    for (int i = 0; i <= 2; i++) {
        RecoSegment& r1 = rs[i];
        const RecoSegment& r2 = rs[(i + 1) % 3];

        const Point P(r1.x1, r1.y1);
        const Point Q(r1.x2, r1.y2);
        const Point R(r2.x1, r2.y1);
        const Point S(r2.x2, r2.y2);
        const double min_PR_PS = std::min(dist2(P, R), dist2(P, S));
        const double min_QR_QS = std::min(dist2(Q, R), dist2(Q, S));
        r1.reversed = min_PR_PS < min_QR_QS;
    }

    for (int i = 0; i <= 2; i++) {
        const RecoSegment& r1 = rs[i];
        const RecoSegment& r2 = rs[(i + 1) % 3];

        const double x1 = r1.reversed ? r1.x1 : r1.x2;
        const double y1 = r1.reversed ? r1.y1 : r1.y2;
        const double x2 = r2.reversed ? r2.x2 : r2.x1;
        const double y2 = r2.reversed ? r2.y2 : r2.y1;

        const double dist = hypot(x1 - x2, y1 - y2);
        if (dist > TRIANGLE_LINEAR_TOLERANCE * (r1.radius + r2.radius)) {
            return nullptr;
        }
    }

    auto s = std::make_unique<Stroke>();
    s->applyStyleFrom(this->stroke);

    for (int i = 0; i <= 2; i++) {
        Point p = rs[i].calcEdgeIsect(&rs[(i + 1) % 3]);
        s->addPoint(p);
    }

    s->addPoint(s->getPoint(0));

    return s;
}

/**
 *  Test if segments form standard shapes
 */
auto ShapeRecognizer::tryRectangle() -> std::unique_ptr<Stroke> {
    // first, we need whole strokes to combine to 4 segments...
    if (this->queueLength < 4) {
        return nullptr;
    }

    RecoSegment* rs = &this->queue[as_unsigned(this->queueLength - 4)];
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

    auto s = std::make_unique<Stroke>();
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

/**
 * Find polygonal corners using the Ramer-Douglas-Peucker algorithm.
 * [start, finish] (inclusive) specifies the range to operate on in the pt array.
 * recursively obtains the farthest point between start and finish and
 * if bigger than epsilon considers it a corner.
 */
void ShapeRecognizer::findPolygonalRDP(const Point* pt, int start, int finish, double epsilon,
                                       std::vector<Corner>& cornerList) {
    if (epsilon <= 0.0) {
        return;
    }

    double dmax = 0;
    int index = start;
    // loop for obtainig the farthest point
    for (int i = start + 1; i < finish; i++) {
        double d = perpendicularDistance(pt[i], pt[start], pt[finish]);

        if (d > dmax) {
            index = i;
            dmax = d;
        }
    }
    if (dmax > epsilon) {

        findPolygonalRDP(pt, start, index, epsilon, cornerList);

        if (std::none_of(cornerList.begin(), cornerList.end(), [index](const Corner& c) { return c.index == index; })) {
            cornerList.push_back({index, dmax});
        }

        findPolygonalRDP(pt, index, finish, epsilon, cornerList);
    }
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
auto ShapeRecognizer::recognizePatterns(Stroke* stroke, double strokeMinSize) -> std::unique_ptr<Stroke> {
    this->stroke = stroke;

    if (!isStrokeLargeEnough(stroke, strokeMinSize)) {
        return nullptr;
    }
    int last_idx = static_cast<int>(stroke->getPointCount()) - 1;
    Inertia ss[4];
    std::vector<Corner> cornerList = {{0, 0.0}};
    int brk[5] = {0};

    const Point* pt = stroke->getPoints();

    double epsilon = calculateDynamicEpsilon(pt, last_idx);

    // first see if it's a polygon
    findPolygonalRDP(stroke->getPoints(), 0, last_idx, epsilon, cornerList);
    cornerList.push_back({last_idx, 0.0});
    std::sort(cornerList.begin(), cornerList.end(), [](const Corner& a, const Corner& b) { return a.index < b.index; });
    cornerList.erase(std::unique(cornerList.begin(), cornerList.end(),
                                 [](const Corner& a, const Corner& b) { return a.index == b.index; }),
                     cornerList.end());
    reduceVerticesToFour(cornerList);
    int n = static_cast<int>(cornerList.size()) - 1;
    // Copy the identified corners from cornerList to brk and calculate inertia for each segment.
    for (int i = 0; i < n && i < 4; i++) {
        brk[i] = cornerList[i].index;
        brk[i + 1] = cornerList[i + 1].index;
        ss[i].calc(std::span<const Point>(pt + brk[i], pt + brk[i + 1] + 1));
    }
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
            while (i < queueLength && queue[as_unsigned(i)].startpt != 0) {
                i++;
            }
            queueLength -= i;
            std::move(std::next(begin(queue), i), std::next(begin(queue), i + queueLength), begin(queue));
        }

        RDEBUG("Queue now has %i + %i edges", this->queueLength, n);

        RecoSegment* rs = &this->queue[as_unsigned(this->queueLength)];
        this->queueLength += n;

        for (int i = 0; i < n; i++) {
            rs[i].startpt = brk[i];
            rs[i].endpt = brk[i + 1];
            rs[i].calcSegmentGeometry(stroke->getPoints(), brk[i], brk[i + 1], ss + i);
        }

        if (auto result = tryTriangle(); result != nullptr) {
            RDEBUG("return triangle");
            return result;
        }
        if (auto result = tryRectangle(); result != nullptr) {
            RDEBUG("return rectangle");
            return result;
        }

        // Removed complicated recognition in commit 5494bd002050182cde3af70bd1924f4062579be5

        if (n == 1 && ss->det() < LINE_MAX_DET)  // current stroke is a line
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

            auto s = std::make_unique<Stroke>();
            s->applyStyleFrom(this->stroke);

            if (aligned) {
                s->addPoint(Point(rs->x1, rs->y1));
                s->addPoint(Point(rs->x2, rs->y2));
            } else {
                const Point P(rs->x1, rs->y1);
                const Point Q(rs->x2, rs->y2);

                const auto& points = stroke->getPointVector();
                const Point& last = points.back();

                const double dx = Q.x - P.x;
                const double dy = Q.y - P.y;
                const double num = dy * last.x - dx * last.y + Q.x * P.y - Q.y * P.x;
                const double num2 = num * num;
                const double den2 = dy * dy + dx * dx;
                const double dist2 = num2 / den2;

                if (dist2 < LINE_POINT_DIST2_THRESHOLD) {
                    s->addPoint(P);
                    s->addPoint(Q);
                } else {
                    s->addPoint(Point(points.front().x, points.front().y));
                    s->addPoint(Point(points.back().x, points.back().y));
                }
            }

            RDEBUG("return line");
            return s;
        }
    }

    // not a polygon: maybe a circle ?
    auto s = CircleRecognizer::recognize(stroke);
    if (s) {
        RDEBUG("return circle");
        return s;
    }

    return nullptr;
}
