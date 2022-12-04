/*
 * Xournal++
 *
 * A setsquare model
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cmath>

#include "model/GeometryTool.h"

/**
 * @brief A class that models a setsquare
 *
 * The setsquare has the shape of a right-angled isosceles triangle and has a certain height, may be rotated and
 * translated. User coordinates are specified in cm.
 */

class SetsquareInputHandler;

namespace xoj::view {
class SetsquareView;
};


class Setsquare: public GeometryTool {
public:
    Setsquare();

    /**
     * @brief A setsquare specified by its height, rotation angle and translations in x- and y-directions
     * @param height the height of the setsquare
     * @param rotation the angle (in radian) around which the setsquare is rotated with respect to the x-axis
     * @param x the x-coordinate o(in pt) f the mid point of the longest side of the setsquare
     * @param y the y-coordinate (in pt) of the mid point of the longest side of the setsquare
     */
    Setsquare(double height, double rotation, double x, double y);

    virtual ~Setsquare();

    void notify(bool resetMask) const override;  // calls the update method of all observers
    Range getToolRange(bool transformed) const override;

    // parameters used when initially displaying setsquare on a page
    static constexpr double INITIAL_HEIGHT = 8.0;
    static constexpr double INITIAL_X = 21. * HALF_CM;
    static constexpr double INITIAL_Y = 15. * HALF_CM;
    // relation between setsquare height and radius
    static constexpr double DISTANCE_SEMICIRCLE_FROM_LEGS = 1.15;
    static inline double radiusFromHeight(double height) {
        return height / std::sqrt(2.) - DISTANCE_SEMICIRCLE_FROM_LEGS;
    }
};
