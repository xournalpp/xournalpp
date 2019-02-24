/*
 * Xournal++
 *
 * Icon for color buttons
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>
#include <gtk/gtk.h>

class ColorSelectImage
{
public:
	ColorSelectImage(int color, bool circle);
	virtual ~ColorSelectImage();

public:
	/**
	 * @return The widget which is drawed
	 */
	GtkWidget* getWidget();

	/**
	 * Color of the icon
	 */
	void setColor(int color);

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

private:
	/**
	 * Draw the widget
	 */
	void drawWidget(cairo_t* cr);

	/**
	 * Draw the widget
	 */
	static void drawWidget(cairo_t* cr, int color, int size, int y, bool circle);

private:
	XOJ_TYPE_ATTRIB;

	/**
	 * The widget which is drawed
	 */
	GtkWidget* widget = NULL;

	/**
	 * Color of the icon
	 */
	int color = 0;

	/**
	 * Size of the icon
	 */
	int size = 16;

	/**
	 * Draw as circle
	 */
	bool circle = true;
};
