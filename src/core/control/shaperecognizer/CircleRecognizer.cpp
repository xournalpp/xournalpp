#include "CircleRecognizer.h"

#include <cmath>     // for hypot, cos, fabs, sin, M_PI
#include <iterator>  // for begin, end, next
#include <vector>    // for vector

#include "model/Point.h"   // for Point
#include "model/Stroke.h"  // for Stroke

#include "Inertia.h"                // for Inertia
#include "ShapeRecognizerConfig.h"  // for RDEBUG, CIRCLE_MAX_SCORE, CIRCLE_...

/**
 * Create circle stroke for inertia
 */
auto CircleRecognizer::makeCircleShape(Stroke* originalStroke, Inertia& inertia) -> Stroke* {
    int npts = static_cast<int>(2 * inertia.rad());
    if (npts < 24) {
        npts = 24;  // min. number of points
    }

    auto* s = new Stroke();
    s->applyStyleFrom(originalStroke);

    for (int i = 0; i <= npts; i++) {
        double x = inertia.centerX() + inertia.rad() * cos((2 * M_PI * i) / npts);
        double y = inertia.centerY() + inertia.rad() * sin((2 * M_PI * i) / npts);
        s->addPoint(Point(x, y));
    }

    return s;
}

/**
 *  Test if we have a circle; inertia has been precomputed by caller
 */
auto CircleRecognizer::scoreCircle(Stroke* s, Inertia& inertia) -> double {
    double r0 = inertia.rad();
    double divisor = inertia.getMass() * r0;

    // Todo: test: std::abs(divisor) <= std::numeric_limits<double>::epsilon()
    if (divisor == 0) {
        return 0;
    }

    double sum = 0.0;
    double x0 = inertia.centerX();
    double y0 = inertia.centerY();

    auto const& pv = s->getPointVector();
    for (auto pt_1st = begin(pv), pt_2nd = std::next(pt_1st), p_end_i = end(pv); pt_1st != p_end_i && pt_2nd != p_end_i;
         ++pt_2nd, ++pt_1st) {
        double dm = hypot(pt_2nd->x - pt_1st->x, pt_2nd->y - pt_1st->y);
        double deltar = hypot(pt_1st->x - x0, pt_1st->y - y0) - r0;
        sum += dm * fabs(deltar);
    }

    return sum / (divisor);
}

auto CircleRecognizer::recognize(Stroke* stroke) -> Stroke* {
    Inertia s;
    s.calc(stroke->getPoints(), 0, stroke->getPointCount());
    RDEBUG("Mass=%.0f, Center=(%.1f,%.1f), I=(%.0f,%.0f, %.0f), Rad=%.2f, Det=%.4f", s.getMass(), s.centerX(),
           s.centerY(), s.xx(), s.yy(), s.xy(), s.rad(), s.det());

    if (s.det() > CIRCLE_MIN_DET) {
        double score = CircleRecognizer::scoreCircle(stroke, s);
        RDEBUG("Circle score: %.2f", score);
        if (score < CIRCLE_MAX_SCORE) {
            return CircleRecognizer::makeCircleShape(stroke, s);
        }
    }

    return nullptr;
}
