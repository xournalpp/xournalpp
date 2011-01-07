/*
 * Xournal++
 *
 * Xournal util functions
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __UTIL_H__
#define __UTIL_H__

#include <gtk/gtk.h>

class Util {
public:
	static GdkColor intToGdkColor(int c);
	static int gdkColorToInt(const GdkColor & c);

	static void cairo_set_source_rgbi(cairo_t *cr, int color);
};

/**
 * Used for testing memory violations
 */

class DebugObject {
public:
	DebugObject();

	void debugTestIsOk();
private:
	int d1;
	int d2;
	int d3;
};

#endif /* __UTIL_H__ */
