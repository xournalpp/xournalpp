/*
 * Xournal++
 *
 * Part of the Xournal shape recognizer
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include "model/Point.h"

class Inertia;

struct RecoSegment final {
    Point calcEdgeIsect(RecoSegment* r2) const;

    /**
     * Find the geometry of a recognized segment
     */
    void calcSegmentGeometry(const Point* pt, int start, int end, Inertia* s);

    int startpt{0};
    int endpt{0};

    double xcenter{0};
    double ycenter{0};
    double angle{0};
    double radius{0};

    double x1{0};
    double y1{0};
    double x2{0};
    double y2{0};

    bool reversed{};
};
