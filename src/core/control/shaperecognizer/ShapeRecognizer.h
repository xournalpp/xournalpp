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

#include <array>   // for array
#include <memory>  // for shared_ptr

#include "RecoSegment.h"
#include "ShapeRecognizerConfig.h"  // for MAX_POLYGON_SIDES

class Path;
class PiecewiseLinearPath;
class Point;
class Inertia;
struct RecoSegment;

class ShapeRecognizer {
public:
    ShapeRecognizer(const PiecewiseLinearPath& path);
    ~ShapeRecognizer() = default;

    std::shared_ptr<Path> recognizePatterns(double strokeMinSize);
    void resetRecognizer();

private:
    std::shared_ptr<PiecewiseLinearPath> tryRectangle();
    // function Stroke* tryArrow(); removed after commit a3f7a251282dcfea8b4de695f28ce52bf2035da2

    static void optimizePolygonal(const Point* pt, int nsides, int* breaks, Inertia* ss);

    int findPolygonal(const Point* pt, int start, int end, int nsides, int* breaks, Inertia* ss);

    static bool isPathLargeEnough(const Path& path, double strokeMinSize);

private:
    std::array<RecoSegment, MAX_POLYGON_SIDES + 1> queue{};
    int queueLength = 0;

    const PiecewiseLinearPath& path;
};
