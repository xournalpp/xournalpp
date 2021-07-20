/*
 * Xournal++
 *
 * A setsquare
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>

#include <gtk/gtk.h>


class Setsquare {
public:
    Setsquare();
    Setsquare(double size, double rotation, double x, double y);
    virtual ~Setsquare();
    void paint(cairo_t* cr);
    void setSize(double size);
    double getSize();
    void setRotation(double rotation);
    double getRotation();
    void setTranslationX(double x);
    double getTranslationX();
    void setTranslationY(double y);
    double getTranslationY();
    void move(double x, double y);
    void rotate(double da);
    void scale(double f);

protected:
    double m_size = 8;
    double m_rotation = 0.0;
    double m_translationX = 14.17 * 21;
    double m_translationY = 14.17 * 15;

    double m_line_width = 0.02;
    double m_radius = 4.5;
    double m_dist_vmarks = 2.5;
    double m_num_vmarks = 35;
    double m_offset = 4.0;
    int m_num_hmarks = 70;

    void drawVerticalMarks(cairo_t* cr);
    void drawHorizontalMarks(cairo_t* cr);
    void drawAngularMarks(cairo_t* cr);
    void drawOutline(cairo_t* cr);
    void updateValues();
    void showTextCenteredAndRotated(cairo_t* cr, std::string text, double fontSize, double angle);
};
