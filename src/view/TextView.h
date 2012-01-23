/*
 * Xournal++
 *
 * Displays a text Element
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __TEXTVIEW_H__
#define __TEXTVIEW_H__

#include <gtk/gtk.h>

class Text;

class TextView {
private:
	TextView();
	virtual ~TextView();

public:
	/**
	 * Calculates the size of a Text model
	 */
	static void calcSize(Text * t, double & width, double & height);

	/**
	 * Draws a Text modle to a cairo surface
	 */
	static void drawText(cairo_t * cr, Text * t);

	/**
	 * Searches text within a Text model, returns XojPopplerRectangle, have to been freed
	 */
	static GList * findText(Text * t, const char * text);

	/**
	 * Initialize a Pango layout
	 */
	static PangoLayout * initPango(cairo_t * cr, Text * t);

	/**
	 * Sets the font name from Text model
	 */
	static void updatePangoFont(PangoLayout * layout, Text * t);
};

#endif /* __TEXTVIEW_H__ */
