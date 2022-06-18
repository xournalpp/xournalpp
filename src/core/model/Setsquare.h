/*
 * Xournal++
 *
 * A setsquare, model and paiting to a cairo context
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string

#include <cairo.h>  // for cairo_t, cairo_matrix_t

constexpr static double HALF_CM = 14.17;
constexpr static double CM = 2. * HALF_CM;

/**
 * @brief A class that models a setsquare, including method to paint the setsquare to a cairo context
 *
 * The setsquare has the shape of a right-angled isosceles triangle and has a certain height, may be rotated and
 * translated. User coordinates are specified in cm. The setsquare has vertical, horizontal and angular marks.
 * The intersection angle with the x-axis is displayed as well
 */

class Setsquare {
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

    /**
     * @brief paints the setsquare to a cairo context
     * @param cr the cairo context
     */
    void paint(cairo_t* cr);

    void setHeight(double height);
    double getHeight() const;
    void setRotation(double rotation);
    double getRotation() const;
    void setTranslationX(double x);
    double getTranslationX() const;
    void setTranslationY(double y);
    double getTranslationY() const;
    double getRadius() const;

    /**
     * @brief translates the setsquare
     * @param x the amount of translation in x-direction
     * @param y the amount of translation in y-direction
     */
    void move(double x, double y);

    /**
     * @brief rotates the setsquare around the mid point of its longest side
     * @param da the angle increase (in radian)
     */
    void rotate(double da);

    /**
     * @brief central scaling of the setsquare with center equal to the mid point of its longest side
     * @param f the scaling factor
     */
    void scale(double f);

    /**
     * @brief returns the matrix which translates from user coordinates (in which the setsquare
     * has its longest side on the x-axis with its mid point equal to the origin) to document coordinates
     * @param matrix the matrix into which the result gets stored
     */
    void getMatrix(cairo_matrix_t& matrix) const;

private:
    /**
     * @brief the height of the setsquare (regarded as an isoceles triangle)
     */
    double height = 8.0;

    /**
     * @brief the angle (in radian) around which the setsquare is rotated with respect to the x-axis
     */
    double rotation = 0.0;

    /**
     * @brief the x-coordinate (in pt) of the mid point of the longest side of the setsquare (by default 10.5 cm)
     */
    double translationX = 21. * HALF_CM;

    /**
     * @brief the y-coordinate (in pt) of the mid point of the longest side of the setsquare (by default 7.5 cm)
     */
    double translationY = 15 * HALF_CM;

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
};
