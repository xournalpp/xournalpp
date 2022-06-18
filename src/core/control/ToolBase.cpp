#include "ToolBase.h"

#include "control/ToolEnums.h"  // for DrawingType, EraserType, ToolSize

ToolBase::ToolBase() = default;

ToolBase::~ToolBase() = default;

/**
 * @return Color of the tool for all drawing tools
 */
auto ToolBase::getColor() const -> Color { return this->color; }

/**
 * @param color Color of the tool for all drawing tools
 */
void ToolBase::setColor(Color color) { this->color = color; }

/**
 * @return Size of a drawing tool
 */
auto ToolBase::getSize() const -> ToolSize { return this->size; }

/**
 * @param size Size of a drawing tool
 */
void ToolBase::setSize(ToolSize size) { this->size = size; }

/**
 * @return Draw special shape
 */
auto ToolBase::getDrawingType() const -> DrawingType { return this->drawingType; }

/**
 * @param drawingType Draw special shape
 */
void ToolBase::setDrawingType(DrawingType drawingType) { this->drawingType = drawingType; }

/**
 * @return Eraser Type
 */
auto ToolBase::getEraserType() const -> EraserType { return this->eraserType; }

/**
 * @param eraserType type of eraser
 */
void ToolBase::setEraserType(EraserType eraserType) { this->eraserType = eraserType; }

/**
 * @return Fill of the shape is enabled
 */
auto ToolBase::getFill() const -> bool { return this->fill; }

/**
 * @param fill Fill of the shape is enabled
 */
void ToolBase::setFill(bool fill) { this->fill = fill; }

/**
 * @return Alpha for fill
 */
auto ToolBase::getFillAlpha() const -> int { return this->fillAlpha; }

/**
 * @param fillAlpha Alpha for fill
 */
void ToolBase::setFillAlpha(int fillAlpha) { this->fillAlpha = fillAlpha; }

/**
 * @return Style of the line drawing
 */
auto ToolBase::getLineStyle() const -> const LineStyle& { return this->lineStyle; }

/**
 * @param style Style of the line drawing
 */
void ToolBase::setLineStyle(const LineStyle& style) { this->lineStyle = style; }
