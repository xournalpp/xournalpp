#include "CompassController.h"

#include "control/Control.h"
#include "control/layer/LayerController.h"
#include "gui/XournalView.h"
#include "model/Compass.h"
#include "model/GeometryTool.h"
#include "model/Stroke.h"
#include "model/XojPage.h"

CompassController::CompassController(XojPageView* view, Compass* compass): GeometryToolController(view, compass) {}

CompassController::~CompassController() = default;

auto CompassController::getType() const -> GeometryToolType { return GeometryToolType::COMPASS; }

auto CompassController::posRelToSide(double x, double y) const -> utl::Point<double> {
    cairo_matrix_t inv = geometryTool->getMatrix();
    cairo_matrix_invert(&inv);
    cairo_matrix_transform_point(&inv, &x, &y);
    return utl::Point<double>(x, -y);
}

auto CompassController::isInsideGeometryTool(double x, double y, double border) const -> bool {
    const utl::Point<double> p = posRelToSide(x, y);
    return std::hypot(p.x, p.y) <= geometryTool->getHeight() + border;
}

auto CompassController::getPointForAngle(double a) const -> utl::Point<double> {
    cairo_matrix_t matrix = geometryTool->getMatrix();
    double x = geometryTool->getHeight() * std::cos(a);
    double y = geometryTool->getHeight() * std::sin(a);
    cairo_matrix_transform_point(&matrix, &x, &y);

    return utl::Point<double>(x, y);
}

auto CompassController::getPointForRadius(double r) const -> utl::Point<double> {
    cairo_matrix_t matrix = geometryTool->getMatrix();
    double x = r;
    double y = 0.;
    cairo_matrix_transform_point(&matrix, &x, &y);

    return utl::Point<double>(x, y);
}

void CompassController::createOutlineStroke(double a) {
    if (!std::isnan(a)) {
        angleMax = a;
        angleMin = a;

        const utl::Point<double> p = this->getPointForAngle(a);
        initializeStroke();
        stroke->addPoint(Point(p.x, p.y));
        stroke->addPoint(Point(p.x, p.y));  // doubled point
        geometryTool->notify();
    } else {
        g_warning("No valid stroke from compass!");
    }
}

void CompassController::createRadialStroke(double x) {
    if (!std::isnan(x)) {
        radiusMax = x;
        radiusMin = x;

        const utl::Point<double> p = posRelToSide(x, 0.);
        initializeStroke();
        stroke->addPoint(Point(p.x, p.y));
        stroke->addPoint(Point(p.x, p.y));  // doubled point
        geometryTool->notify();
    } else {
        g_warning("No valid radius from compass!");
    }
}

void CompassController::updateOutlineStroke(double x) {
    angleMax = std::max(this->angleMax, x);
    angleMin = std::min(this->angleMin, x);
    stroke->deletePointsFrom(0);
    const auto h = view->getXournal()->getControl()->getToolHandler();
    const bool filled = (h->getFill() != -1);
    const utl::Point<double> c = utl::Point<double>{geometryTool->getTranslationX(), geometryTool->getTranslationY()};

    if (filled && angleMax < angleMin + 2 * M_PI) {
        stroke->addPoint(Point(c.x, c.y));
    }
    for (auto i = 0; i <= 100; i++) {
        const utl::Point<double> p =
                getPointForAngle(angleMin + static_cast<double>(i) / 100.0 * (angleMax - angleMin));
        stroke->addPoint(Point(p.x, p.y));
    }
    if (filled && angleMax < angleMin + 2 * M_PI) {
        stroke->addPoint(Point(c.x, c.y));
    }
    geometryTool->notify();
}

void CompassController::updateRadialStroke(double x) {
    radiusMax = std::max(this->radiusMax, x);
    radiusMin = std::min(this->radiusMin, x);
    stroke->deletePointsFrom(0);
    const utl::Point<double> p1 = getPointForRadius(radiusMin);
    const utl::Point<double> p2 = getPointForRadius(radiusMax);

    stroke->addPoint(Point(p1.x, p1.y));
    stroke->addPoint(Point(p2.x, p2.y));
    geometryTool->notify();
}

void CompassController::finalizeOutlineStroke() {
    angleMax = std::numeric_limits<double>::lowest();
    angleMin = std::numeric_limits<double>::max();
    addStrokeToLayer();
}

void CompassController::finalizeRadialStroke() {
    radiusMax = std::numeric_limits<double>::lowest();
    radiusMin = std::numeric_limits<double>::max();
    addStrokeToLayer();
}

auto CompassController::existsOutlineStroke() -> bool { return angleMax != std::numeric_limits<double>::lowest(); }

auto CompassController::existsRadialStroke() -> bool { return radiusMax != std::numeric_limits<double>::lowest(); }
