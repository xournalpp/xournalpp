#include "CircleRecognizer.h"

#include <cmath>

#include "model/PiecewiseLinearPath.h"
#include "model/Spline.h"
#include "util/LoopUtil.h"

#include "Inertia.h"
#include "ShapeRecognizerConfig.h"

/**
 *  Test if we have a circle; inertia has been precomputed by caller
 */
auto CircleRecognizer::scoreCircle(const PiecewiseLinearPath& path, Inertia& inertia) -> double {
    double r0 = inertia.rad();
    double divisor = inertia.getMass() * r0;

    // Todo: test: std::abs(divisor) <= std::numeric_limits<double>::epsilon()
    if (divisor == 0) {
        return 0;
    }

    double sum = 0.0;
    double x0 = inertia.centerX();
    double y0 = inertia.centerY();

    auto const& pv = path.getData();
    for (auto pt_1st = begin(pv), pt_2nd = std::next(pt_1st), p_end_i = end(pv); pt_1st != p_end_i && pt_2nd != p_end_i;
         ++pt_2nd, ++pt_1st) {
        double dm = hypot(pt_2nd->x - pt_1st->x, pt_2nd->y - pt_1st->y);
        double deltar = hypot(pt_1st->x - x0, pt_1st->y - y0) - r0;
        sum += dm * fabs(deltar);
    }

    return sum / (divisor);
}

auto CircleRecognizer::recognize(const PiecewiseLinearPath& path) -> std::shared_ptr<Spline> {
    Inertia s;
    s.calc(path.getData().data(), 0, path.getData().size());
    RDEBUG("Mass=%.0f, Center=(%.1f,%.1f), I=(%.0f,%.0f, %.0f), Rad=%.2f, Det=%.4f", s.getMass(), s.centerX(),
           s.centerY(), s.xx(), s.yy(), s.xy(), s.rad(), s.det());

    if (s.det() > CIRCLE_MIN_DET) {
        double score = CircleRecognizer::scoreCircle(path, s);
        RDEBUG("Circle score: %.2f", score);
        if (score < CIRCLE_MAX_SCORE) {
            std::shared_ptr<Spline> circle = std::make_shared<Spline>();
            circle->makeEllipse(Point(s.centerX(), s.centerY()), s.rad(), s.rad());
            return circle;
        }
    }

    return nullptr;
}
