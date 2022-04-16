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

#include <gtk/gtk.h>

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

    void setHeight(double height);
    double getHeight() const;
    void setRotation(double rotation);
    double getRotation() const;
    void setTranslationX(double x);
    double getTranslationX() const;
    void setTranslationY(double y);
    double getTranslationY() const;

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
};
