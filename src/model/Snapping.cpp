#include "Snapping.h"

#include <cmath>

namespace Snapping {

[[nodiscard]] inline double roundToMultiple(double val, double multiple) { return val - std::remainder(val, multiple); }
[[nodiscard]] inline double distance(Point const& a, Point const& b) { return std::hypot(b.x - a.x, b.y - a.y); }
double snapVertically(double y, double gridSize, double tolerance) {
    double ySnapped = roundToMultiple(y, gridSize);
    return abs(ySnapped - y) < tolerance * gridSize ? ySnapped : y;
}

double snapHorizontally(double x, double gridSize, double tolerance) {
    double xSnapped = roundToMultiple(x, gridSize);
    return abs(xSnapped - x) < tolerance * gridSize ? xSnapped : x;
}

Point snapToGrid(Point const& pos, double gridSize, double tolerance) {
    double abs_tolerance = (gridSize / sqrt(2)) * tolerance;
    Point ret{roundToMultiple(pos.x, gridSize), roundToMultiple(pos.y, gridSize), pos.z};
    return distance(ret, pos) < abs_tolerance ? ret : pos;
}

double snapAngle(double radian, double tolerance) {
    auto snapped = roundToMultiple(radian, M_PI_4 / 3.0);
    double abs_tolerance = (M_PI_4 / 6.0) * tolerance;
    return std::abs(snapped - radian) < abs_tolerance ? snapped : radian;
}

Point snapRotation(Point const& pos, Point const& center, double tolerance) {
    auto const dist = distance(pos, center);
    auto const angle = std::atan2(pos.y - center.y, pos.x - center.x);
    auto const snappedAngle = snapAngle(angle, tolerance);
    return {center.x + dist * std::cos(snappedAngle),  //
            center.y + dist * std::sin(snappedAngle), pos.z};
}

}  // namespace Snapping
