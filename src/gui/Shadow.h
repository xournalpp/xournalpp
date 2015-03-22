/*
 * Xournal++
 *
 * Settings Dialog
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __SHADOW_H__
#define __SHADOW_H__

#include <gtk/gtk.h>

class Shadow
{
private:
	Shadow();
	virtual ~Shadow();

	void drawShadowEdge(cairo_t* cr, int x, int y, int width, int height,
						const unsigned char* edge, double r, double g, double b);
	void drawShadowImpl(cairo_t* cr, int x, int y, int width, int height);

	void paintEdge(cairo_t* cr, cairo_surface_t* image, int x, int y, int width,
				int height);

	void drawShadowTop(cairo_t* cr, int x, int y, int width, double r, double g,
					double b);
	void drawShadowLeft(cairo_t* cr, int x, int y, int height, double r, double g,
						double b);
	void drawShadowRight(cairo_t* cr, int x, int y, int height, double r, double g,
						double b);
	void drawShadowBottom(cairo_t* cr, int x, int y, int width, double r, double g,
						double b);

public:
	/**
	 * This is the public interface of this class
	 */
	static void drawShadow(cairo_t* cr, int x, int y, int width, int height);
	static int getShadowBottomRightSize();
	static int getShadowTopLeftSize();

private:
	static Shadow* instance;

	cairo_surface_t* edgeTopLeft;
	cairo_surface_t* edgeTopRight;
	cairo_surface_t* edgeBottomLeft;
	cairo_surface_t* edgeBottomRight;

	cairo_surface_t* top;
	cairo_surface_t* right;
	cairo_surface_t* bottom;
	cairo_surface_t* left;
};

#endif /* __SHADOW_H__ */
