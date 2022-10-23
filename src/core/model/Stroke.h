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

#include "AudioElement.h"  // for AudioElement
#include "LineStyle.h"     // for LineStyle
#include "Point.h"         // for Point

class Element;
class ObjectInputStream;
class ObjectOutputStream;
class ShapeContainer;

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

class ErasableStroke;
struct PaddedBox;
struct PathParameter;
template <class T, size_t N>
class SmallVector;

using IntersectionParametersContainer = SmallVector<PathParameter, 4>;

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
     * @brief Create a partial clone whose points are those of parameters between lowerBound and upperBound
     * Assumes both lowerBound and upperBound are valid parameters of the stroke, and lowerBound <= upperBound
     */
    std::unique_ptr<Stroke> cloneSection(const PathParameter& lowerBound, const PathParameter& upperBound) const;

    /**
     * @brief Create a partial clone of a closed stroke (i.e. points.front() == points.back()) with points
     *     getPoint(startParam) -- ... -- points.back() == points.front() -- ... -- getPoint(endParam)
     * Assumes both startParam and endParam are valid parameters of the stroke, and endParam.index < startParam.index
     */
    std::unique_ptr<Stroke> cloneCircularSectionOfClosedStroke(const PathParameter& startParam,
                                                               const PathParameter& endParam) const;

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

    void addPoint(const Point& p);
    void setLastPoint(double x, double y);
    void setFirstPoint(double x, double y);
    void setLastPoint(const Point& p);
    int getPointCount() const;
    void freeUnusedPointItems();
    std::vector<Point> const& getPointVector() const;
    Point getPoint(int index) const;
    Point getPoint(PathParameter parameter) const;
    const Point* getPoints() const;

    /**
     * @brief Replace the stroke's points by the ones in the provided vector (they will be copied).
     * @param other New vector of points for the stroke
     * @param snappingBox (optional) Precomputed snapping box of the new points (i.e. the smallest Range containing all
     * the points, stroke width or pressure values not being considered). The snappingBox parameter avoids a
     * recomputation of the bounding boxes if the new points have no pressure values.
     */
    void setPointVector(const std::vector<Point>& other, const Range* const snappingBox = nullptr);
    void setPointVector(std::vector<Point>&& other, const Range* const snappingBox = nullptr);

private:
    void setPointVectorInternal(const Range* const snappingBox);

public:
    void deletePoint(int index);
    void deletePointsFrom(size_t index);

    void setToolType(StrokeTool type);
    StrokeTool getToolType() const;

    const LineStyle& getLineStyle() const;
    void setLineStyle(const LineStyle& style);

    bool intersects(double x, double y, double halfEraserSize) const override;
    bool intersects(double x, double y, double halfEraserSize, double* gap) const override;

    /**
     * @brief Find the parameters within a certain interval corresponding to the points where the stroke crosses in
     * or out of the given box with its padding. Intersections are ignored if the stroke does not hit the small box
     * itself.
     * @param box The padded box
     * @param firstIndex The lower bound of the interval
     * @param lastIndex The upper bound of the interval
     * @return The parameters (sorted). The size of the returned vector is always even.
     *
     * Warning: this function does not test if the box intersects the stroke's bounding box.
     * For optimization purposes, this test should be performed beforehand by the caller.
     */
    IntersectionParametersContainer intersectWithPaddedBox(const PaddedBox& box, size_t firstIndex,
                                                           size_t lastIndex) const;

    IntersectionParametersContainer intersectWithPaddedBox(const PaddedBox& box) const;

    void setPressure(const std::vector<double>& pressure);
    void setLastPressure(double pressure);
    void setSecondToLastPressure(double pressure);
    void clearPressure();
    void scalePressure(double factor);

    bool hasPressure() const;
    double getAvgPressure() const;

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

    // The array with the points
    std::vector<Point> points{};

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
};
