#include "SetsquareController.h"

#include "control/Control.h"
#include "control/layer/LayerController.h"
#include "gui/XournalView.h"
#include "model/GeometryTool.h"
#include "model/Setsquare.h"
#include "model/Stroke.h"
#include "model/XojPage.h"
#include "undo/InsertUndoAction.h"  // for InsertUndoAction

using xoj::util::Rectangle;
SetsquareController::SetsquareController(XojPageView* view, Setsquare* s): GeometryToolController(view, s) {}

SetsquareController::~SetsquareController() = default;

auto SetsquareController::getType() const -> std::string { return "setsquare"; }

auto SetsquareController::posRelToSide(Leg leg, double x, double y) const -> utl::Point<double> {
    cairo_matrix_t matrix{};
    s->getMatrix(matrix);
    cairo_matrix_invert(&matrix);
    cairo_matrix_transform_point(&matrix, &x, &y);
    switch (leg) {
        case HYPOTENUSE:
            return utl::Point<double>(x, -y);
        case LEFT_LEG:
            return utl::Point<double>((y + x) / sqrt(2.), (y - x - s->getHeight()) / sqrt(2.));
        case RIGHT_LEG:
            return utl::Point<double>((y - x) / sqrt(2.), (y + x - s->getHeight()) / sqrt(2.));
        default:
            g_error("Invalid enum value: %d", leg);
    }
}

auto SetsquareController::isInsideGeometryTool(double x, double y, double border) const -> bool {
    return posRelToSide(HYPOTENUSE, x, y).y < border && posRelToSide(LEFT_LEG, x, y).y < border &&
           posRelToSide(RIGHT_LEG, x, y).y < border;
}

auto SetsquareController::getPointForPos(double xCoord) const -> utl::Point<double> {
    cairo_matrix_t matrix{};
    double x = xCoord;
    double y = 0.0;
    s->getMatrix(matrix);
    cairo_matrix_transform_point(&matrix, &x, &y);

    return utl::Point<double>(x, y);
}

void SetsquareController::createStroke(double x) {
    if (!std::isnan(x)) {
        hypotenuseMax = x;
        hypotenuseMin = x;

        const auto p = this->getPointForPos(x);
        initializeStroke();
        stroke->addPoint(Point(p.x, p.y));
        stroke->addPoint(Point(p.x, p.y));  // doubled point
        s->notify();
    } else {
        g_warning("No valid stroke from setsquare!");
    }
}

void SetsquareController::createRadius(double x, double y) {
    const auto p = posRelToSide(HYPOTENUSE, x, y);
    this->strokeAngle = std::atan2(p.y, p.x);
    initializeStroke();
    updateRadius(x, y);
}

void SetsquareController::updateStroke(double x) {
    hypotenuseMax = std::max(this->hypotenuseMax, x);
    hypotenuseMin = std::min(this->hypotenuseMin, x);
    stroke->deletePointsFrom(0);
    const auto p1 = getPointForPos(hypotenuseMin);
    const auto p2 = getPointForPos(hypotenuseMax);

    stroke->addPoint(Point(p1.x, p1.y));
    stroke->addPoint(Point(p2.x, p2.y));
    s->notify();
}

void SetsquareController::updateRadius(double x, double y) {
    stroke->deletePointsFrom(0);
    const auto c = getPointForPos(0);
    stroke->addPoint(Point(c.x, c.y));

    const auto p = posRelToSide(HYPOTENUSE, x, y);
    const auto rad = std::hypot(p.x, p.y);

    if (rad >= s->getHeight() / std::sqrt(2.) - 1.15) {  // TODO
        this->strokeAngle = std::atan2(p.y, p.x);
        stroke->addPoint(Point(x, y));
    } else {
        cairo_matrix_t matrix{};
        auto qx = rad * std::cos(this->strokeAngle);
        auto qy = -rad * std::sin(this->strokeAngle);
        s->getMatrix(matrix);
        cairo_matrix_transform_point(&matrix, &qx, &qy);

        stroke->addPoint(Point(qx, qy));
    }
    s->notify();
}

void SetsquareController::finalizeStroke() {
    hypotenuseMax = NAN;
    hypotenuseMin = NAN;
    addStrokeToLayer();
}

void SetsquareController::finalizeRadius() {
    strokeAngle = NAN;
    addStrokeToLayer();
}

auto SetsquareController::existsStroke() -> bool { return !std::isnan(hypotenuseMax) && !std::isnan(hypotenuseMin); }

auto SetsquareController::existsRadius() -> bool { return !std::isnan(strokeAngle); }
