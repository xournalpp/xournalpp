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

	virtual int getPrefferedWidth() = 0;
	virtual int getPrefferedHeight() = 0;

private:
	XOJ_TYPE_ATTRIB;

protected:
	GtkAdjustment* adjHorizontal = NULL;
	GtkAdjustment* adjVertical = NULL;

	GtkWidget* xournal = NULL;
	Layout* layout = NULL;
};
