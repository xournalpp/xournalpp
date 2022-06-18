#include "Setsquare.h"

#include <algorithm>         // for max, min
#include <cmath>             // for sin, sqrt, abs, pow, cos, floor, remainder
#include <cstdlib>           // for abs
#include <initializer_list>  // for initializer_list
#include <iomanip>           // for operator<<, setprecision
#include <sstream>           // for stringstream, basic_ostream, fixed, basi...

constexpr double LINE_WIDTH = .02;
constexpr double FONT_SIZE = .2;
constexpr double CIRCLE_RAD = .3;
constexpr double TICK_SMALL = .1;
constexpr double TICK_LARGE = .2;
constexpr double DISTANCE_SEMICIRCLE_FROM_LEGS = 1.15;
constexpr double MAX_HOR_POS_VMARKS = 2.5;
constexpr int MIN_OFFSET_ANG_MARKS = 2;
constexpr double RELATIVE_CIRCLE_POS = .75;
constexpr double RELATIVE_MAX_HOR_POS_VMARKS = .6;
constexpr double HMARK_POS = 2.;
constexpr double MIN_DIST_FROM_HMARK = .2;
constexpr int MIN_VMARK_SMALL = 3;
constexpr int MIN_VMARK_LARGE = 5;
constexpr int OFFSET_FROM_SEMICIRCLE = 2.;
constexpr double ZERO_MARK_TICK = .5;
constexpr int SKIPPED_HMARKS = 8;

// parameters used when initially displaying setsquare on a page
constexpr double INITIAL_HEIGHT = 8.0;
constexpr double INITIAL_X = 21. * HALF_CM;
constexpr double INITIAL_Y = 15. * HALF_CM;

constexpr double rad(double n) { return n * M_PI / 180.; }
constexpr double rad(int n) { return rad(static_cast<double>(n)); }
constexpr double deg(double a) { return a * 180.0 / M_PI; }
inline double cathete(double h, double o) { return std::sqrt(std::pow(h, 2) - std::pow(o, 2)); }

Setsquare::Setsquare(): height(INITIAL_HEIGHT), rotation(.0), translationX(INITIAL_X), translationY(INITIAL_Y) {}

Setsquare::Setsquare(double height, double rotation, double x, double y):
        height(height), rotation(rotation), translationX(x), translationY(y) {}

Setsquare::~Setsquare() {}

void Setsquare::updateValues() {
    radius = height / std::sqrt(2.) - DISTANCE_SEMICIRCLE_FROM_LEGS;
    circlePos = height * RELATIVE_CIRCLE_POS;
    horPosVmarks = std::min(MAX_HOR_POS_VMARKS, radius * RELATIVE_MAX_HOR_POS_VMARKS);
    minVmark = (std::abs(horPosVmarks - HMARK_POS - TICK_SMALL / 2.) < MIN_DIST_FROM_HMARK - TICK_SMALL / 2.) ?
                       MIN_VMARK_LARGE :
                       MIN_VMARK_SMALL;
    maxVmark = static_cast<int>(std::floor(cathete(radius, horPosVmarks) * 10.0)) - OFFSET_FROM_SEMICIRCLE;

    // The following computation of the angular marks offset is based on experimentation.
    offset = std::max(MIN_OFFSET_ANG_MARKS,
                      static_cast<int>(256.0 / std::pow(height, 2)));  // larger offset for small height
    if (std ::abs(radius - std::round(radius)) < .2) {  // larger offset when semicircle comes close to big hmarks
        offset = std::max(offset, static_cast<int>(24.0 / radius));
    }
    maxHmark = static_cast<int>(std::floor(height * 10.0)) - SKIPPED_HMARKS;
}

void Setsquare::setHeight(double height) { this->height = height; }
auto Setsquare::getHeight() const -> double { return this->height; }

void Setsquare::setRotation(double rotation) { this->rotation = rotation; }
auto Setsquare::getRotation() const -> double { return this->rotation; }

void Setsquare::setTranslationX(double x) { this->translationX = x; }
auto Setsquare::getTranslationX() const -> double { return this->translationX; }

void Setsquare::setTranslationY(double y) { this->translationY = y; }
auto Setsquare::getTranslationY() const -> double { return this->translationY; }

auto Setsquare::getRadius() const -> double { return this->radius; }

void Setsquare::move(double x, double y) {
    this->translationX += x;
    this->translationY += y;
}

void Setsquare::rotate(double da) { this->rotation += da; }

void Setsquare::scale(double f) { this->height *= f; }

void Setsquare::getMatrix(cairo_matrix_t& matrix) const {
    cairo_matrix_init_identity(&matrix);
    cairo_matrix_translate(&matrix, this->translationX, this->translationY);
    cairo_matrix_rotate(&matrix, this->rotation);
    cairo_matrix_scale(&matrix, CM, CM);
}

