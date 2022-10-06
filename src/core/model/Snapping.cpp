#include "Snapping.h"

#include <algorithm>  // for min, max
#include <cmath>      // for pow, atan2, cos, hypot, remainder, sin, abs
#include <cstdlib>    // for abs

#include "model/Point.h"  // for Point

namespace Snapping {

[[nodiscard]] inline double roundToMultiple(double val, double multiple) { return val - std::remainder(val, multiple); }
[[nodiscard]] inline double distance(Point const& a, Point const& b) { return std::hypot(b.x - a.x, b.y - a.y); }
double snapVertically(double y, double gridSize, double tolerance) {
    double ySnapped = roundToMultiple(y, gridSize);
    return std::abs(ySnapped - y) < tolerance * gridSize / 2.0 ? ySnapped : y;
}

double snapHorizontally(double x, double gridSize, double tolerance) {
    double xSnapped = roundToMultiple(x, gridSize);
    return std::abs(xSnapped - x) < tolerance * gridSize / 2.0 ? xSnapped : x;
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

Point projToLine(Point const& pos, Point const& first, Point const& second) {
    const double scalar = ((first.x - pos.x) * (second.y - first.y) + (first.y - pos.y) * (first.x - second.x)) /
                          (std::pow(second.y - first.y, 2) + std::pow(first.x - second.x, 2));
    const double projX = pos.x + scalar * (second.y - first.y);
    const double projY = pos.y + scalar * (first.x - second.x);
    return Point(projX, projY, pos.z);
}

double distanceLine(Point const& pos, Point const& first, Point const& second) {
    const auto proj = projToLine(pos, first, second);
    if (std::min(first.x, second.x) <= proj.x && proj.x <= std::max(first.x, second.x) &&
        std::min(first.y, second.y) <= proj.y && proj.y <= std::max(first.y, second.y)) {
        return distance(pos, proj);
    } else {
        const double dist1 = distance(pos, first);
        const double dist2 = distance(pos, second);
        return std::min(dist1, dist2);
    }
}

Point snapToLine(Point const& pos, Point const& first, Point const& second, double tolerance) {
    if (first.x == second.x && first.y == second.y) {
        return distance(pos, first) > tolerance ? pos : first;
    }
    const double dist = distanceLine(pos, first, second);
    const double dist1 = distance(pos, first);
    const double dist2 = distance(pos, second);
    const double distance = std::min({dist, dist1, dist2});
    if (distance > tolerance) {
        return pos;
    } else if (distance == dist1) {
        return first;
    } else if (distance == dist2) {
        return second;
    } else {
        return projToLine(pos, first, second);
    }
}
}  // namespace Snapping
