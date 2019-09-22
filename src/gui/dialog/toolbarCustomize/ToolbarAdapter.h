/*
 * Xournal++
 *
 * Toolbar drag & drop helper class
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once


#include "gui/MainWindow.h"
#include "gui/toolbarMenubar/ToolMenuHandler.h"

class ToolbarAdapter
{
public:
	ToolbarAdapter(GtkWidget* toolbar, string toolbarName, ToolMenuHandler* toolHandler, MainWindow* window);
	~ToolbarAdapter();

private:

	void cleanupToolbars();
	void prepareToolItems();
	void cleanToolItem(GtkToolItem* it);
	void prepareToolItem(GtkToolItem* it);
	void showToolbar();

private:

	/**
	 * Drag a Toolitem from toolbar
	 */
	static void toolitemDragBegin(GtkWidget* widget, GdkDragContext* context, void* unused);

	/**
	 * Drag a Toolitem from toolbar STOPPED
	 */
	static void toolitemDragEnd(GtkWidget* widget, GdkDragContext* context, void* unused);

	/**
	 * Remove a toolbar item from the tool where it was
	 */
	void removeFromToolbar(AbstractToolItem* item, string toolbarName, int id);
	static void toolitemDragDataGet(GtkWidget* widget, GdkDragContext* context, GtkSelectionData* selection_data,
									guint info, guint time, ToolbarAdapter* adapter);

	/**
	 * A tool item was dragged to the toolbar
	 */
	static bool toolbarDragMotionCb(GtkToolbar* toolbar, GdkDragContext* context,
									gint x, gint y, guint time, ToolbarAdapter* adapter);
	static void toolbarDragLeafeCb(GtkToolbar* toolbar, GdkDragContext* context, guint time, ToolbarAdapter* adapter);
	static void toolbarDragDataReceivedCb(GtkToolbar* toolbar, GdkDragContext* context, gint x, gint y,
										  GtkSelectionData* data, guint info, guint time, ToolbarAdapter* adapter);

private:
	GtkWidget* w;
	string toolbarName;
	MainWindow* window;

	GtkToolItem* spacerItem;
	ToolMenuHandler* toolHandler;
};
