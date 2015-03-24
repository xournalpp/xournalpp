/*
 * Xournal++
 *
 * Scrollbar adapter
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#ifndef __SCROLLBAR_H__
#define __SCROLLBAR_H__

#include <config.h>

#ifdef ENABLE_OS
// Overlay scrollbar
#include <os/os.h>
#endif

#include <gtk/gtk.h>
#include <XournalType.h>

class Scrollbar;

class ScrollbarListener
{
public:
	virtual void scrolled(Scrollbar* scrollbar) = 0;
};

class Scrollbar
{
public:
	Scrollbar(bool horizontal);
	~Scrollbar();

public:
	int getValue();
	int getMax();

	void setMax(int max);
	void setValue(int value);

	void scroll(int relPos);

	void setPageSize(int size);
	int getPageSize();

	double getWheelDelta(GdkScrollDirection direction);

	void setPageIncrement(int inc);

	void ensureAreaIsVisible(int lower, int upper);

	void addListener(ScrollbarListener* listener);
	void removeScrollbarListener(ScrollbarListener* listener);

	GtkWidget* getWidget();

private:
	static void scrolled(GtkAdjustment* adjustment, Scrollbar* scrollbar);

private:
	XOJ_TYPE_ATTRIB;

	GtkWidget* scrollbar;
	GtkAdjustment* adj;
	GList* listener;

	int value;
};

#endif //__SCROLLBAR_H__

