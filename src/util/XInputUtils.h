/*
 * Xournal++
 *
 * XInput util functions
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <config-debug.h>

#include <gtk/gtk.h>

#ifdef DEBUG_INPUT
#define INPUTDBG(msg, ...) printf("INPUT:: " msg, __VA_ARGS__); printf(" on %s:%i\n",  __FILE__, __LINE__)
#define INPUTDBG2(msg)     printf("INPUT:: " msg " on %s:%i\n",  __FILE__, __LINE__)
#else
#define INPUTDBG(msg, ...)
#define INPUTDBG2(msg)
#endif

class XInputUtils
{
private:
	XInputUtils();
	virtual ~XInputUtils();

public:
	static void fixXInputCoords(GdkEvent* event, GtkWidget* widget);
	static void handleScrollEvent(GdkEventButton* event, GtkWidget* widget);

	/**
	 * Avoid crash if e.g. a mouse is plugged out...
	 */
	static gboolean onMouseEnterNotifyEvent(GtkWidget* widget, GdkEventCrossing* event);
	static gboolean onMouseLeaveNotifyEvent(GtkWidget* widget, GdkEventCrossing* event);

	static void initUtils(GtkWidget* win);

	static void setLeafEnterWorkaroundEnabled(bool enabled);

private:
	static int screenWidth;
	static int screenHeight;

	static int enableLeafEnterWorkaround;

};
