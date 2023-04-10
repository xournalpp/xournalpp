/*
 * Xournal++
 *
 * Snapping methods
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include "Point.h"

namespace Snapping {

/**
 * @brief If a value is near enough to the y-coordinate of a grid point, it returns the nearest y-coordinate of the
 * grid point. Otherwise the original value itself.
 * @param y the value
 * @param gridSize the distance to each snapping point
 * @param tolerance the tolerance as a fraction of a half grid diagonal (assumed to be between 0 and 1)
 */
[[nodiscard]] double snapVertically(double y, double gridSize, double tolerance);

/**
 * @brief If a value is near enough to the x-coordinate of a grid point, it returns the nearest x-coordinate of the
 * grid point. Otherwise the original value itself.
 * @param x the value
 * @param gridSize the distance to each snapping point
 * @param tolerance the tolerance as a fraction of a half grid diagonal (assumed to be between 0 and 1)
 */
[[nodiscard]] double snapHorizontally(double x, double gridSize, double tolerance);

/**
 * @brief If a points distance to the nearest grid point is under a certain tolerance, it returns the nearest
 * grid point. Otherwise the original Point itself.
 * @param pos the position
 * @param gridSize the distance to each snapping point
 * @param tolerance the tolerance as a fraction of a half grid diagonal (assumed to be between 0 and 1)
 */
[[nodiscard]] Point snapToGrid(Point const& pos, double gridSize, double tolerance);

/**
 * @brief if the angles distance to a multiple quarter of PI is under a certain tolerance, it returns the latter.
 * Otherwise the original angle.
 * @param radian the angle (in radian)
 * @param tolerance the tolerance as a fraction of M_PI/8 (assumed to be between 0 and 1)
 */
[[nodiscard]] double snapAngle(double radian, double tolerance);

/**
 * @brief Snaps the angle between the horizontal axis and the line between the given point and center
 * and returns the point rotated accordingly
 * @param pos the coordinate of the point
 * @param center the center of rotation
 * @param tolerance the tolerance as a fraction of M_PI/8 (assumed to be between 0 and 1)
 */
[[nodiscard]] Point snapRotation(Point const& pos, Point const& center, double tolerance);

/**
 * @brief For a given point computes the nearest point on a straight line specified by two points lying on it
 * @param pos the given point
 * @param first the first point of the straight line
 * @param second the second point of the straight line
 */
[[nodiscard]] Point projToLine(Point const& pos, Point const& first, Point const& second);

/**
 * @brief Computes the distance from a given point to a line sgement specified by its end points
 * @param pos the given point
 * @param first the first end point of the line segment
 * @param second the second end point of the line segment
 */
[[nodiscard]] double distanceLine(Point const& pos, Point const& first, Point const& second);

}  // namespace Snapping
