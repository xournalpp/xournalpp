/*
 * Xournal++
 *
 * Part of the Xournal shape recognizer
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once
#include <config-debug.h>
#include <glib.h>

#define MAX_POLYGON_SIDES 4

#define LINE_POINT_DIST2_THRESHOLD 15                // threshold of maximum distance from point to line
#define SEGMENT_MAX_DET 0.045                        // maximum score for a polygon segment (ideal line = 0)
#define LINE_MAX_DET 0.015                           // maximum score for single line, stricter than for polygons
#define CIRCLE_MIN_DET 0.95                          // minimum det. score for circle (ideal circle = 1)
#define CIRCLE_MAX_SCORE 0.10                        // max circle score for circle (ideal circle = 0)
#define SLANT_TOLERANCE (5 * M_PI / 180)             // ignore slanting by +/- 5 degrees
#define TRIANGLE_LINEAR_TOLERANCE 0.3                // vertex gap tolerance in triangles
#define RECTANGLE_ANGLE_TOLERANCE (15 * M_PI / 180)  // angle tolerance in rectangles
#define RECTANGLE_LINEAR_TOLERANCE 0.20              // vertex gap tolerance in rectangles
#define POLYGON_LINEAR_TOLERANCE 0.20                // vertex gap tolerance in closed polygons


#ifdef DEBUG_RECOGNIZER
#define RDEBUG(msg, ...) g_message("ShapeReco::" msg, ##__VA_ARGS__)
#else
#define RDEBUG(msg, ...)
#endif
