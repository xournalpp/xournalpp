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
#include "util/Point.h"
#include "view/SetsquareView.h"

enum Leg { HYPOTENUSE, LEFT_LEG, RIGHT_LEG };

/**
 * @brief A class that controls a setsquare

 * The setsquare can be moved, rotated and scaled
 * There are methods for translating coordinates
 * and methods to deal with the temporary stroke
 * that is displayed near the longest side or
 * the midpoint to the longest side of the setsquare.
 */

class SetsquareController {
public:
    SetsquareController(XojPageView* view, std::unique_ptr<SetsquareView>& setsquareView,
                        std::unique_ptr<Setsquare>& s);
    ~SetsquareController() = default;

public:
    double getHeight() const;
    double getRotation() const;
    double getTranslationX() const;
    double getTranslationY() const;

    /**
     * @brief moves the setquare in x- and y-direction
     * @param x the translation in x-direction (in document coordinates)
     * @param y the translation in y-direction (in document coordinates)
     */
    void move(double x, double y);

    /**
     * @brief rotates the setsquare around a given center
     * @param da the rotation angle
     * @param cx the x-coordinate of the rotation center (in document coordinates)
     * @param cy the y-coordinate of the rotation center (in document coordinates)
     */
    void rotate(double da, double cx, double cy);

    /**
     * @brief resizes the setsquare by the factor f with respect to a given scaling center
     * @param f the scaling factor
     * @param cx the x-coordinate of the scaling center (in document coordinates)
     * @param cy the y-coordinate of the scaling center (in document coordiantes)
     */
    void scale(double f, double cx, double cy);

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
    bool isInsideSetsquare(double x, double y, double border = 0.0) const;

    /**
     * @brief the point (in document coordinates) for a given position on the longest side of the setsquare
     * @param x the x-coordinate of the point on the longest side of the setsquare (when unrotated and untranslated)
     */
    utl::Point<double> getPointForPos(double x) const;

    /**
     * @brief creates a stroke starting at the given position of the longest side of the setsquare
     * @param x the x-coordinate of the point on the longest side of the setsquare (when unrotated and untranslated)
     */
    void createStroke(double x);

    /**
     * @brief updates the stroke aligned to the longest side of the setsquare
     * @param x the x-coordinate of the point on the longest side of the setsquare (when unrotated and untranslated)
     * updating the stroke
     */
    void updateStroke(double x);

    /**
     * @brief finishes the stroke aligned to the longest side of the setsquare
     */
    void finalizeStroke();

    /**
     * @brief creates a radius starting at the given position to the origin of the setsquare
     * @param x the x-coordinate of the current point
     * @param y the y-coordinate of the current point
     */
    void createRadius(double x, double y);

    /**
     * @brief updates the radius to the origin of the setsquare
     * @param x the x-coordinate of the current point
     * @param y the y-coordinate of the current point
     * updating the stroke
     */
    void updateRadius(double x, double y);

    /**
     * @brief finishes the radius to the origin of the setsquare
     */
    void finalizeRadius();

    /**
     * @brief adds the stroke to the layer and rerenders the stroke area
     */
    void addStrokeToLayer();

    /**
     * @brief initializes the stroke by using the properties from the tool handler
     */
    void initializeStroke();

    /**
     * checks whether a stroke already exists
     */
    bool existsStroke();

    /**
     * checks whether a radius already exists
     */
    bool existsRadius();

    /**
     * @brief the page view of the page with respect to which the setsquare is initialized
     */
    XojPageView* getView() const;

    /**
     * @brief the page with respect to which the setsquare is initialized
     */
    PageRef getPage() const;

    /**
     * @brief paints the setsquare and temporary stroke to a cairo context, after scaling it by the zoom factor
     * @param cr the cairo context
     */
    void paint(cairo_t* cr);

private:
    XojPageView* view;

    std::unique_ptr<SetsquareView> setsquareView;

    /**
     * @brief the underlying setsquare
     */
    std::unique_ptr<Setsquare> s;

    /**
     * @brief The stroke drawn aligned to the longest side of the setsquare or ending at the midpoint of the longest
     * side of the setsquare
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
     * @brief the current angle at which the temporary radius is drawn
     */
    double strokeAngle = NAN;
};
