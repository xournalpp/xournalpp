#include "Setsquare.h"

#include <cmath>
#include <ctime>
#include <iostream>


Setsquare::Setsquare(): m_size(8.0), m_rotation(0.0), m_translationX(14.17 * 21), m_translationY(14.17 * 15) {}

Setsquare::Setsquare(double size, double rotation, double x, double y):
        m_size(size), m_rotation(rotation), m_translationX(x), m_translationY(y) {}

Setsquare::~Setsquare() {}

void Setsquare::updateValues() {
    m_radius = (m_size / std::sqrt(2) - .65) - 0.5;
    m_dist_vmarks = std::min(2.5, m_radius * .6);
    m_num_vmarks = static_cast<int>(std::floor(std::sqrt(std::pow(m_radius, 2) - std::pow(m_dist_vmarks, 2)) * 10) - 2);
    m_offset = std::max(2.0, 256 / pow(m_size, 2));
    m_num_hmarks = static_cast<int>(std::round((m_size - 1) * 10));
    // g_print("m_size = %f, m_radius = %f, m_dist_vmarks = %f, m_num_vmarks = %f. \n", m_size, m_radius, m_dist_vmarks,
    // m_num_vmarks);
}

void Setsquare::setSize(double size) { m_size = size; }
auto Setsquare::getSize() -> double { return m_size; }

void Setsquare::setRotation(double rotation) { m_rotation = rotation; }
auto Setsquare::getRotation() -> double { return m_rotation; }

void Setsquare::setTranslationX(double x) { m_translationX = x; }
auto Setsquare::getTranslationX() -> double { return m_translationX; }

void Setsquare::setTranslationY(double y) { m_translationY = y; }
auto Setsquare::getTranslationY() -> double { return m_translationY; }

void Setsquare::move(double x, double y) {
    m_translationX += x;
    m_translationY += y;
}

void Setsquare::rotate(double da) { m_rotation += da; }

void Setsquare::scale(double f) { m_size *= f; }

void Setsquare::paint(cairo_t* cr) {
    cairo_save(cr);

    // const int width = 2000;
    // const int height = 1000;

    //  cairo_translate(width/2, 0);
    //  cairo_scale(width/(2*m_size), height/m_size);
    cairo_translate(cr, m_translationX, m_translationY);
    cairo_rotate(cr, m_rotation);
    cairo_scale(cr, 14.17 * 2, 14.17 * 2);
    cairo_set_line_width(cr, m_line_width);
    cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, .2);

    updateValues();

    cairo_set_source_rgb(cr, 1, 0, 0);
    drawOutline(cr);

    cairo_set_source_rgb(cr, 0, 0, 1);
    drawHorizontalMarks(cr);

    cairo_set_source_rgb(cr, 0, .5, 0);
    drawVerticalMarks(cr);

    cairo_set_source_rgb(cr, .5, 0, .5);
    drawAngularMarks(cr);
    cairo_restore(cr);
}

void Setsquare::drawOutline(cairo_t* cr) {
    cairo_save(cr);
    // BEGIN: outline
    cairo_move_to(cr, m_size, 0);
    cairo_line_to(cr, -m_size, 0);
    cairo_line_to(cr, 0, m_size);
    cairo_close_path(cr);
    cairo_stroke_preserve(cr);
    cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 0.1);
    cairo_fill(cr);
    // END: outline
    cairo_restore(cr);
}

void Setsquare::drawAngularMarks(cairo_t* cr) {
    cairo_save(cr);
    // BEGIN: 45 degree marks
    // clipping region around marks parallel to hypotenuse in center
    for (auto i = .5; i <= m_num_vmarks / 10; i += .5) {
        double x = sqrt(pow(m_radius - 0.25, 2) - pow(i, 2));
        cairo_rectangle(cr, -x, i - 0.15, 2 * x, 0.3);
    }
    cairo_rectangle(cr, -m_size, 0, 2 * m_size, m_size);  // clip to the outside
    cairo_clip(cr);

    // clip vertical stripes
    cairo_rectangle(cr, -m_dist_vmarks - 0.25, 0.25, .75, m_num_vmarks / 10 + 0.25);
    cairo_rectangle(cr, m_dist_vmarks - 0.5, 0.25, .75, m_num_vmarks / 10 + 0.25);
    cairo_rectangle(cr, -m_size, 0, 2 * m_size, m_size);  // clip to the outside
    cairo_clip(cr);

    // draw marks
    for (auto i: {45 * M_PI / 180, 135 * M_PI / 180}) {
        const double rad1 = 1;                                   // 1
        const double rad2 = m_dist_vmarks * std::sqrt(2) - 0.5;  // 3
        const double rad3 = m_radius + 0.3;                      // 4.8
        const double rad4 = m_size / std::sqrt(2) - 0.3;         // 5.4
        cairo_move_to(cr, cos(i) * rad1, sin(i) * rad1);
        cairo_line_to(cr, cos(i) * rad2, sin(i) * rad2);
        cairo_move_to(cr, cos(i) * rad3, sin(i) * rad3);
        cairo_line_to(cr, cos(i) * rad4, sin(i) * rad4);
    }
    cairo_stroke(cr);
    cairo_reset_clip(cr);
    // END: 45 degree marks

    // BEGIN: marks and numbers around semicircle
    for (int i = 1; i < 180; i++) {
        double x = m_size * sin(45 * M_PI / 180) / sin((i > 90 ? i - 45 : 135 - i) * M_PI / 180);
        size_t n = 180 - i;
        double t = ((i % 5) == 0) / 10.0 + .1;
        cairo_move_to(cr, x * cos(i * M_PI / 180), x * sin(i * M_PI / 180));
        if (i % 10 == 0 && i > m_offset && i < 180 - m_offset) {
            cairo_line_to(cr, (m_radius + .8) * cos(i * M_PI / 180), (m_radius + .8) * sin(i * M_PI / 180));
            cairo_move_to(cr, m_radius * cos(i * M_PI / 180), m_radius * sin(i * M_PI / 180));
            cairo_rel_move_to(cr, 0.3 * cos(i * M_PI / 180), 0.3 * sin(i * M_PI / 180));
            showTextCenteredAndRotated(cr, std::to_string(i), 0.2, i + 270);
            if (n != 90) {
                cairo_move_to(cr, m_radius * cos(i * M_PI / 180), m_radius * sin(i * M_PI / 180));
                cairo_rel_move_to(cr, 0.6 * cos(i * M_PI / 180), 0.6 * sin(i * M_PI / 180));
                showTextCenteredAndRotated(cr, std::to_string(180 - i), 0.2, i + 270);
            }
        } else {
            cairo_rel_line_to(cr, -t * cos(i * M_PI / 180), -t * sin(i * M_PI / 180));
            if (i > m_offset && i < 180 - m_offset) {
                cairo_move_to(cr, m_radius * cos(i * M_PI / 180), m_radius * sin(i * M_PI / 180));
                cairo_rel_line_to(cr, t * cos(i * M_PI / 180), t * sin(i * M_PI / 180));
            }
        }
    }
    cairo_stroke(cr);
    // END: marks and numbers around semicircle
    cairo_restore(cr);
}

