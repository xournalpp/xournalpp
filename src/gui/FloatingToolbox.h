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


class FloatingToolbox 
{
public:
	FloatingToolbox(MainWindow* theMainWindow, GtkOverlay *overlay);
	virtual ~FloatingToolbox();

public:
	void show( int x, int y);
	void showForConfiguration();
	void hide( );
	void flagRecalculateSizeRequired();	// trigger recalc size on next getOverlayPosition. Used when new toolbars loaded.

private:
	/**
	 * Callback for positioning overlayed floating menu
	 */
	static gboolean  getOverlayPosition (GtkOverlay *overlay, GtkWidget *widget, GdkRectangle *allocation, FloatingToolbox* self);
	
	/**
	 * Callback to hide floating Toolbar when mouse leaves it
	 */
	static void handleLeaveFloatingToolbox(GtkWidget * floatingToolbox, GdkEvent  *event,  FloatingToolbox* self);
	
	void show( );

	
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
