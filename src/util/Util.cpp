#include "Util.h"
#include <assert.h>

GdkColor Util::intToGdkColor(int c) {
	GdkColor color = { 0, 0, 0, 0 };
	color.red = (c >> 8) & 0xff00;
	color.green = (c >> 0) & 0xff00;
	color.blue = (c << 8) & 0xff00;
	return color;
}

int Util::gdkColorToInt(const GdkColor & c) {
	return (c.red >> 8) << 16 | (c.green >> 8) << 8 | (c.blue >> 8);
}

void Util::cairo_set_source_rgbi(cairo_t *cr, int c) {
	double r = ((c >> 16) & 0xff) / 255.0;
	double g = ((c >> 8) & 0xff) / 255.0;
	double b = (c & 0xff) / 255.0;

	cairo_set_source_rgb(cr, r, g, b);
}

DebugObject::DebugObject() {
	this->d1 = 0xffff0000;
	this->d2 = 465456;
	this->d3 = 89535395;
}

void DebugObject::debugTestIsOk() {
	assert(!(this->d1 != 0xffff0000 || this->d2 != 465456 || this->d3 != 89535395));
}

