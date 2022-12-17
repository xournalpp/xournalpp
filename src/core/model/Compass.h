/*
 * Xournal++
 *
 * A compass model
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/GeometryTool.h"

/**
 * @brief A class that models a compass
 *
 * The compass has the shape of a circle and has a certain height (radius), may be rotated and
 * translated. User coordinates are specified in cm.
 */

namespace xoj::view {
class CompassView;
};

class CompassInputHandler;

class Compass: public GeometryTool {
public:
    Compass();

    /**
     * @brief A compass specified by its radius, rotation angle and translations in x- and y-directions
     * @param height the radius of the compass
     * @param rotation the angle (in radian) around which the compass is rotated with respect to the x-axis
     * @param x the x-coordinate (in pt) of the center of the compass
     * @param y the y-coordinate (in pt) of the center of the compass
     */
    Compass(double height, double rotation, double x, double y);

    virtual ~Compass();

    void notify(bool resetMask) const override;  // calls the update method of all observers
    Range getToolRange(bool transformed) const override;

    // parameters used when initially displaying the compass on a page
    static constexpr double INITIAL_HEIGHT = 3.0;
    static constexpr double INITIAL_X = 21. * HALF_CM;
    static constexpr double INITIAL_Y = 15. * HALF_CM;
};
