/*
 * Xournal++
 *
 * Toolbar drag & drop helper class
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __TOOLBARADAPTER_H__
#define __TOOLBARADAPTER_H__

#include "ToolbarDragDropHelper.h"
#include "ToolItemDragCurrentData.h"
#include "../../toolbarMenubar/AbstractToolItem.h"
#include "../../toolbarMenubar/ToolMenuHandler.h"
#include "../../MainWindow.h"
#include "../../toolbarMenubar/model/ToolbarData.h"

#include "../../../util/Util.h"
#include "../../ToolitemDragDrop.h"
#include "../../widgets/SelectColor.h"
#include "../../toolbarMenubar/ColorToolItem.h"
#include "../../../control/Control.h"

class ToolbarAdapter
{
public:
	ToolbarAdapter(GtkWidget* toolbar, String toolbarName,
	               ToolMenuHandler* toolHandler, MainWindow* window);

	~ToolbarAdapter();

private:

	void cleanupToolbars();
	void prepareToolItems();
	void cleanToolItem(GtkToolItem* it);
	void prepareToolItem(GtkToolItem* it);
	void showToolbar();

	/**
	 * Drag a Toolitem from toolbar
	 */
	static void toolitemDragBegin(GtkWidget* widget, GdkDragContext* context,
	                              void* unused);

	/**
	 * Drag a Toolitem from toolbar STOPPED
	 */
	static void toolitemDragEnd(GtkWidget* widget, GdkDragContext* context,
	                            void* unused);

	/**
	 * Remove a toolbar item from the tool where it was
	 */
	void removeFromToolbar(AbstractToolItem* item, String toolbarName, int id);

	static void toolitemDragDataGet(GtkWidget* widget, GdkDragContext* context,
	                                GtkSelectionData* selection_data,
	                                guint info, guint time, ToolbarAdapter* adapter);

	/**
	 * A tool item was dragged to the toolbar
	 */
	static bool toolbarDragMotionCb(GtkToolbar* toolbar, GdkDragContext* context,
	                                gint x, gint y, guint time,
	                                ToolbarAdapter* adapter);

	static void toolbarDragLeafeCb(GtkToolbar* toolbar, GdkDragContext* context,
	                               guint time, ToolbarAdapter* adapter);

	static void toolbarDragDataReceivedCb(GtkToolbar* toolbar,
	                                      GdkDragContext* context,
	                                      gint x, gint y,
	                                      GtkSelectionData* data,
	                                      guint info, guint time,
	                                      ToolbarAdapter* adapter);

private:
	XOJ_TYPE_ATTRIB;

	GtkWidget* w;
	String toolbarName;
	MainWindow* window;

	GtkToolItem* spacerItem;
	ToolMenuHandler* toolHandler;
};

#endif /* __TOOLBARADAPTER_H__ */
