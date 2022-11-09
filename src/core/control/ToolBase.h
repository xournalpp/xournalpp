/*
 * Xournal++
 *
 * Base class for a tool, which contains the tool configuration,
 * this is also used for other classes, e.g. to store current tool configuration
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/LineStyle.h"  // for LineStyle
#include "util/Color.h"       // for Color

#include "ToolEnums.h"  // for DrawingType, EraserType, ToolSize, DRAW...


class ToolBase {
public:
    ToolBase();
    virtual ~ToolBase();

private:
    void operator=(const ToolBase& t);
    ToolBase(const ToolBase& t);

public:
    /**
     * @return Color of the tool for all drawing tools
     */
    Color getColor() const;

    /**
     * @param color Color of the tool for all drawing tools
     */
    void setColor(Color color);

    /**
     * @return Size of a drawing tool
     */
    ToolSize getSize() const;

    /**
     * @param size Size of a drawing tool
     */
    void setSize(ToolSize size);

    /**
     * @return Draw special shape
     */
    DrawingType getDrawingType() const;

    /**
     * @param drawingType Draw special shape
     */
    void setDrawingType(DrawingType drawingType);

    /**
     * @brief Get the Eraser Type object
     *
     * @return EraserType
     */
    EraserType getEraserType() const;

    /**
     * @brief Set the Eraser Type object
     *
     * @param eraserType
     */
    void setEraserType(EraserType eraserType);

    /**
     * @return Fill of the shape is enabled
     */
    bool getFill() const;

    /**
     * @param fill Fill of the shape is enabled
     */
    void setFill(bool fill);

    /**
     * @return Alpha for fill
     */
    int getFillAlpha() const;

    /**
     * @param fillAlpha Alpha for fill
     */
    void setFillAlpha(int fillAlpha);

    /**
     * @return Style of the line drawing
     */
    const LineStyle& getLineStyle() const;

    /**
     * @param style Style of the line drawing
     */
    void setLineStyle(const LineStyle& style);

private:
    /**
     * Color of the tool for all drawing tools
     */
    Color color = Colors::black;

    /**
     * Size of a drawing tool
     */
    ToolSize size = TOOL_SIZE_MEDIUM;

    /**
     * Draw special shape
     */
    DrawingType drawingType = DRAWING_TYPE_DEFAULT;

    /**
     * Eraser type
     */
    EraserType eraserType = ERASER_TYPE_DEFAULT;

    /**
     * Fill of the shape is enabled
     */
    bool fill = false;

    /**
     * Alpha for fill
     */
    int fillAlpha = 128;
    ;

    /**
     * Style of the line drawing
     */
    LineStyle lineStyle;
};
