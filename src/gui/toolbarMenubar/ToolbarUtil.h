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
	static GtkWidget* newSepeartorImage();

	/**
	 * Create color for Background / Foreground color indicator / button
	 */
public:
	/**
	 * Create a new GtkImage with preview color
	 */
	static GtkWidget* newColorIcon(int color, int size = 22, bool circle = true);

	/**
	 * Create a new cairo_surface_t* with preview color
	 */
	static cairo_surface_t* newColorIconSurface(int color, int size = 22, bool circle = true);

	/**
	 * Create a new GdkPixbuf* with preview color
	 */
	static GdkPixbuf* newColorIconPixbuf(int color, int size = 22, bool circle = true);
};
