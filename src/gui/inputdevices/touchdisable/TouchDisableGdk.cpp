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
	GdkDisplay* display = gdk_display_get_default();
	GdkSeat* displaySeat = gdk_display_get_default_seat(display);
	GdkDevice* pointerDevice = gdk_seat_get_pointer(displaySeat);
	this->touchSeat = gdk_device_get_seat(pointerDevice);
}

void TouchDisableGdk::enableTouch()
{
#ifdef DEBUG_INPUT
	g_message("Enable touch");
#endif

	//gdk_seat_ungrab(this->touchSeat);
	gtk_grab_remove(this->widget);
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

	// Discard all events that are not meant for our view
	gtk_grab_add(this->widget);
	//gdk_seat_grab(this->touchSeat, window, GDK_SEAT_CAPABILITY_TOUCH, true, nullptr, nullptr, nullptr, nullptr);
}

