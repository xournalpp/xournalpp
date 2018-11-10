/*
 * Xournal++
 *
 * Base class for Background selection dialog entry
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

#include <gtk/gtk.h>

class BaseElementView
{
public:
	BaseElementView();
	virtual ~BaseElementView();

public:
	GtkWidget* getWidget();
	int getWidth();
	int getHeight();

protected:
	void updateSize();
	void paint(cairo_t* cr);

private:
	static gboolean drawCallback(GtkWidget* widget, cairo_t* cr, BaseElementView* element);

private:
	XOJ_TYPE_ATTRIB;

	bool selected;

	GtkWidget* widget;
	cairo_surface_t* crBuffer;
};
