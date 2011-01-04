/*
 * Xournal Extended
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

class Shadow {
private:
	Shadow() {
	}
	virtual ~Shadow() {
	}

	static void drawShadowEdge(cairo_t *cr, int x, int y, int width, int height, const unsigned char * edge, double r,
			double g, double b);

public:
	static void drawShadow(cairo_t *cr, int x, int y, int width, int height, double r, double g, double b);
	static int getShadowBottomRightSize();
	static int getShadowTopLeftSize();
};

#endif /* __SHADOW_H__ */
