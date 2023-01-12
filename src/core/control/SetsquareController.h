/*
 * Xournal++
 *
 * A setsquare controller
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>

#include "gui/PageView.h"
#include "model/GeometryTool.h"
#include "util/Point.h"

#include "GeometryToolController.h"

class Setsquare;
class Stroke;

enum Leg { HYPOTENUSE, LEFT_LEG, RIGHT_LEG };

/**
 * @brief A class that controls a setsquare

 * The setsquare can be moved, rotated and scaled
 * There are methods for translating coordinates
 * and methods to deal with the temporary stroke
 * that is displayed near the longest side or
 * the midpoint to the longest side of the setsquare.
 */

class SetsquareController: public GeometryToolController {
public:
    SetsquareController(XojPageView* view, Setsquare* setsquare);
    ~SetsquareController() override;

public:
    GeometryToolType getType() const override;

    /**
     * @brief returns the position of a point relative to a coordinate system, in which the given setsquare leg lies on
     * the x-axis with the origin in its center (where the unit is 1 cm)
     * @param leg the leg of the setsquare
     * @param x the x-coordinate of the point (in document coordinates)
     * @param y the y-coordinate of the point (in document coordinates)
     */
    utl::Point<double> posRelToSide(Leg leg, double x, double y) const;

    /**
     * @brief checks whether a point with given coordinates lies in the setsquare with an additional border enlarging
     * (or shrinked) it
     * @param x the x-coordinate of the given point (in document coordinates)
     * @param y the y-coordinate of the given point (in document coordinates)
     * @param border the size of the border (if negative, the setsquare is shrinked via the border)
     */
    bool isInsideGeometryTool(double x, double y, double border = 0.0) const override;

    /**
     * @brief the point (in document coordinates) for a given position on the longest side of the setsquare
     * @param x the x-coordinate of the point on the longest side of the setsquare (when unrotated and untranslated)
     */
    utl::Point<double> getPointForPos(double x) const;

    /**
     * @brief creates a stroke starting at the given position of the longest side of the setsquare
     * @param x the x-coordinate of the point on the longest side of the setsquare (when unrotated and untranslated)
     */
    void createEdgeStroke(double x);

    /**
     * @brief updates the stroke aligned to the longest side of the setsquare
     * @param x the x-coordinate of the point on the longest side of the setsquare (when unrotated and untranslated)
     * updating the stroke
     */
    void updateEdgeStroke(double x);

    /**
     * @brief finishes the stroke aligned to the longest side of the setsquare
     */
    void finalizeEdgeStroke();

    /**
     * @brief creates a radius starting at the given position to the origin of the setsquare
     * @param x the x-coordinate of the current point
     * @param y the y-coordinate of the current point
     */
    void createRadialStroke(double x, double y);

    /**
     * @brief updates the radius to the origin of the setsquare
     * @param x the x-coordinate of the current point
     * @param y the y-coordinate of the current point
     * updating the stroke
     */
    void updateRadialStroke(double x, double y);

    /**
     * @brief finishes the radius to the origin of the setsquare
     */
    void finalizeRadialStroke();

    /**
     * checks whether a stroke already exists
     */
    bool existsEdgeStroke();

    /**
     * checks whether a radius already exists
     */
    bool existsRadialStroke();

private:
    /**
     * @brief when a stroke aligned to the longest side (hypotenuse) of the setsquare is drawn, the minimal and maximal
     * x-coordinates of the point to be drawn (with respect to an unrotated, and untranslated coordinate system) are
     * saved in the variables hypotenuseMin and hypotenuseMax
     */
    double hypotenuseMax = std::numeric_limits<double>::lowest();
    double hypotenuseMin = std::numeric_limits<double>::max();

    /**
     * @brief the current angle at which the temporary radius is drawn
     */
    double strokeAngle = NAN;
};
