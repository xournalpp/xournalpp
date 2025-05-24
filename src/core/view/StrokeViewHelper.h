/*
 * Xournal++
 *
 * Draw stroke
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <vector>

#include <cairo.h>

class LineStyle;
class Path;
class Point;

namespace xoj::view::StrokeViewHelper {

/**
 * @brief No pressure sensitivity, one line is drawn, with given width and line style (dashes)
 */
void drawNoPressure(cairo_t* cr, const Path& path, const double strokeWidth, const LineStyle& lineStyle,
                    double dashOffset = 0);

/**
 * @brief Draw a stroke with pressure, for this multiple lines with different widths needs to be drawn.
 * @return New dash offset, if one wants to keep on drawing the same stroke.
 *      Effectively, the return value equals dashOffset + length of the path.
 */
double drawWithPressure(cairo_t* cr, const std::vector<Point>& pts, const LineStyle& lineStyle, double dashOffset = 0);
};  // namespace xoj::view::StrokeViewHelper
