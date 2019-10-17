/*
 * Xournal++
 *
 * Scroll handling for different scroll implementations
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

#include <gtk/gtk.h>

class Layout;

class ScrollHandling
{
public:
	ScrollHandling(GtkAdjustment* adjHorizontal, GtkAdjustment* adjVertical);
	virtual ~ScrollHandling();

public:
	GtkAdjustment* getHorizontal();
	GtkAdjustment* getVertical();

	virtual void init(GtkWidget* xournal, Layout* layout);

	virtual void setLayoutSize(int width, int height) = 0;

	virtual int getPreferredWidth() = 0;
	virtual int getPreferredHeight() = 0;

	virtual void translate(cairo_t* cr, double& x1, double& x2, double& y1, double& y2) = 0;
	virtual void translate(double& x, double& y) = 0;

	virtual bool fullRepaint() = 0;

	virtual void scrollChanged() = 0;

private:
	protected:
	GtkAdjustment* adjHorizontal = nullptr;
	GtkAdjustment* adjVertical = nullptr;

	GtkWidget* xournal = nullptr;
	Layout* layout = nullptr;
};
