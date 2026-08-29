#include <bit>  // for bit_cast

#include <cairo.h>
#include <gtest/gtest.h>

#include "util/Matrix.h"

TEST(UtilMatrix, testCairo) {
    xoj::util::Matrix m{1, 2, 3, -5, {6, -2}};
    cairo_matrix_t n = std::bit_cast<cairo_matrix_t>(m);

    auto expectEq = [](const xoj::util::Matrix& m, const cairo_matrix_t& n) {
        EXPECT_DOUBLE_EQ(m.xx, n.xx);
        EXPECT_DOUBLE_EQ(m.yx, n.yx);
        EXPECT_DOUBLE_EQ(m.xy, n.xy);
        EXPECT_DOUBLE_EQ(m.yy, n.yy);
        EXPECT_DOUBLE_EQ(m.shift.x, n.x0);
        EXPECT_DOUBLE_EQ(m.shift.y, n.y0);
    };

    expectEq(m, n);

    cairo_matrix_translate(&n, 2, -2);
    m = m.translate(2, -2);
    expectEq(m, n);

    cairo_matrix_scale(&n, .5, -1);
    m = m.scale(.5, -1);
    expectEq(m, n);

    cairo_matrix_invert(&n);
    m = m.inverse();
    expectEq(m, n);

    cairo_matrix_rotate(&n, 1);
    m = m.rotate(1);
    expectEq(m, n);

    xoj::util::Point<double> p{3, -5};
    auto q = p;
    p = m * p;
    cairo_matrix_transform_point(&n, &q.x, &q.y);
    EXPECT_DOUBLE_EQ(p.x, q.x);
    EXPECT_DOUBLE_EQ(p.y, q.y);
}
