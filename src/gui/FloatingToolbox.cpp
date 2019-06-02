#include "FloatingToolbox.h"
#include "MainWindow.h"
#include "control/Control.h"
#include "GladeGui.h"
#include <gdk/gdk.h>
#include "control/settings/ButtonConfig.h"
#include "gui/ToolbarDefinitions.h"


FloatingToolbox::FloatingToolbox(MainWindow* theMainWindow, GtkOverlay* overlay)
{
	XOJ_INIT_TYPE(FloatingToolbox);

	this->mainWindow = theMainWindow;
	this->floatingToolbox = theMainWindow->get("floatingToolbox");
	this->floatingToolboxX = 200;
	this->floatingToolboxY = 200;
	this->floatingToolboxState = recalcSize;

	gtk_overlay_add_overlay(overlay, this->floatingToolbox);
	gtk_overlay_set_overlay_pass_through(overlay, this->floatingToolbox, TRUE);
	gtk_widget_add_events(this->floatingToolbox, GDK_LEAVE_NOTIFY_MASK);
	g_signal_connect(this->floatingToolbox, "leave-notify-event", G_CALLBACK(handleLeaveFloatingToolbox), this);
	//position overlay widgets
	g_signal_connect(theMainWindow->get("mainOverlay"), "get-child-position", G_CALLBACK(this->getOverlayPosition), this);
}


FloatingToolbox::~FloatingToolbox()
{
	XOJ_CHECK_TYPE(FloatingToolbox);

	XOJ_RELEASE_TYPE(FloatingToolbox);
}


void FloatingToolbox::show(int x, int y)
{
	XOJ_CHECK_TYPE(FloatingToolbox);

	this->floatingToolboxX = x;
	this->floatingToolboxY = y;
	this->show();
}


/****
 * floatingToolboxActivated
 *  True if the user has:
 *    assigned a mouse or stylus button to bring up the floatingToolbox;
 *    or enabled tapAction and Show FloatingToolbox( prefs->DrawingArea->ActionOnToolTap );
 *    or put tools in the FloatingToolbox.
 *
 */
bool FloatingToolbox::floatingToolboxActivated()
{
	Settings* settings = this->mainWindow->getControl()->getSettings();
	bool show = false;
	ButtonConfig* cfg = nullptr;

	//check if any buttons assigned to bring up toolbox
	for (int id = 0; id < BUTTON_COUNT; id++)
	{
		cfg = settings->getButtonConfig(id);

		if (cfg->getAction() == TOOL_FLOATING_TOOLBOX)
		{
			return true;													// return TRUE
		}
	}

	//check if user can show Floating Menu with tap.
	if (settings->getDoActionOnStrokeFiltered() && settings->getStrokeFilterEnabled())
	{
		return true;													// return TRUE
	}

	//check for tools in toolbox:
	for (int index = 0; index < FLOATINGTOOLBOX_TOOLBARS_LEN; index++)
	{
		const char* guiName = FLOATINGTOOLBOX_TOOLBARS[index].guiName;
		GtkToolbar* toolbar1 = GTK_TOOLBAR(this->mainWindow->get(guiName));
		int num = gtk_toolbar_get_n_items(toolbar1);

		if (num > 0)
		{
			return true;													// return TRUE
		}
	}

	return false;
}


void FloatingToolbox::showForConfiguration()
{
	XOJ_CHECK_TYPE(FloatingToolbox);

	if (this->floatingToolboxActivated())		// Do not show if not being used - at least while experimental.
	{
		GtkWidget* overlay = this->mainWindow->get("mainOverlay");
		GtkWidget* label = this->mainWindow->get("labelFloatingToolbox");
		GtkWidget* boxContents = this->mainWindow->get("boxContents");
		gint wx, wy;
		gtk_widget_translate_coordinates(boxContents, gtk_widget_get_toplevel(boxContents), 0, 0, &wx, &wy);
		this->floatingToolboxX = wx + 40;	//when configuration state these are
		this->floatingToolboxY = wy + 40;	// topleft coordinates( otherwise center).
		this->floatingToolboxState = configuration;
		this->show(true);
	}
}


void FloatingToolbox::show(bool showTitle)
{
	XOJ_CHECK_TYPE(FloatingToolbox);

	gtk_widget_hide(this->floatingToolbox);		//force showing in new position
	gtk_widget_show_all(this->floatingToolbox);

	if (!showTitle)
	{
		gtk_widget_hide(this->mainWindow->get("labelFloatingToolbox"));
	}
}


void FloatingToolbox::hide()
{
	XOJ_CHECK_TYPE(FloatingToolbox);

	if (this->floatingToolboxState == configuration)
	{
		this->floatingToolboxState = recalcSize;
	}

	gtk_widget_hide(this->floatingToolbox);
}


void FloatingToolbox::flagRecalculateSizeRequired()
{
	this->floatingToolboxState = recalcSize;
}


/**
 * getOverlayPosition - this is how we position the widget in the overlay under the mouse
 *
 * The requested location is communicated via the FloatingToolbox member variables:
 * ->floatingToolbox,		so we can operate on the right widget
 * ->floatingToolboxState,	are we configuring, resizing or just moving
 * ->floatingToolboxX,		where to display
 * ->floatingToolboxY.
 *
 */
gboolean  FloatingToolbox::getOverlayPosition(GtkOverlay*   overlay,
        GtkWidget*    widget,
        GdkRectangle* allocation,
        FloatingToolbox* self)
{
	XOJ_CHECK_TYPE_OBJ(self, FloatingToolbox);

	if (widget == self->floatingToolbox)
	{
		gtk_widget_get_allocation(widget, allocation);	//get existing width and height

		if (self->floatingToolboxState != noChange ||  allocation->height < 2)	//if recalcSize or configuration or  initiation.
		{
			GtkRequisition natural;
			gtk_widget_get_preferred_size(widget,  NULL,  &natural);
			allocation->width = natural.width;
			allocation->height = natural.height;
		}

		switch(self->floatingToolboxState)
		{
			case recalcSize:	//fallthrough 		note: recalc done above
			case noChange:
				// show centered on x,y
				allocation->x = self->floatingToolboxX - allocation->width / 2;
				allocation->y = self->floatingToolboxY - allocation->height / 2;
				self->floatingToolboxState = noChange;
				break;

			case configuration:
				allocation->x = self->floatingToolboxX;
				allocation->y = self->floatingToolboxY;
				allocation->width = std::max(allocation->width + 32, 50); //always room for one more...
				allocation->height  = std::max(allocation->height, 50);
				break;
		}

		return true;
	}

	return false;
}


void FloatingToolbox::handleLeaveFloatingToolbox(GtkWidget* floatingToolbox, GdkEvent*  event,  FloatingToolbox* self)
{
	XOJ_CHECK_TYPE_OBJ(self, FloatingToolbox);

	if (floatingToolbox == self->floatingToolbox)
	{
		if (self->floatingToolboxState !=  configuration)
		{
			self->hide();
		}
	}
}

