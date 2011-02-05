#include "Shadow.h"
#include "ShadowCode.c"

int Shadow::getShadowBottomRightSize() {
	return shadowBottomRightSize;
}

int Shadow::getShadowTopLeftSize() {
	return shadowTopLeftSize;
}

void Shadow::drawShadowEdge(cairo_t *cr, int x, int y, int width, int height, const unsigned char * edge, double r,
		double g, double b) {

	int w = width << 1;
	int h = height << 1;

	for (int u = 0; u < h; u++) {
		for (int i = 0; i < w; i++) {
			double a = edge[u * w + i] / 255.0;
			cairo_set_source_rgba(cr, r, g, b, a);

			cairo_rectangle(cr, x + i - width, y + u - height, 1, 1);
			cairo_fill(cr);
		}
	}
}

typedef const unsigned char * u8ptr;

void Shadow::drawShadow(cairo_t *cr, int x, int y, int width, int height, double r, double g, double b) {

	double a;

	// draw shadow on top and left
	for (int i = 0; i < shadowTopLeftSize; i++) {
		a = shadowTopLeft[i] / 255.0;
		cairo_set_source_rgba(cr, r, g, b, a);

		// top
		cairo_rectangle(cr, x + shadowTopLeftSize, y + i - shadowTopLeftSize, width - shadowTopLeftSize
				- shadowBottomRightSize + 1, 1);

		// left
		cairo_rectangle(cr, x + i - shadowTopLeftSize, y + shadowTopLeftSize, 1, height - shadowTopLeftSize
				- shadowBottomRightSize + 1);
		cairo_fill(cr);
	}

	// draw shadow on bottom and right
	for (int i = 0; i < shadowBottomRightSize; i++) {
		a = shadowBottomRight[i] / 255.0;
		cairo_set_source_rgba(cr, r, g, b, a);

		// bottom
		cairo_rectangle(cr, x + shadowTopLeftSize, y + height + i, width - shadowTopLeftSize - shadowBottomRightSize
				+ 1, 1);

		// right
		cairo_rectangle(cr, x + width + i, y + shadowTopLeftSize, 1, height - shadowTopLeftSize - shadowBottomRightSize
				+ 1);
		cairo_fill(cr);
	}

	const int sBrSize = shadowBottomRightSize;
	const int sTlSize = shadowTopLeftSize;
	drawShadowEdge(cr, x, y, sTlSize, sTlSize, (u8ptr) shadowEdge1, r, g, b);
	drawShadowEdge(cr, x + width + 1, y, sBrSize, sTlSize, (u8ptr) shadowEdge2, r, g, b);
	drawShadowEdge(cr, x + width + 1, y + height + 1, sBrSize, sBrSize, (u8ptr) shadowEdge3, r, g, b);
	drawShadowEdge(cr, x, y + height + 1, shadowTopLeftSize, sBrSize, (u8ptr) shadowEdge4, r, g, b);
}
