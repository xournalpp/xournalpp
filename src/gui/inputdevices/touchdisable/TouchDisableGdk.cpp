//
// Created by ulrich on 22.04.19.
//

#include <gdk/gdk.h>
#include <gui/XournalView.h>
#include "TouchDisableGdk.h"

TouchDisableGdk::TouchDisableGdk(GtkWidget* widget) : TouchDisableInterface(), widget(widget)
{
}

TouchDisableGdk::~TouchDisableGdk()
{
}

void TouchDisableGdk::init()
{
}

void TouchDisableGdk::enableTouch()
{
#ifdef DEBUG_INPUT
	g_message("Enable touch");
#endif

	gtk_grab_remove(this->widget);
	gdk_pointer_ungrab(GDK_CURRENT_TIME);
}

void TouchDisableGdk::disableTouch()
{
#ifdef DEBUG_INPUT
	g_message("Disable touch using GDK grabs");
#endif
	GdkWindow* window = gtk_widget_get_window(this->widget);

	/**
	 * See the following link if window dragging by double clicks on empty widget space occurs
	 * https://www.reddit.com/r/kde/comments/aaeo91
	 */
	gdk_pointer_grab(window, false, GDK_TOUCH_MASK, nullptr, nullptr, GDK_CURRENT_TIME);
	gtk_grab_add(this->widget);
}

