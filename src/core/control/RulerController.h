/*
 * Xournal++
 *
 * Controls the physical ruler geometry tool.
 */

#pragma once

#include <limits>

#include "control/GeometryToolController.h"

class Ruler;

class RulerController: public GeometryToolController {
public:
    RulerController(XojPageView* view, Ruler* ruler);
    ~RulerController() override;

    GeometryToolType getType() const override;
    bool isInsideGeometryTool(double x, double y, double border = 0.) const override;

    xoj::util::Point<double> posRelToRuler(double x, double y) const;
    bool isNearRulerEdge(double x, double y, double tolerancePixels, double zoom) const;
    void createStroke(double x);
    void updateStroke(double x);
    void finalizeStroke();
    void cancelStroke();
    bool existsStroke() const;

private:
    double strokeMin = std::numeric_limits<double>::max();
    double strokeMax = std::numeric_limits<double>::lowest();
};
