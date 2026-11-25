/*
 * Xournal++
 *
 * A physical ruler geometry tool.
 */

#pragma once

#include "model/GeometryTool.h"

class Ruler: public GeometryTool {
public:
    static constexpr double INITIAL_LENGTH = 30.0;
    static constexpr double THICKNESS = 2.0;

    Ruler();
    Ruler(double length, double rotation, double x, double y);
    ~Ruler() override;

    void notify(bool resetMask = false) const override;
    Range getToolRange(bool transformed) const override;
};
