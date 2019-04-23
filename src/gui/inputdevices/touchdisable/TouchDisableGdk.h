/*
 * Xournal++
 *
 * [Header description]
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once


#include "TouchDisableInterface.h"
#include "gtk/gtk.h"

class TouchDisableGdk : public TouchDisableInterface
{
public:
	explicit TouchDisableGdk(GtkWidget* widget);
	~TouchDisableGdk() override;
public:
	void enableTouch() override;
	void disableTouch() override;
	void init() override;
	static bool eventCallback(GtkWidget* widget, GdkEvent* event, TouchDisableGdk* self);
private:
	GtkWidget* widget;
};


