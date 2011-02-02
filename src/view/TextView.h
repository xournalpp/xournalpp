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
	static void calcSize(Text * t, double & width, double & height);
	static void drawText(cairo_t *cr, Text * t);
	static GList * findText(Text * t, const char * text);
	static PangoLayout * initPango(cairo_t *cr, Text * t);
};

#endif /* __TEXTVIEW_H__ */
