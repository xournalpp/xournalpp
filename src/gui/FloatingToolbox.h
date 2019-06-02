/*
 * Xournal++
 *
 * The Floating Toolbox
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once


#include "GladeGui.h"

class MainWindow;

enum FloatingToolBoxState {
		recalcSize = 0,
		configuration,
		noChange
};

/************************
 * FloatingToolbox:
 *  Uses a GtkOverlay to show a widget containing toolbars.
 *  The getOverlayPosition callback is used to position the toolbox widget relative to the users input device.
 *  The ui/main.glade file defines the floatingToolbox as a GtkRevealer widget - attempts to use another widget 
 *  had issues with leave-notify-event.
 * 
 * 
 * 
 * 
 */

class FloatingToolbox 
{
public:
	FloatingToolbox(MainWindow* theMainWindow, GtkOverlay *overlay);
	virtual ~FloatingToolbox();

public:
	/**
	 *  show(x,y): Show Toolbox at centered at x,y relative to main window. 
	 */
	void show(int x, int y); 
	
	/**
	 * showForConfiguration(): Does not hide when pointer leaves so that user can drag and drop.
	 */
	void showForConfiguration();
	
	void hide();
	
	/**
	 * flagRecalculateSizeRequired(): trigger recalc size on next getOverlayPosition. Used when new toolbars loaded.
	 */
	void flagRecalculateSizeRequired();

private:
	/**
	 * Callback for positioning overlayed floating menu
	 */
	static gboolean  getOverlayPosition (GtkOverlay *overlay, GtkWidget *widget, GdkRectangle *allocation, FloatingToolbox* self);
	
	/**
	 * Callback to hide floating Toolbar when mouse leaves it
	 */
	static void handleLeaveFloatingToolbox(GtkWidget * floatingToolbox, GdkEvent  *event,  FloatingToolbox* self);
	
	void show(bool showTitle = false);
	
	/**
	 * check if user has assigned a button to activate, or has put tools in the FloatingToolbox.
	 */
	bool floatingToolboxActivated();

	
private:
	XOJ_TYPE_ATTRIB;

	MainWindow* mainWindow;
	GtkWidget *floatingToolbox;

	/**
	 * Communicating with getOverlayPosition callback
	 * */
	int floatingToolboxX = 0;
	int floatingToolboxY = 0;
	FloatingToolBoxState floatingToolboxState = recalcSize;
	
};
