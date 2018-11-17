/*
 * Xournal++
 *
 * Repaint helper class
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once


#include <XournalType.h>
#include <Rectangle.h>

#include <gtk/gtk.h>
#include <list>

class RepaintWidgetHandler
{
public:
	RepaintWidgetHandler(GtkWidget * width);
	~RepaintWidgetHandler();

public:
	void repaintComplete();
	void repaintRects(Rectangle* rect);

private:
	static bool idleRepaint(RepaintWidgetHandler * data);
	void addRepaintCallback();

private:
	XOJ_TYPE_ATTRIB;

	GMutex mutex;

	int rescaleId;

	bool complete;
	std::list<Rectangle*> rects;
	GtkWidget* widget;
};
