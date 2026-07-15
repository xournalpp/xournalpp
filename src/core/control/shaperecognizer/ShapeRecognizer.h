/*
 * Xournal++
 *
 * Xournal Shape recognizer
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <array>  // for array
#include <memory>
#include <vector>

#include "RecoSegment.h"
#include "ShapeRecognizerConfig.h"  // for MAX_POLYGON_SIDES

class Stroke;
class Point;
class Inertia;
struct RecoSegment;

struct Corner {
    int index;    ///< The index of the point within the stroke's point array.
    double dmax;  ///< The maximum perpendicular distance (error) found at this vertex.
};

class ShapeRecognizer {
public:
    ShapeRecognizer();
    virtual ~ShapeRecognizer();

    auto recognizePatterns(Stroke* stroke, double strokeMinSize) -> std::unique_ptr<Stroke>;
    void resetRecognizer();

private:
    auto tryTriangle() -> std::unique_ptr<Stroke>;
    auto tryRectangle() -> std::unique_ptr<Stroke>;

    /**
     * @brief Calculates a dynamic epsilon for the RDP algorithm based on the stroke's dimensions.
     * To prevent thin shapes (like rectangles with high aspect ratios) from being
     * collapsed into single lines, this function uses the shortest side of the
     * bounding box to determine the simplification threshold.
     *
     * @param pt Array of points in the stroke.
     * @param last_idx The index of the last point in the array.
     * @return A double representing the epsilon value, with a minimum floor of 1.0.
     */
    double calculateDynamicEpsilon(const Point* pt, int last_idx);

    // function Stroke* tryArrow(); removed after commit a3f7a251282dcfea8b4de695f28ce52bf2035da2
    /**
     * @brief Calculates the perpendicular distance from point p to the line segment ab.
     * @param p The point to measure.
     * @param a Start point of the segment.
     * @param b End point of the segment.
     * @return The perpendicular distance. If a and b coincide, returns Euclidean distance to a.
     */
    static double perpendicularDistance(const Point& p, const Point& a, const Point& b);

    static void optimizePolygonal(const Point* pt, int nsides, int* breaks, Inertia* ss);

    /**
     * @brief Identifies shape corners using the Ramer-Douglas-Peucker (RDP) algorithm.
     * This function recursively finds points with the highest perpendicular distance
     * from the line connecting start and finish, marking them as corners if they exceed epsilon.
     * @param pt Pointer to the array of points.
     * @param start Starting index of the segment to analyze.
     * @param finish Ending index of the segment to analyze.
     * @param epsilon The error tolerance for corner detection.
     * @param cornerList Reference to the vector where identified corners will be stored.
     */
    void findPolygonalRDP(const Point* pt, int start, int finish, double epsilon, std::vector<Corner>& cornerList);

    /**
     * @brief Reduces the number of identified vertices until a maximum of 4 internal corners remain.
     * Iteratively removes the vertices with the lowest perpendicular distance error
     * to ensure the resulting polygon is a rectangle or triangle.
     * @param cornerList The list of vertices to be pruned.
     */
    void reduceVerticesToFour(std::vector<Corner>& cornerList);

    static bool isStrokeLargeEnough(Stroke* stroke, double strokeMinSize);

private:
    std::array<RecoSegment, MAX_POLYGON_SIDES + 1> queue{};
    int queueLength;

    Stroke* stroke;
};
