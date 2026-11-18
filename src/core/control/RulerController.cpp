#include "RulerController.h"

#include <algorithm>
#include <cmath>

#include "model/Ruler.h"
#include "model/Stroke.h"

namespace {
constexpr double SNAP_EDGE_Y = 0.;
constexpr double SNAP_EPSILON = 1e-6;
}

RulerController::RulerController(XojPageView* view, Ruler* ruler): GeometryToolController(view, ruler) {}
RulerController::~RulerController() = default;

auto RulerController::getType() const -> GeometryToolType { return GeometryToolType::TRAD_RULER; }

auto RulerController::posRelToRuler(double x, double y) const -> xoj::util::Point<double> {
    auto matrix = geometryTool->getMatrix();
    cairo_matrix_invert(&matrix);
    cairo_matrix_transform_point(&matrix, &x, &y);
    return {x, y};
}

auto RulerController::isInsideGeometryTool(double x, double y, double border) const -> bool {
    const auto p = posRelToRuler(x, y);
    return p.x >= -geometryTool->getHeight() / 2. - border && p.x <= geometryTool->getHeight() / 2. + border &&
           p.y >= -border && p.y <= Ruler::THICKNESS + border;
}

auto RulerController::isNearRulerEdge(double x, double y, double tolerancePixels, double zoom) const -> bool {
    const auto p = posRelToRuler(x, y);
    const double halfLength = geometryTool->getHeight() / 2. + SNAP_EPSILON;
    if (p.x < -halfLength || p.x > halfLength) {
        return false;
    }
    const double edgeDistancePixels = std::abs(p.y) * CM * zoom;
    return edgeDistancePixels <= tolerancePixels + SNAP_EPSILON;
}

void RulerController::createStroke(double x) {
    x = std::clamp(x, -geometryTool->getHeight() / 2., geometryTool->getHeight() / 2.);
    strokeMin = strokeMax = x;
    initializeStroke();
    auto matrix = geometryTool->getMatrix();
    double sx = x;
    double sy = SNAP_EDGE_Y;
    cairo_matrix_transform_point(&matrix, &sx, &sy);
    stroke->addPoint(Point(sx, sy));
    stroke->addPoint(Point(sx, sy));
    geometryTool->notify();
}

void RulerController::updateStroke(double x) {
    x = std::clamp(x, -geometryTool->getHeight() / 2., geometryTool->getHeight() / 2.);
    strokeMin = std::min(strokeMin, x);
    strokeMax = std::max(strokeMax, x);
    stroke->deletePointsFrom(0);
    auto matrix = geometryTool->getMatrix();
    double x1 = strokeMin;
    double x2 = strokeMax;
    double y1 = SNAP_EDGE_Y;
    double y2 = SNAP_EDGE_Y;
    cairo_matrix_transform_point(&matrix, &x1, &y1);
    cairo_matrix_transform_point(&matrix, &x2, &y2);
    stroke->addPoint(Point(x1, y1));
    stroke->addPoint(Point(x2, y2));
    geometryTool->notify();
}

void RulerController::finalizeStroke() {
    strokeMin = std::numeric_limits<double>::max();
    strokeMax = std::numeric_limits<double>::lowest();
    addStrokeToLayer();
}

auto RulerController::existsStroke() const -> bool {
    return strokeMin != std::numeric_limits<double>::max();
}
