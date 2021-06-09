#include "Path.h"

#include <cmath>
#include <memory>

#include "Point.h"

Path& Path::operator=(const std::vector<Point>& vector) {
    data = vector;
    return *this;
}

const std::vector<Point>& Path::getData() const { return data; }

const Point& Path::getLastKnot() const { return data.back(); }

const Point& Path::getFirstKnot() const { return data.front(); }

std::vector<Path::Parameter> Path::intersectWithRectangle(const Rectangle<double>& rectangle) const {
    return this->intersectWithRectangle(rectangle, 0, this->nbSegments() - 1);
}

bool Path::empty() const { return data.empty(); }

void Path::clear() { data.clear(); }

void Path::freeUnusedPointItems() { data = {data.begin(), data.end()}; }

void Path::move(double dx, double dy) {
    for (Point& p: data) {
        p.x += dx;
        p.y += dy;
    }
}

cairo_matrix_t Path::rotate(double x0, double y0, double th) {
    cairo_matrix_t rotMatrix;
    cairo_matrix_init_identity(&rotMatrix);
    cairo_matrix_translate(&rotMatrix, x0, y0);
    cairo_matrix_rotate(&rotMatrix, th);
    cairo_matrix_translate(&rotMatrix, -x0, -y0);

    for (auto&& p: data) { cairo_matrix_transform_point(&rotMatrix, &p.x, &p.y); }

    return rotMatrix;
}

std::pair<cairo_matrix_t, double> Path::scale(double x0, double y0, double fx, double fy, double rotation,
                                              bool restoreLineWidth) {
    std::pair<cairo_matrix_t, double> result;
    cairo_matrix_t* scaleMatrix = &(result.first);
    cairo_matrix_init_identity(scaleMatrix);
    cairo_matrix_translate(scaleMatrix, x0, y0);
    cairo_matrix_rotate(scaleMatrix, rotation);
    cairo_matrix_scale(scaleMatrix, fx, fy);
    cairo_matrix_rotate(scaleMatrix, -rotation);
    cairo_matrix_translate(scaleMatrix, -x0, -y0);

    if (restoreLineWidth) {
        for (auto&& p: data) { cairo_matrix_transform_point(scaleMatrix, &p.x, &p.y); }
        result.second = 1.0;
    } else {
        double fz = std::sqrt(std::abs(fx * fy));
        for (auto&& p: data) {
            cairo_matrix_transform_point(scaleMatrix, &p.x, &p.y);
            p.z *= fz;
        }
        result.second = fz;
    }
    return result;
}

bool Path::isPointOnBoundary(const Point& p, const Rectangle<double>& r) {
    return ((p.x == r.x || p.x == r.x + r.width) && p.y >= r.y && p.y <= r.y + r.height) ||
           ((p.y == r.y || p.y == r.y + r.height) && p.x >= r.x && p.x <= r.x + r.width);
}
