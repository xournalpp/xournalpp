#include "RulerController.h"

#include <algorithm>
#include <cmath>

#include "model/Ruler.h"
#include "model/Stroke.h"

RulerController::RulerController(XojPageView* view, Ruler* ruler): GeometryToolController(view, ruler) {}
RulerController::~RulerController() = default;

auto RulerController::getType() const -> GeometryToolType { return GeometryToolType::TRAD_RULER; }

auto RulerController::posRelToRuler(double x, double y) const -> xoj::util::Point<double> {
    auto matrix = geometryTool->getMatrix();
    cairo_matrix_invert(&matrix);
    cairo_matrix_transform_point(&matrix, &x, &y);
    return {x / CM, y / CM};
}

auto RulerController::isInsideGeometryTool(double x, double y, double border) const -> bool {
    const auto p = posRelToRuler(x, y);
    return p.x >= -geometryTool->getHeight() / 2. - border && p.x <= geometryTool->getHeight() / 2. + border &&
           p.y >= -border && p.y <= Ruler::THICKNESS + border;
}

void RulerController::createStroke(double x) {
    strokeMin = strokeMax = x;
    initializeStroke();
    const auto matrix = geometryTool->getMatrix();
    double sx = x * CM;
    double sy = 0.;
    cairo_matrix_t m = matrix;
    cairo_matrix_transform_point(&m, &sx, &sy);
    stroke->addPoint(Point(sx, sy));
    stroke->addPoint(Point(sx, sy));
    geometryTool->notify();
}

void RulerController::updateStroke(double x) {
    strokeMin = std::min(strokeMin, x);
    strokeMax = std::max(strokeMax, x);
    stroke->deletePointsFrom(0);
    auto matrix = geometryTool->getMatrix();
    double x1 = strokeMin * CM;
    double x2 = strokeMax * CM;
    double y1 = 0.;
    double y2 = 0.;
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
