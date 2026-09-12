#include "util/Matrix.h"

#include <bit>  // for bit_cast
#include <ostream>

#include <cairo.h>

using namespace xoj::util;

auto operator<<(std::ostream& os, Matrix const& m) -> std::ostream& {
    return os << "Matrix(" << m.xx << ", " << m.xy << ", " << m.shift.x << "\n"
              << "       " << m.yx << ", " << m.yy << ", " << m.shift.y << ")";
}

void Matrix::transformCairo(cairo_t* cr) const {
    auto m = std::bit_cast<cairo_matrix_t>(*this);
    cairo_transform(cr, &m);
}

// Static tests

// For compatibility with cairo
static_assert(sizeof(Matrix) == sizeof(cairo_matrix_t));
static_assert(offsetof(Matrix, xx) == offsetof(cairo_matrix_t, xx));
static_assert(offsetof(Matrix, yx) == offsetof(cairo_matrix_t, yx));
static_assert(offsetof(Matrix, xy) == offsetof(cairo_matrix_t, xy));
static_assert(offsetof(Matrix, yy) == offsetof(cairo_matrix_t, yy));
static_assert(offsetof(Matrix, shift.x) == offsetof(cairo_matrix_t, x0));
static_assert(offsetof(Matrix, shift.y) == offsetof(cairo_matrix_t, y0));


static constexpr auto test_ident = Matrix{1, 0, 0, 1, {0, 0}};
static constexpr auto test_trans = Matrix{1, 0, 0, 1, {14, 10}};
static constexpr auto test_scale = Matrix{2, 0, 0, 3, {0, 0}};
static constexpr auto test_rot = Matrix{0, -1, 1, 0, {0, 0}};
static constexpr auto test_all = Matrix{2, -1, 4, 3, {12, 10}};

static_assert(test_ident * test_ident == test_ident);
static_assert(test_ident * test_trans == test_trans);
static_assert(test_trans * test_ident == test_trans);
static_assert(test_rot.inverse() * test_rot * test_all == test_all * test_ident);
static_assert(test_scale == Matrix{}.scale(2, 3));

static constexpr double EPSILON = 1e-10;
static constexpr bool almostEqual(const Matrix& m, const Matrix& n) {
    // todo(cpp23): use std::abs (constexpr)
    return (m.xx - n.xx < EPSILON && n.xx - m.xx < EPSILON) && (m.xy - n.xy < EPSILON && n.xy - m.xy < EPSILON) &&
           (m.yx - n.yx < EPSILON && n.yx - m.yx < EPSILON) && (m.yy - n.yy < EPSILON && n.yy - m.yy < EPSILON) &&
           (m.shift.x - n.shift.x < EPSILON && n.shift.x - m.shift.x < EPSILON) &&
           (m.shift.y - n.shift.y < EPSILON && n.shift.y - m.shift.y < EPSILON);
}
static_assert(almostEqual(test_all.inverse() * test_all, test_ident));
static_assert(almostEqual(test_all* test_all.inverse(), test_ident));

static_assert(test_trans * Point<double>(1, -3) == Point<double>(15, 7));
