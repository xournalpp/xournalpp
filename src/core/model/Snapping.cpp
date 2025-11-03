#include "Snapping.h"

#include <algorithm>  // for min, max
#include <array>      // for array
#include <cmath>      // for pow, atan2, cos, hypot, remainder, sin, abs
#include <cstdlib>    // for abs

#include "model/Point.h"  // for Point

namespace Snapping {

[[nodiscard]] inline double roundToMultiple(double val, double multiple) { return val - std::remainder(val, multiple); }
[[nodiscard]] inline double distance(Point const& a, Point const& b) { return std::hypot(b.x - a.x, b.y - a.y); }

double snapVertically(double y, double gridSize, double tolerance) {
    return snapVertically(y, gridSize, tolerance, 0.0);
}

double snapVertically(double y, double gridSize, double tolerance, double yOffset) {
    double ySnapped = roundToMultiple(y - yOffset, gridSize) + yOffset;
    return std::abs(ySnapped - y) < tolerance * gridSize / 2.0 ? ySnapped : y;
}

double snapHorizontally(double x, double gridSize, double tolerance) {
    return snapHorizontally(x, gridSize, tolerance, 0.0);
}

double snapHorizontally(double x, double gridSize, double tolerance, double xOffset) {
    double xSnapped = roundToMultiple(x - xOffset, gridSize) + xOffset;
    return std::abs(xSnapped - x) < tolerance * gridSize / 2.0 ? xSnapped : x;
}

Point snapToGrid(Point const& pos, double gridSize, double tolerance) {
    return snapToGrid(pos, gridSize, tolerance, 0.0, 0.0);
}

Point snapToGrid(Point const& pos, double gridSize, double tolerance, double xOffset, double yOffset) {
    double abs_tolerance = (gridSize / sqrt(2)) * tolerance;
    Point ret{roundToMultiple(pos.x - xOffset, gridSize) + xOffset,
              roundToMultiple(pos.y - yOffset, gridSize) + yOffset, pos.z};
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

Point snapToIsometricGrid(Point const& pos, double triangleSize, double tolerance, double xOffset, double yOffset) {
    // Snap to triangular lattice grid. Find nearest vertex from ~7 candidates around the point.
    const double columnSpacing = std::sqrt(3.0) / 2.0 * triangleSize;
    const double rowSpacing = triangleSize / 2.0;

    // Transform to grid-relative coordinates
    const double relativeX = pos.x - xOffset;
    const double relativeY = pos.y - yOffset;

    // Find approximate grid cell containing the point
    const int approxCol = static_cast<int>(std::round(relativeX / columnSpacing));
    const int approxRow = static_cast<int>(std::round(relativeY / rowSpacing));

    // Build list of candidate vertices to check
    // Maximum of 7 candidates: center vertex + up to 6 neighbors
    constexpr size_t MAX_CANDIDATES = 7;
    std::array<Point, MAX_CANDIDATES> candidates;
    size_t candidateCount = 0;

    // Lambda helper to add candidate vertices
    auto addCandidate = [&](int col, int row) {
        if (candidateCount < MAX_CANDIDATES) {
            candidates[candidateCount++] = Point(xOffset + col * columnSpacing, yOffset + row * rowSpacing, pos.z);
        }
    };

    // Add center vertex
    addCandidate(approxCol, approxRow);

    // Add adjacent vertices in the same column (even columns only)
    const bool isEvenColumn = (approxCol % 2 == 0);
    if (isEvenColumn) {
        addCandidate(approxCol, approxRow - 2);
        addCandidate(approxCol, approxRow + 2);
    }

    // Add adjacent vertices in neighboring columns (forms the triangular connections)
    addCandidate(approxCol - 1, approxRow - 1);
    addCandidate(approxCol - 1, approxRow + 1);
    addCandidate(approxCol + 1, approxRow - 1);
    addCandidate(approxCol + 1, approxRow + 1);

    // Find the nearest vertex from all candidates
    Point nearestVertex = candidates[0];
    double minDistance = distance(pos, nearestVertex);

    for (size_t i = 1; i < candidateCount; ++i) {
        const double dist = distance(pos, candidates[i]);
        if (dist < minDistance) {
            minDistance = dist;
            nearestVertex = candidates[i];
        }
    }

    // Only snap if within tolerance threshold
    const double snapThreshold = triangleSize * tolerance;
    return (minDistance < snapThreshold) ? nearestVertex : pos;
}

}  // namespace Snapping
