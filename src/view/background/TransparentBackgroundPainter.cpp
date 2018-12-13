#include "TransparentBackgroundPainter.h"

#include <Util.h>

TransparentBackgroundPainter::TransparentBackgroundPainter()
{
	XOJ_INIT_TYPE(TransparentBackgroundPainter);

}

TransparentBackgroundPainter::~TransparentBackgroundPainter()
{
	XOJ_CHECK_TYPE(TransparentBackgroundPainter);

	XOJ_RELEASE_TYPE(TransparentBackgroundPainter);
}

void TransparentBackgroundPainter::resetConfig()
{
	XOJ_CHECK_TYPE(TransparentBackgroundPainter);

	this->foregroundColor1 = 0xBDBDBD;
	this->lineWidth = 1.5;
	this->drawRaster1 = 14.17;
}

void TransparentBackgroundPainter::paint()
{
	XOJ_CHECK_TYPE(TransparentBackgroundPainter);

	//paintBackgroundColor();
	//paintBackgroundDotted();
}

void TransparentBackgroundPainter::paintBackgroundTransparent()
{
	XOJ_CHECK_TYPE(TransparentBackgroundPainter);

}
