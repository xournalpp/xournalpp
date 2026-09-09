#pragma once
#include <cassert>

template <class Matrix>
auto constexpr multiply(Matrix const& lhs, Matrix const& rhs) -> Matrix {
    return {lhs.a * rhs.a + lhs.b * rhs.c,
            lhs.a * rhs.b + lhs.b * rhs.d,
            lhs.c * rhs.a + lhs.d * rhs.c,
            lhs.c * rhs.b + lhs.d * rhs.d,
            lhs.a * rhs.tx + lhs.b * rhs.ty + lhs.tx,
            lhs.c * rhs.tx + lhs.d * rhs.ty + lhs.ty};
}

template <class Matrix>
auto constexpr scale(double sx, double sy, Matrix const& rhs) -> Matrix {
    return {sx * rhs.a, sx * rhs.b, sy * rhs.c, sy * rhs.d, sx * rhs.tx, sy * rhs.ty};
}

template <class Matrix>
auto constexpr translate(double tx, double ty, Matrix const& rhs) -> Matrix {
    return {rhs.a, rhs.b, rhs.c, rhs.d, tx + rhs.tx, ty + rhs.ty};
}

template <class Matrix>
auto constexpr inverse(Matrix const& self) -> Matrix {
    auto det = self.a * self.d - self.b * self.c;  // 3
    assert(det != 0);
    return {self.d / det,
            (-self.b) / det,
            (-self.c) / det,
            self.a / det,
            (self.c * self.ty - self.d * self.tx) / det,
            (self.b * self.tx - self.a * self.ty) / det};
}
