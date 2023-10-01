/*
 * Xournal++
 *
 * A stroke on the document
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>  // for size_t
#include <memory>   // for unique_ptr
#include <vector>   // for vector

#include "path/Path.h"

#include "AudioElement.h"  // for AudioElement
#include "LineStyle.h"     // for LineStyle
#include "Point.h"         // for Point

class Element;
class ObjectInputStream;
class ObjectOutputStream;
class Range;
class ShapeContainer;

class ErasableStroke;
struct PaddedBox;

template <class T, size_t N>
class SmallVector;
using IntersectionParametersContainer = SmallVector<Path::Parameter, 4>;

class SplineSegment;

class StrokeTool {
public:
    enum Value { PEN, ERASER, HIGHLIGHTER };
    StrokeTool(Value v): value(v) {}

    [[nodiscard]] bool isPressureSensitive() const { return value == PEN; }
    [[nodiscard]] bool hasLineStyle() const { return value == PEN; }
    operator const Value&() const { return value; }
    operator Value&() { return value; }

private:
    Value value = PEN;
};

enum StrokeCapStyle {
    ROUND = 0,
    BUTT = 1,
    SQUARE = 2
};  // Must match the indices in StrokeView::CAIRO_LINE_CAP
    // and in EraserHandler::PADDING_COEFFICIENT_CAP

class Stroke: public AudioElement {
public:
    Stroke();
    Stroke(Stroke const&) = default;
    Stroke(Stroke&&) = default;

    Stroke& operator=(Stroke const&) = default;
    Stroke& operator=(Stroke&&) = default;
    ~Stroke() override;

public:
    Stroke* cloneStroke() const;
    Element* clone() const override;

    /**
     * Clone style attributes, but not the data (position, width etc.)
     */
    void applyStyleFrom(const Stroke* other);

    void setWidth(double width);
    double getWidth() const;

    /**
     * Option to fill the shape:
     *  -1: The shape is not filled
     * 255: The shape is fully opaque filled
     * ...
     *   1: The shape is nearly fully transparent filled
     */
    int getFill() const;

    /**
     * Option to fill the shape:
     *  -1: The shape is not filled
     * 255: The shape is fully opaque filled
     * ...
     *   1: The shape is nearly fully transparent filled
     */
    void setFill(int fill);

    void freeUnusedPointItems();

    void setToolType(StrokeTool type);
    StrokeTool getToolType() const;

    const LineStyle& getLineStyle() const;
    void setLineStyle(const LineStyle& style);

    double isPointNearby(double x, double y, double veryClose, double toFar) const override;

    /**
     * @brief Find the parameters corresponding to the points where the stroke crosses in or out of the given box with
     * its padding. Intersections are ignored if the stroke does not hit the small box itself.
     * @param box The padded box
     * @return The parameters (sorted). The size of the returned vector is always even.
     *
     * Warning: this function does not test if the box intersects the stroke's bounding box.
     * For optimization purposes, this test should be performed beforehand by the caller.
     */
    Path::IntersectionParametersContainer intersectWithPaddedBox(const PaddedBox& box) const;

    void move(double dx, double dy) override;
    void scale(double x0, double y0, double fx, double fy, double rotation, bool restoreLineWidth) override;
    void rotate(double x0, double y0, double th) override;

    bool isInSelection(ShapeContainer* container) const override;

    ErasableStroke* getErasable() const;
    void setErasable(ErasableStroke* erasable);

    StrokeCapStyle getStrokeCapStyle() const;
    void setStrokeCapStyle(const StrokeCapStyle capStyle);

    [[maybe_unused]] void debugPrint() const;

public:
    bool hasPressure() const;
    void setPressureSensitive(bool b);

    /**
     * @brief If the stroke is pressure-sensitive, get the path's pressure values in a vector.
     * Otherwise, get an empty vector.
     */
    std::vector<double> getPressureValues() const;

    /**
     * @brief If the stroke is pressure-sensitive, set the path's pressure values to the given values.
     * Otherwise, do nothing.
     */
    void setPressureValues(const std::vector<double>& pressures);

public:
    // Serialize interface
    void serialize(ObjectOutputStream& out) const override;
    void readSerialized(ObjectInputStream& in) override;

    bool rescaleWithMirror() override;

protected:
    void calcSize() const override;

private:
    // The stroke width cannot be inherited from Element
    double width = 0;
    StrokeTool toolType = StrokeTool::PEN;

    /**
     * Dashed line
     */
    LineStyle lineStyle;

    ErasableStroke* erasable = nullptr;

    /**
     * Option to fill the shape:
     *  -1: The shape is not filled
     * 255: The shape is fully opaque filled
     * ...
     *   1: The shape is nearly fully transparent filled
     */
    int fill = -1;

    StrokeCapStyle capStyle = StrokeCapStyle::ROUND;

private:
    /**
     * @brief Pointer to the path the stroke follows
     */
    std::shared_ptr<Path> path;

    /**
     * @brief Cache of sampled spline points for drawing pressure-sensitive splines
     * Empty for other types of strokes.
     */
    mutable std::vector<Point> pointCache{};

    /**
     * @brief Flag: is the stroke pressure-sensitive?
     */
    bool pressureSensitive = false;

public:
    const std::vector<Point>& getPointsToDraw() const;

    void resizePointCache(size_t const);
    void clearPointCache();
    void addToPointCache(const SplineSegment& seg);
    size_t getCacheSize() const;

    const Path& getPath() const;
    bool hasPath() const;

    /**
     * @brief Replace the stroke's path by the one provided
     * @param p New path for the stroke
     * @param snappingBox (optional) Precomputed snapping box of the new points (i.e. the smallest Range containing all
     * the points, stroke width or pressure values not being considered). The snappingBox parameter avoids a
     * recomputation of the bounding boxes if the new points have no pressure values.
     */
    void setPath(std::shared_ptr<Path> p, const Range* const snappingBox = nullptr);

    std::shared_ptr<const Path> getPathPointer() const;

    void unsetSizeCalculated();

    /**
     * @brief Approximate the points using Schneider's algorithm
     */
    void splineFromPLPath();

    friend class ErasableStroke;
    friend class ErasablePressureSpline;
};