void Setsquare::paint(cairo_t* cr) {
    cairo_save(cr);

    cairo_matrix_t matrix{};
    getMatrix(matrix);
    cairo_transform(cr, &matrix);
    cairo_set_line_width(cr, LINE_WIDTH);
    cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, FONT_SIZE);

    updateValues();

    // clip to slightly enlarged setsquare for performance reasons
    const double enlargedHeight = this->height + LINE_WIDTH;
    cairo_move_to(cr, enlargedHeight, -LINE_WIDTH);
    cairo_line_to(cr, -enlargedHeight, -LINE_WIDTH);
    cairo_line_to(cr, .0, enlargedHeight);
    cairo_close_path(cr);
    cairo_clip(cr);

    cairo_set_source_rgb(cr, 1., .0, .0);  // red
    drawOutline(cr);

    cairo_set_source_rgb(cr, .0, .0, 1.);  // blue
    drawHorizontalMarks(cr);

    cairo_set_source_rgb(cr, .0, .5, .0);  // green
    drawVerticalMarks(cr);

    cairo_set_source_rgb(cr, .5, .0, .5);  // violet
    drawAngularMarks(cr);

    cairo_set_source_rgb(cr, .0, .5, .5);  // turquoise
    drawRotation(cr);

    cairo_restore(cr);
}

void Setsquare::drawOutline(cairo_t* cr) const {
    cairo_save(cr);

    cairo_move_to(cr, this->height, .0);
    cairo_line_to(cr, -this->height, .0);
    cairo_line_to(cr, .0, this->height);
    cairo_close_path(cr);
    cairo_stroke_preserve(cr);

    cairo_set_source_rgba(cr, .2, .2, .2, .1);  // transparent gray
    cairo_fill(cr);

    cairo_restore(cr);
}

void Setsquare::drawRotation(cairo_t* cr) const {
    cairo_save(cr);

    // write the angle between hypotenuse and horizontal axis within a small circle
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << std::abs(std::remainder(deg(this->rotation), 180.));
    cairo_move_to(cr, .0, this->circlePos);
    showTextCenteredAndRotated(cr, ss.str(), -deg(this->rotation));

    cairo_move_to(cr, .0, this->circlePos);
    cairo_new_sub_path(cr);
    cairo_arc(cr, .0, this->circlePos, CIRCLE_RAD, .0, 2. * M_PI);
    cairo_stroke(cr);

    cairo_restore(cr);
}

void Setsquare::drawAngularMarks(cairo_t* cr) const {
    cairo_save(cr);

    // BEGIN: 45 degree marks
    clipHorizontalStripes(cr);
    clipVerticalStripes(cr);

    for (int i: {45, 135}) {
        const double cs = std::cos(rad(i));
        const double si = std::sin(rad(i));

        // inside semicircle
        const double radInsideUpper = this->radius - .3;  // 4.2
        cairo_move_to(cr, cs, si);
        cairo_line_to(cr, radInsideUpper * cs, radInsideUpper * si);

        // outside semicircle
        const double radOutsideLower = this->radius + .3;                  // 4.8
        const double radOutsideUpper = this->height / std::sqrt(2.) - .3;  // 5.4
        cairo_move_to(cr, radOutsideLower * cs, radOutsideLower * si);
        cairo_line_to(cr, radOutsideUpper * cs, radOutsideUpper * si);
    }
    cairo_stroke(cr);
    cairo_reset_clip(cr);
    // END: 45 degree marks

    // BEGIN: marks and numbers around semicircle
    for (int i = 1; i < 180; i++) {
        // the radius corresponding to the point with angle i on one of the short sides of the setsquare
        // it is computed using the law of sines
        const double radCath = this->height * std::sin(rad(45)) / std::sin(rad(i > 90 ? i - 45 : 135 - i));
        const double cs = std::cos(rad(i));
        const double si = std::sin(rad(i));
        const double tick = (i % 5 == 0) ? TICK_LARGE : TICK_SMALL;
        cairo_move_to(cr, radCath * cs, radCath * si);  // move to one of the short sides of the set square
        if (i % 10 == 0 && i > this->offset && i < 180 - this->offset) {
            // large tick near short side of set square
            const double radTickEnd = (i == 90) ? (this->circlePos + .5) : (this->radius + .8);
            cairo_line_to(cr, radTickEnd * cs, radTickEnd * si);

            // show increasing numbers near semi-circle
            const double radInc = this->radius + 0.3;
            cairo_move_to(cr, radInc * cs, radInc * si);
            showTextCenteredAndRotated(cr, std::to_string(i), i + 270);

            // show decreasing numbers a bit further from semi-circle
            if (i != 90) {
                const double radDec = this->radius + 0.6;
                cairo_move_to(cr, radDec * cs, radDec * si);
                showTextCenteredAndRotated(cr, std::to_string(180 - i), i + 270);
            }
        } else {
            cairo_rel_line_to(cr, -tick * cs, -tick * si);  // just a tick near short side of set square
        }

        if (i > this->offset && i < 180 - this->offset) {
            // tick near semi-circle
            cairo_move_to(cr, this->radius * cs, this->radius * si);
            cairo_rel_line_to(cr, tick * cs, tick * si);
        }
    }
    cairo_stroke(cr);
    // END: marks and numbers around semicircle

    cairo_restore(cr);
}

