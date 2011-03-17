/*
 * Xournal++
 *
 * XInput util functions
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __XINPUTUTILS_H__
#define __XINPUTUTILS_H__

#include <gtk/gtk.h>

class XInputUtils {
private:
	XInputUtils();
	virtual ~XInputUtils();

public:
	static void fixXInputCoords(GdkEvent * event, GtkWidget * widget);
	static void handleScrollEvent(GdkEventButton * event, GtkWidget * widget);

	/**
	 * Avoid crash if e.g. a mouse is plugged out...
	 */
	static gboolean onMouseEnterNotifyEvent(GtkWidget * widget, GdkEventCrossing * event);
	static gboolean onMouseLeaveNotifyEvent(GtkWidget * widget, GdkEventCrossing * event);

	static void initUtils(GtkWidget * win);

private:
	static int screenWidth;
	static int screenHeight;

};

#endif /* __XINPUTUTILS_H__ */
