/*
 * Xournal++
 *
 * A point of a stroke
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once


namespace xoj::util {
template <class T>
class Rectangle;
}  // namespace xoj::util

/**
 * @class Point
 * @brief Representation of a point.
 */
class Point {
public:
    Point() = default;
    ~Point() = default;
    Point(const Point& p) = default;

    Point& operator=(Point const&) = default;
    inline bool operator==(const Point& p) const { return x == p.x && y == p.y && z == p.z; }

    /**
     * @brief Point from two values.
     * @param x X value of the point.
     * @param y Y value of the point.
     */
    Point(double x, double y);

    /**
     * @brief Point from three values.
     * @param x X value of the point.
     * @param y Y value of the point.
     * @param z Z value of the point. This denotes the pressure sensitivity.
     */
    Point(double x, double y, double z);

public:
    /**
     * @brief Compute the distance to another point.
     * @param p The other point.
     * @return The Euclidean distance to the other point.
     */
    double lineLengthTo(const Point& p) const;

    /**
     * @brief Compute new Point in the direction from this to another Point.
     * @param p The other Point.
     * @param length The line length or vector length.
     * @return The new Point.
     */
    Point lineTo(const Point& p, double length) const;

    /**
     * @brief Compute new Point in the direction from this to another Point.
     * @param p The other Point.
     * @param ratio The line length over the distance between this and the other Point p
     * @return The new Point.
     */
    Point relativeLineTo(const Point& p, double ratio) const;

    /**
     * @brief Compare if this Point has the same position as another Point.
     * @param p The other Point.
     * @return True if the coordinates are equal. False otherwise.
     */
    bool equalsPos(const Point& p) const;

    /**
     * @brief Test if the point lies (strictly) inside a given rectangle
     * @param rect The rectangle
     * @return True if the point is in rectangle, false otherwise
     */
    bool isInside(const xoj::util::Rectangle<double>& rect) const;

public:
    /**
     * @brief Private storage for x coordinate.
     */
    double x = 0;

    /**
     * @brief Private storage for y coordinate.
     */
    double y = 0;

    /**
     * @brief Private storage for pressure.
     */
    double z = NO_PRESSURE;

    static constexpr double NO_PRESSURE = -1;
};
