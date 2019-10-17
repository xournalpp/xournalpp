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

class BackgroundSelectDialogBase;

class BaseElementView
{
public:
	BaseElementView(int id, BackgroundSelectDialogBase* dlg);
	virtual ~BaseElementView();

public:
	GtkWidget* getWidget();
	int getWidth();
	int getHeight();

	/**
	 * Select / unselect this entry
	 */
	void setSelected(bool selected);

	/**
	 * Repaint this widget
	 */
	void repaint();

protected:
	/**
	 * Apply the size to the Widget
	 */
	void updateSize();

	/**
	 * Paint the whole widget
	 */
	void paint(cairo_t* cr);

	/**
	 * Paint the contents (without border / selection)
	 */
	virtual void paintContents(cairo_t* cr) = 0;

	/**
	 * Get the width in pixel, without shadow / border
	 */
	virtual int getContentWidth() = 0;

	/**
	 * Get the height in pixel, without shadow / border
	 */
	virtual int getContentHeight() = 0;

	/**
	 * Will be called before getContentWidth() / getContentHeight(), can be overwritten
	 */
	virtual void calcSize();

private:
	static gboolean drawCallback(GtkWidget* widget, cairo_t* cr, BaseElementView* element);
	static gboolean mouseButtonPressCallback(GtkWidget* widget, GdkEventButton* event, BaseElementView* element);

private:
	protected:
	BackgroundSelectDialogBase* dlg;

private:
	/**
	 * Element ID, starting with 0
	 */
	int id = -1;

	bool selected = false;

	GtkWidget* widget = nullptr;
	cairo_surface_t* crBuffer = nullptr;
};
