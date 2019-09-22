#include "ToolBase.h"

ToolBase::ToolBase()
{
}

ToolBase::~ToolBase()
{
}

/**
 * Apply data from another ToolBase or any extending class
 */
void ToolBase::applyFrom(const ToolBase* t)
{
	this->color = t->color;
	this->size = t->size;
	this->drawingType = t->drawingType;
	this->fill = t->fill;
	this->fillAlpha = t->fillAlpha;
	this->lineStyle = t->lineStyle;
}

/**
 * @return Color of the tool for all drawing tools
 */
int ToolBase::getColor() const
{
	return this->color;
}

/**
 * @param color Color of the tool for all drawing tools
 */
void ToolBase::setColor(int color)
{
	this->color = color;
}

/**
 * @return Size of a drawing tool
 */
ToolSize ToolBase::getSize() const
{
	return this->size;
}

/**
 * @param size Size of a drawing tool
 */
void ToolBase::setSize(ToolSize size)
{
	this->size = size;
}

/**
 * @return Draw special shape
 */
DrawingType ToolBase::getDrawingType() const
{
	return this->drawingType;
}

/**
 * @param drawingType Draw special shape
 */
void ToolBase::setDrawingType(DrawingType drawingType)
{
	this->drawingType = drawingType;
}

/**
 * @return Fill of the shape is enabled
 */
bool ToolBase::getFill() const
{
	return this->fill;
}

/**
 * @param fill Fill of the shape is enabled
 */
void ToolBase::setFill(bool fill)
{
	this->fill = fill;
}

/**
 * @return Alpha for fill
 */
int ToolBase::getFillAlpha() const
{
	return this->fillAlpha;
}

/**
 * @param fillAlpha Alpha for fill
 */
void ToolBase::setFillAlpha(int fillAlpha)
{
	this->fillAlpha = fillAlpha;
}

/**
 * @return Style of the line drawing
 */
const LineStyle& ToolBase::getLineStyle() const
{
	return this->lineStyle;
}

/**
 * @param style Style of the line drawing
 */
void ToolBase::setLineStyle(const LineStyle& style)
{
	this->lineStyle = style;
}

