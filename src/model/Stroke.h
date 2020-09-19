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
#include "Point.h"

enum StrokeTool { STROKE_TOOL_PEN, STROKE_TOOL_ERASER, STROKE_TOOL_HIGHLIGHTER };

class EraseableStroke;

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

    void addPoint(const Point& p);
    void setLastPoint(double x, double y);
    void setFirstPoint(double x, double y);
    void setLastPoint(const Point& p);
    int getPointCount() const;
    void freeUnusedPointItems();
    std::vector<Point> const& getPointVector() const;
    Point getPoint(int index) const;
    const Point* getPoints() const;

    void deletePoint(int index);
    void deletePointsFrom(int index);

    void setToolType(StrokeTool type);
    StrokeTool getToolType() const;

    const LineStyle& getLineStyle() const;
    void setLineStyle(const LineStyle& style);

    bool intersects(double x, double y, double halfEraserSize) override;
    bool intersects(double x, double y, double halfEraserSize, double* gap) override;

    void setPressure(const vector<double>& pressure);
    void setLastPressure(double pressure);
    void clearPressure();
    void scalePressure(double factor);

    bool hasPressure() const;
    double getAvgPressure() const;

    void move(double dx, double dy) override;
    void scale(double x0, double y0, double fx, double fy, double rotation, bool restoreLineWidth) override;
    void rotate(double x0, double y0, double th) override;

    bool isInSelection(ShapeContainer* container) override;

    EraseableStroke* getEraseable();
    void setEraseable(EraseableStroke* eraseable);

    void debugPrint();

public:
    // Serialize interface
    void serialize(ObjectOutputStream& out) override;
    void readSerialized(ObjectInputStream& in) override;

protected:
    void calcSize() const override;

private:
    // The stroke width cannot be inherited from Element
    double width = 0;

    StrokeTool toolType = STROKE_TOOL_PEN;

    // The array with the points
    std::vector<Point> points{};

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
};
