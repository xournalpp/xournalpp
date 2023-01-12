/*
 * Xournal++
 *
 * A compass controller
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

class Compass;
class Stroke;

/**
 * @brief A class that controls a compass

 * The compass can be moved, rotated and scaled
 * There are methods for translating coordinates
 * and methods to deal with the temporary stroke
 * that is displayed near the the outline of the
 * compass or the marked radius.
 */

class CompassController: public GeometryToolController {
public:
    CompassController(XojPageView* view, Compass* compass);
    ~CompassController() override;

public:
    GeometryToolType getType() const override;

    /**
     * @brief returns the position of a point relative to a coordinate system, in which the
     * positive x-axis coincides with the distinguished compass axis and the origin lies
     * in the center of the compass
     * @param x the x-coordinate of the point (in document coordinates)
     * @param y the y-coordinate of the point (in document coordinates)
     */
    utl::Point<double> posRelToSide(double x, double y) const;

    /**
     * @brief checks whether a point with given coordinates lies in the geometry tool with an additional
     * border enlarging (or shrinked) it
     * @param x the x-coordinate of the given point (in document coordinates)
     * @param y the y-coordinate of the given point (in document coordinates)
     * @param border the size of the border (if negative, the geometry tool is shrinked via the border)
     */
    bool isInsideGeometryTool(double x, double y, double border = 0.0) const override;

    /**
     * @brief the point (in document coordinates) for a given angle on the outline of the compass
     * @param a the angle with respect to the distinguished compass axis of the point
     */
    utl::Point<double> getPointForAngle(double a) const;

    /**
     * @brief the point (in document coordinates) for a given radius on the marked radius of the compass
     * @param r the x-coordinate with respect to a coordinate system, in which the positive x-axis
     * coincides with the marked radius and the origin lies in the center of the compass
     */
    utl::Point<double> getPointForRadius(double r) const;

    /**
     * @brief creates a stroke starting at the given angle of the outline of the compass
     * @param a the angle of the point on the outline of the compass (when unrotated and untranslated)
     */
    void createOutlineStroke(double a);

    /**
     * @brief updates the stroke near the outline of the compass
     * @param a the angle of the point on the outline of the compass (when unrotated and untranslated)
     * updating the stroke
     */
    void updateOutlineStroke(double a);

    /**
     * @brief finishes the stroke near the outline of the compass
     */
    void finalizeOutlineStroke();

    /**
     * @brief finishes the stroke aligned to the marked radius of the compass
     */
    void finalizeRadialStroke();

    /**
     * @brief creates a stroke starting at the given point of the marked radius of the compass
     * @param x the x-coordinate with respect to the marked radius
     */
    void createRadialStroke(double x);

    /**
     * @brief updates the stroke near the marked radius of the compass
     * @param x the x-coordinate with respect to the marked radius
     */
    void updateRadialStroke(double x);

    /**
     * checks whether a stroke near the outline already exists
     */
    bool existsOutlineStroke();

    /**
     * checks whether a radius already exists
     */
    bool existsRadialStroke();

private:
    /**
     * @brief when a stroke near the radius with the measuring marks
     * is drawn, the minimal and maximal radii are saved in
     * the variables radiusMax and radiusMin
     *
     */
    double radiusMax = std::numeric_limits<double>::lowest();
    double radiusMin = std::numeric_limits<double>::max();

    /**
     * @brief when a stroke near the outline of the compass is drawn, the minimal and maximal
     * angles of the point to be drawn (with respect to an unrotated, and untranslated coordinate system)
     * are saved in the variables angleMin and angleMax
     */
    double angleMax = std::numeric_limits<double>::lowest();
    double angleMin = std::numeric_limits<double>::max();
};