void Setsquare::drawVerticalMarks(cairo_t* cr) {
    cairo_save(cr);
    // BEGIN: vertical marks in center
    const size_t max = (size_t)(m_num_vmarks / 10);
    for (auto i = 1; i <= max; i++) {
        cairo_move_to(cr, 0, i - .25);
        cairo_rel_line_to(cr, 0, .5);
    }
    cairo_stroke(cr);
    // END: vertical marks in center

    // BEGIN: VERTICAL marks within circle
    // clip vertical stripes
    cairo_rectangle(cr, -m_dist_vmarks - 0.25, 0.25, .75, m_num_vmarks / 10 + 0.25);
    cairo_rectangle(cr, m_dist_vmarks - 0.5, 0.25, .75, m_num_vmarks / 10 + 0.25);
    cairo_rectangle(cr, -.25, 0.25, .5, m_num_vmarks / 10 + 0.25);
    cairo_rectangle(cr, -m_size, 0, 2 * m_size, m_size);  // clip to the outside
    cairo_clip(cr);


    // draw marks
    for (auto i = .5; i <= m_num_vmarks / 10; i += .5) {
        double x = sqrt(pow(m_radius - 0.25, 2) - pow(i, 2));
        cairo_move_to(cr, -x, i);
        cairo_line_to(cr, x, i);
        cairo_stroke(cr);
    }
    cairo_reset_clip(cr);
    // END: VERTICAL marks within circle


    // BEGIN: minor vertical marks with numbers
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, .2);

    for (int i = 3; i <= m_num_vmarks; i++) {
        // double x = sqrt(pow(90, 2) - pow(2 * i, 2));
        int n = (int)floor(i / 10);
        std::string text = std::to_string(n);
        int j = (i % 5 == 0);
        cairo_move_to(cr, -m_dist_vmarks, (double)i / 10.0);
        cairo_rel_line_to(cr, .1 + j / 10.0, 0);
        if (i % 10 == 0) {
            cairo_rel_move_to(cr, .12, .06);
            cairo_show_text(cr, text.c_str());
        };
        cairo_move_to(cr, m_dist_vmarks, (double)i / 10.0);
        cairo_rel_line_to(cr, (-.1 - j / 10.0), 0);
        if (i % 10 == 0) {
            cairo_rel_move_to(cr, -.24, .06);
            cairo_show_text(cr, text.c_str());
        };
    }
    cairo_stroke(cr);
    // END: minor vertical marks with numbers
    cairo_restore(cr);
}

void Setsquare::drawHorizontalMarks(cairo_t* cr) {
    cairo_save(cr);
    // BEGIN: measuring marks on top

    for (int i = -m_num_hmarks; i <= m_num_hmarks; i++) {
        int n = abs(i / 10);
        double t = ((i % 5) == 0) / 10.0 + .1;
        cairo_move_to(cr, (double)i / 10.0, 0);
        cairo_rel_line_to(cr, 0, t);
        if (i % 10 == 0) {
            cairo_rel_move_to(cr, -0.06, 0.18);
            cairo_show_text(cr, std::to_string(n).c_str());
        }
    }
    cairo_stroke(cr);
    // END: measuring marks on top
    cairo_restore(cr);
}

void Setsquare::showTextCenteredAndRotated(cairo_t* cr, std::string text, double fontSize, double angle) {
    cairo_save(cr);
    cairo_matrix_t matrix;
    cairo_matrix_init_identity(&matrix);
    cairo_text_extents_t te;
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_text_extents(cr, text.c_str(), &te);
    double dx = te.x_bearing + te.width / 2;
    double dy = te.y_bearing + te.height / 2;
    // g_print("dx = %f,  dy = %f. \n", dx, dy);
    cairo_matrix_scale(&matrix, fontSize, fontSize);
    cairo_matrix_translate(&matrix, dx / fontSize, dy / fontSize);
    cairo_matrix_rotate(&matrix, angle * M_PI / 180.0);
    cairo_matrix_translate(&matrix, -dx / fontSize, -dy / fontSize);
    cairo_set_font_matrix(cr, &matrix);
    cairo_rel_move_to(cr, -dx, -dy);
    cairo_show_text(cr, text.c_str());
    cairo_rel_move_to(cr, dx, dy);
    cairo_restore(cr);
}
