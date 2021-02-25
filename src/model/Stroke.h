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

#include "AudioElement.h"
#include "Element.h"
#include "LineStyle.h"
#include "PiecewiseLinearPath.h"
#include "Point.h"
#include "Spline.h"

enum StrokeTool { STROKE_TOOL_PEN, STROKE_TOOL_ERASER, STROKE_TOOL_HIGHLIGHTER };

class EraseableStroke;
class Path;

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
    Element* clone() override;

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

    bool intersects(double x, double y, double halfEraserSize) override;
    bool intersects(double x, double y, double halfEraserSize, double* gap) override;

    void move(double dx, double dy) override;
    void scale(double x0, double y0, double fx, double fy, double rotation, bool restoreLineWidth) override;
    void rotate(double x0, double y0, double th) override;

    bool isInSelection(ShapeContainer* container) override;

    EraseableStroke* getEraseable();
    void setEraseable(EraseableStroke* eraseable);

    [[maybe_unused]] void debugPrint();

public:
    // Deprecated use of Stroke::points
    [[deprecated("Use class Path or its descendants")]] void addPoint(const Point& p);
    //     [[deprecated("Use class Path or its descendants")]] void setFirstPoint(double x, double y);
    //     [[deprecated("Use class Path or its descendants")]] void setLastPoint(double x, double y);
    //     [[deprecated("Use class Path or its descendants")]] void setLastPoint(const Point& p);

    // Used in StrokeRecognizer (and satellite classes), LoadHandler, SaveHandler, EditSelectionContents,
    // SizeUndoAction, DocumentView
    [[deprecated("Use class Path or its descendants")]] int getPointCount() const;

    // Used in StrokeRecognizer (and satellite classes) and StrokeView
    [[deprecated("Use class Path or its descendants")]] std::vector<Point> const& getPointVector() const;

    // Used in SaveHandler and SizeUndoAction
    [[deprecated("Use class Path or its descendants")]] Point getPoint(int index) const;

    // Used only in StrokeRecognizer (and satellite classes)
    [[deprecated("Use class Path or its descendants")]] const Point* getPoints() const;

    //     [[deprecated("Use class Path or its descendants")]] void deletePoint(int index);
    //     [[deprecated("Use class Path or its descendants")]] void deletePointsFrom(int index);

    // Deprecated handling of pressure
    [[deprecated("Use class Path or its descendants")]] void setPressure(const vector<double>& pressure);

    [[deprecated("Use class Path or its descendants")]] void setSecondToLastPressure(double pressure);

    //     [[deprecated("Use class Path or its descendants")]] void setLastPressure(double pressure);
    //     [[deprecated("Use class Path or its descendants")]] void clearPressure();
    [[deprecated("Use class Path or its descendants")]] void scalePressure(double factor);

    [[deprecated("Use class Path or its descendants")]] double getAvgPressure() const;

    bool hasPressure() const;
    void setPressureSensitive(bool b);

public:
    // Serialize interface
    void serialize(ObjectOutputStream& out) override;
    void readSerialized(ObjectInputStream& in) override;

    bool rescaleWithMirror() override;

protected:
    void calcSize() const override;

private:
    // The stroke width cannot be inherited from Element
    double width = 0;

    StrokeTool toolType = STROKE_TOOL_PEN;

    // The array with the points
    //     std::vector<Point> points{};

    /**
     * Dashed line
     */
    LineStyle lineStyle;

    EraseableStroke* eraseable = nullptr;

    /**
     * Option to fill the shape:
     *  -1: The shape is not filled
     * 255: The shape is fully opaque filled
     * ...
     *   1: The shape is nearly fully transparent filled
     */
    int fill = -1;

private:
    /**
     * @brief Pointer to the path the stroke follows
     */
    std::shared_ptr<Path> path;

    /**
     * @brief Cache of sampled spline points for drawing pressure-sensitive splines
     * Empty for other types of strokes.
     */
    std::vector<Point> pointCache{};

    /**
     * @brief Flag: is the stroke pressure sensitive?
     */
    bool pressureSensitive = false;

    /**
     * @brief Store the parameters of the intersection points found by a call to intersects()
     * Avoid recomputing those intersection points when erasing...
     */
    std::vector<Path::Parameter> intersectionParameters{};

public:
    bool isSpline() const { return path && path->getType() != Path::PIECEWISE_LINEAR; }

    //     const Spline& getSpline() const { return spline; }
    const Spline& getSpline() const { return *dynamic_cast<Spline*>(path.get()); }

    const std::vector<Point>& getPointsToDraw();

    const Path& getPath() { return *(path.get()); }

    void setPath(std::shared_ptr<Path> p);

    void unsetSizeCalculated();

    /**
     * @brief Approximate the points using Schneider's algorithm
     */
    void splineFromPLPath();

    //     SomeType splitAtSingularPoints();

    friend class EraseableStroke;
    friend class EraseablePressureSpline;

    /**
     * @brief Attorney class to allow some stroke handlers to directly set the stroke bounding boxes
     * This avoids calling the generic Stroke::calcSize() on geometric shapes whose bounding box is already known.
     */
    class Attorney {
    private:
        static void setBoundingBoxes(Stroke& stroke, const Rectangle<double>& thinBB, const Rectangle<double>& thickBB);

        friend class EllipseHandler;
    };
    friend class Attorney;
};
