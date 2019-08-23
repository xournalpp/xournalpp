#include "ToolBase.h"

ToolBase::ToolBase()
{
	XOJ_INIT_TYPE(ToolBase);
}

ToolBase::~ToolBase()
{
	XOJ_CHECK_TYPE(ToolBase);
	XOJ_RELEASE_TYPE(ToolBase);
}

/**
 * Apply data from another ToolBase or any extending class
 */
void ToolBase::applyFrom(const ToolBase* t)
{
	XOJ_CHECK_TYPE(ToolBase);

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
	XOJ_CHECK_TYPE(ToolBase);

	return this->color;
}

/**
 * @param color Color of the tool for all drawing tools
 */
void ToolBase::setColor(int color)
{
	XOJ_CHECK_TYPE(ToolBase);

	this->color = color;
}

/**
 * @return Size of a drawing tool
 */
ToolSize ToolBase::getSize() const
{
	XOJ_CHECK_TYPE(ToolBase);

	return this->size;
}

/**
 * @param size Size of a drawing tool
 */
void ToolBase::setSize(ToolSize size)
{
	XOJ_CHECK_TYPE(ToolBase);

	this->size = size;
}

/**
 * @return Draw special shape
 */
DrawingType ToolBase::getDrawingType() const
{
	XOJ_CHECK_TYPE(ToolBase);

	return this->drawingType;
}

/**
 * @param drawingType Draw special shape
 */
void ToolBase::setDrawingType(DrawingType drawingType)
{
	XOJ_CHECK_TYPE(ToolBase);

	this->drawingType = drawingType;
}

/**
 * @return Fill of the shape is enabled
 */
bool ToolBase::getFill() const
{
	XOJ_CHECK_TYPE(ToolBase);

	return this->fill;
}

/**
 * @param fill Fill of the shape is enabled
 */
void ToolBase::setFill(bool fill)
{
	XOJ_CHECK_TYPE(ToolBase);

	this->fill = fill;
}

/**
 * @return Alpha for fill
 */
int ToolBase::getFillAlpha() const
{
	XOJ_CHECK_TYPE(ToolBase);

	return this->fillAlpha;
}

/**
 * @param fillAlpha Alpha for fill
 */
void ToolBase::setFillAlpha(int fillAlpha)
{
	XOJ_CHECK_TYPE(ToolBase);

	this->fillAlpha = fillAlpha;
}

/**
 * @return Style of the line drawing
 */
const LineStyle& ToolBase::getLineStyle() const
{
	XOJ_CHECK_TYPE(ToolBase);

	return this->lineStyle;
}

/**
 * @param style Style of the line drawing
 */
void ToolBase::setLineStyle(const LineStyle& style)
{
	XOJ_CHECK_TYPE(ToolBase);

	this->lineStyle = style;
}
