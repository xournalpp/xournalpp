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
	static gboolean onMouseEnterNotifyEvent(GtkWidget * widget, GdkEventCrossing * event, gpointer user_data);
	static gboolean onMouseLeaveNotifyEvent(GtkWidget * widget, GdkEventCrossing * event, gpointer user_data);
};

#endif /* __XINPUTUTILS_H__ */
