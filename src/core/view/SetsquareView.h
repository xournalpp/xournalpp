/*
 * Xournal++
 *
 * A setsquare view
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>

#include "gui/PageView.h"
#include "model/Setsquare.h"
#include "util/Point.h"

class Setsquare;

/**
 * @brief A class that renders a setsquare
 *
 * The setsquare has vertical, horizontal and angular marks.
 * The intersection angle with the x-axis is displayed as well.
 *
 * See the file development/documentation/Setsquare_Readme.md for
 * details how the setsquare is rendered.
 *
 * A temporary stroke is displayed when it is created near the
 * longest side of the setsquare or from a point to the mid point
 * of the longest side of the setsquare.
 */

class SetsquareView {
public:
    SetsquareView(Setsquare* s);
    ~SetsquareView() = default;

public:
    /**
     * @brief paints the setsquare and temporary stroke to a cairo context
     * @param cr the cairo context
     */
    void paint(cairo_t* cr);

    /**
     * @brief the radius of the semicircle displayed on the setsquare
     */
    double getRadius() const;

    void setStroke(Stroke* stroke);

private:
    /**
     * @brief draws the setsquare to a cairo context
     * @param cr the cairo context drawn to
     */
    void drawSetsquare(cairo_t* cr);

    /**
     * @brief draws the temporary stroke at the longest side of the setsquare to a cairo context
     * @param cr the cairo context drawn to
     */
    void drawTemporaryStroke(cairo_t* cr);

    /**
     * @brief the radius of the semi-circle for the angular marks
     */
    double radius = 4.5;

    /**
     * @brief the distance of the circle containing the rotation angle display from the mid point of the longest side of
     * the setsquare
     */
    double circlePos = 6.0;

    /**
     * @brief the distance (in cm) of the vertical marks from the symmetry axis of the setsquare
     */
    double horPosVmarks = 2.5;

    /**
     * @brief the index of the first vertical mark which should be drawn (which should not overlap with the measuring
     * marks)
     */
    int minVmark = 3;

    /**
     * @brief the index of the last vertical mark to be drawn
     */
    int maxVmark = 35;

    /**
     * @brief the number of angular marks that are left away on both ends (in order not to overlap with the measuring
     * marks)
     */
    int offset = 4;

    /**
     * @brief the index of the last horizontal mark to be drawn
     */
    int maxHmark = 70;

    void drawVerticalMarks(cairo_t* cr) const;
    void drawHorizontalMarks(cairo_t* cr) const;
    void drawAngularMarks(cairo_t* cr) const;
    void drawOutline(cairo_t* cr) const;
    void drawRotation(cairo_t* cr) const;
    void clipVerticalStripes(cairo_t* cr) const;
    void clipHorizontalStripes(cairo_t* cr) const;

    /**
     * @brief updates the values radius, horPosVmarks, minVmark, maxVmark computed from the height of the setsquare
     */
    void updateValues();

    /**
     * @brief renders text centered and possibly rotated at the current position on a cairo context
     * @param cr the cairo context
     * @param text the text string
     * @param angle the rotation angle
     */
    void showTextCenteredAndRotated(cairo_t* cr, std::string text, double angle) const;

private:
    /**
     * @brief the underlying setsquare
     */
    Setsquare* s;

    /**
     * @brief The stroke drawn aligned to the longest side of the setsquare
     */
    Stroke* stroke = nullptr;

    /**
     * @brief when a stroke aligned to the longest side (hypotenuse) of the setsquare is drawn, the minimal and maximal
     * x-coordinates of the point to be drawn (with respect to an unrotated, and untranslated coordinate system) are
     * saved in the variables hypotenuseMin and hypotenuseMax
     */
    double hypotenuseMax = NAN;
    double hypotenuseMin = NAN;

    /**
     * @brief
     */
    double strokeAngle = NAN;
};
