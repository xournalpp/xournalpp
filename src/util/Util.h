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
#include <String.h>

class Util {
private:
	Util();
	virtual ~Util();

public:
	static GdkColor intToGdkColor(int c);
	static int gdkColorToInt(const GdkColor & c);

	static void cairo_set_source_rgbi(cairo_t *cr, int color);

	static String getAutosaveFilename();

	static int getPid();

	static void fakeExposeWidget(GtkWidget * widget, GdkPixmap * pixmap);
	static GdkPixbuf * newPixbufFromWidget(GtkWidget * widget);

private:
	static String getSettingsSubfolder(String subfolder);
};

#endif /* __UTIL_H__ */
