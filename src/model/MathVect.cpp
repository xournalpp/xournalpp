#include "MathVect.h"

#include <cmath>

MathVect3::MathVect3(const Point& p, const Point& q): dx(q.x - p.x), dy(q.y - p.y), dz(q.z - p.z) {}
MathVect3::MathVect3(double dx, double dy, double dz): dx(dx), dy(dy), dz(dz) {}

MathVect3 MathVect3::operator+(const MathVect3& u) const { return {dx + u.dx, dy + u.dy, dz + u.dz}; }
MathVect3 MathVect3::operator-(const MathVect3& u) const { return {dx - u.dx, dy - u.dy, dz - u.dz}; }
MathVect3 MathVect3::operator-() const { return {-dx, -dy, -dz}; }

void MathVect3::operator*=(const double d) {
    dx *= d;
    dy *= d;
    dz *= d;
}
void MathVect3::operator/=(const double d) {
    dx /= d;
    dy /= d;
    dz /= d;
}

double MathVect3::norm() const { return sqrt(dx * dx + dy * dy + dz * dz); }

void MathVect3::normalize() {
    double initNorm = norm();
    if (initNorm >= EPSILON) {
        dx /= initNorm;
        dy /= initNorm;
        dz /= initNorm;
    } else {
        dx = dy = dz = 0.0;
    }
}

Point MathVect3::translatePoint(const Point& p) const { return {dx + p.x, dy + p.y, dz + p.z}; }

double MathVect3::scalarProduct(const MathVect3& u, const MathVect3& v) {
    return u.dx * v.dx + u.dy * v.dy + u.dz * v.dz;
}


MathVect3 operator*(const double c, const MathVect3& u) { return {c * u.dx, c * u.dy, c * u.dz}; }
