/*
 * Xournal++
 *
 * Utility functions used for Toolbar implementation
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#include <gtk/gtk.h>

class ToolbarUtil {
private:
	ToolbarUtil();
	virtual ~ToolbarUtil();

public:
	static void fakeExposeWidget(GtkWidget* widget, GdkPixmap* pixmap);
	static GdkPixbuf* newPixbufFromWidget(GtkWidget* widget, int iconSize = 24);
	static GtkWidget* newSepeartorImage();
};