void Setsquare::drawVerticalMarks(cairo_t* cr) const {
    cairo_save(cr);

    const auto max = this->maxVmark / 10;  // number of full centimeters

    // BEGIN: VERTICAL marks within semicircle
    clipVerticalStripes(cr);

    // draw vertical marks
    for (double i = .5; i <= max; i += .5) {
        const double x = cathete(this->radius - .25, i);
        cairo_move_to(cr, -x, i);
        cairo_line_to(cr, x, i);
        cairo_stroke(cr);
    }
    cairo_reset_clip(cr);
    // END: VERTICAL marks within circle


    // BEGIN: vertical measuring marks with numbers
    for (int i = this->minVmark; i <= this->maxVmark; i++) {
        const double y = static_cast<double>(i) / 10.;
        const double tick = (i % 5 == 0) ? TICK_LARGE : TICK_SMALL;

        // marks and numbers (left and right)
        for (const double sign: {-1., 1.}) {
            cairo_move_to(cr, sign * this->horPosVmarks, y);
            cairo_rel_line_to(cr, -sign * tick, .0);
            if (i % 10 == 0) {
                const auto text = std::to_string(i / 10);
                cairo_rel_move_to(cr, -sign * TICK_SMALL, .0);
                showTextCenteredAndRotated(cr, text, .0);
            };
        }
    }
    cairo_stroke(cr);
    // END: vertical measuring marks with numbers

    cairo_restore(cr);
}

void Setsquare::drawHorizontalMarks(cairo_t* cr) const {
    cairo_save(cr);

    const auto max = this->maxVmark / 10;  // number of full centimeters

    // BEGIN: line indicating horizontal 0
    for (auto i = 1; i <= max; i++) {
        cairo_move_to(cr, .0, static_cast<double>(i) - ZERO_MARK_TICK / 2.);
        cairo_rel_line_to(cr, .0, ZERO_MARK_TICK);
    }
    cairo_stroke(cr);
    // END: line indicating horizontal 0

    // BEGIN: measuring marks on top
    for (int i = -this->maxHmark; i <= this->maxHmark; i++) {
        const double tick = (i % 5 == 0) ? TICK_LARGE : TICK_SMALL;
        // draw marks
        cairo_move_to(cr, static_cast<double>(i) / 10., .0);
        cairo_rel_line_to(cr, .0, tick);
        if (i % 10 == 0) {
            // draw numbers
            cairo_rel_move_to(cr, .0, FONT_SIZE / 2.);
            const auto text = std::to_string(std::abs(i / 10));
            showTextCenteredAndRotated(cr, text.c_str(), .0);
        }
    }
    cairo_stroke(cr);
    // END: measuring marks on top

    cairo_restore(cr);
}

void Setsquare::showTextCenteredAndRotated(cairo_t* cr, std::string text, double angle) const {
    cairo_save(cr);

    cairo_text_extents_t te;
    cairo_text_extents(cr, text.c_str(), &te);
    const double dx = te.x_bearing + te.width / 2.0;
    const double dy = te.y_bearing + te.height / 2.0;

    cairo_rotate(cr, rad(angle));
    cairo_rel_move_to(cr, -dx, -dy);
    cairo_text_path(cr, text.c_str());

    cairo_restore(cr);
}

void Setsquare::clipVerticalStripes(cairo_t* cr) const {
    // y-coordinate of the highest vertical mark
    const auto y = static_cast<double>(this->maxVmark) / 10.;

    cairo_rectangle(cr, -this->horPosVmarks - .25, .0, .75, y + .5);          // left stripe
    cairo_rectangle(cr, this->horPosVmarks - .5, .0, .75, y + .5);            // right stripe
    cairo_rectangle(cr, -.25, .0, .5, y + .5);                                // middle stripe
    cairo_rectangle(cr, -this->height, .0, 2. * this->height, this->height);  // clip to the outside
    cairo_clip(cr);
}

void Setsquare::clipHorizontalStripes(cairo_t* cr) const {
    for (auto i = .5; i <= static_cast<double>(this->maxVmark) / 10.0; i += .5) {
        double x = cathete(this->radius - .25, i);
        cairo_rectangle(cr, -x, i - .15, 2. * x, .3);
    }
    cairo_rectangle(cr, -this->height, .0, 2. * this->height, this->height);  // clip to the outside
    cairo_clip(cr);
}
