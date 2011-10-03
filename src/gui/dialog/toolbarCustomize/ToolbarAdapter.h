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

#include "ToolbarListener.h"
#include "ToolbarDragDropHelper.h"
#include "../../toolbarMenubar/AbstractToolItem.h"
#include "../../toolbarMenubar/ToolMenuHandler.h"
#include "../../MainWindow.h"
#include "../../toolbarMenubar/model/ToolbarData.h"

#include "../../../util/Util.h"

#include "../../ToolitemDragDrop.h"

class ToolbarAdapter {
public:
	ToolbarAdapter(GtkWidget * toolbar, String toolbarName, ToolbarListener * listener, ToolMenuHandler * toolHandler,
			MainWindow * window) {
		XOJ_INIT_TYPE( ToolbarAdapter);

		this->w = toolbar;
		this->toolbarName = toolbarName;
		this->listener = listener;
		this->toolHandler = toolHandler;
		this->window = window;

		// prepare drag & drop
		gtk_drag_dest_set(toolbar, GTK_DEST_DEFAULT_ALL, NULL, 0, GDK_ACTION_MOVE);
		ToolbarDragDropHelper::dragDestAddToolbar(toolbar);

		g_signal_connect(toolbar, "drag_motion", G_CALLBACK(toolbarDragMotionCb), this);
		g_signal_connect(toolbar, "drag_leave", G_CALLBACK(toolbarDragLeafeCb), this);
		g_signal_connect(toolbar, "drag_data_received", G_CALLBACK(toolbarDragDataReceivedCb), this);

		showToolbar();

		// TODO: !!! cursor wider löschen
		// gdk_window_set_cursor (gtk_widget_get_window (GTK_WIDGET(item)), NULL);

	}

	~ToolbarAdapter() {
		XOJ_CHECK_TYPE( ToolbarAdapter);

		// remove drag & drop handler
		g_signal_handlers_disconnect_by_func(this->w, (gpointer) toolbarDragMotionCb, this);
		g_signal_handlers_disconnect_by_func(this->w, (gpointer) toolbarDragLeafeCb, this);
		g_signal_handlers_disconnect_by_func(this->w, (gpointer) toolbarDragDataReceivedCb, this);

		cleanupToolbars();

		XOJ_RELEASE_TYPE(ToolbarAdapter);
	}

private:

	void cleanupToolbars() {
		GtkWidget * w = GTK_WIDGET(this->spacerItem);
		GtkWidget * parent = gtk_widget_get_parent(w);
		gtk_container_remove(GTK_CONTAINER(parent), w);
	}

	void showToolbar() {
		gtk_widget_show(this->w);

		GtkToolbar * tb = GTK_TOOLBAR(this->w);
		gtk_toolbar_set_icon_size(tb, GTK_ICON_SIZE_SMALL_TOOLBAR);

		GtkToolItem * it = gtk_tool_item_new();
		this->spacerItem = it;
		gtk_toolbar_insert(tb, it, 0);

		// TODO: ist hier wirklich metadata nötig?
		//		ToolitemDragDrop::attachMetadata(GTK_WIDGET(it), -2, NULL);

		GtkOrientation orientation = gtk_toolbar_get_orientation(tb);
		if (orientation == GTK_ORIENTATION_HORIZONTAL) {
			GtkAllocation alloc = { 0, 0, 0, 20 };
			gtk_widget_set_allocation(GTK_WIDGET(it), &alloc);
		} else if (orientation == GTK_ORIENTATION_VERTICAL) {
			GtkAllocation alloc = { 0, 0, 20, 0 };
			gtk_widget_set_allocation(GTK_WIDGET(it), &alloc);
		}
	}

private:

	/**
	 * A tool item was dragged to the toolbar
	 */
	static bool toolbarDragMotionCb(GtkToolbar * toolbar, GdkDragContext * context, gint x, gint y, guint time,
			ToolbarAdapter * adapter) {
		XOJ_CHECK_TYPE_OBJ(adapter, ToolbarAdapter);

		GdkAtom target = gtk_drag_dest_find_target(GTK_WIDGET(toolbar), context, NULL);
		if (target != ToolbarDragDropHelper::atomToolItem) {
			gdk_drag_status(context, (GdkDragAction) 0, time);
			return false;
		}

		gint ipos = gtk_toolbar_get_drop_index(toolbar, x, y);
		GtkOrientation orientation = gtk_toolbar_get_orientation(toolbar);
		gdk_drag_status(context, context->suggested_action, time);

		g_return_val_if_fail(ToolbarAdapter::currentDragItem != NULL, true);

		gtk_toolbar_set_drop_highlight_item(toolbar, ToolbarAdapter::currentDragItem->createTmpItem(orientation
				== GTK_ORIENTATION_HORIZONTAL), ipos);

		return true;
	}

	static void toolbarDragLeafeCb(GtkToolbar * toolbar, GdkDragContext * context, guint time, ToolbarAdapter * adapter) {
		XOJ_CHECK_TYPE_OBJ(adapter, ToolbarAdapter);

		gtk_toolbar_set_drop_highlight_item(toolbar, NULL, -1);
	}

	static void toolbarDragDataReceivedCb(GtkToolbar * toolbar, GdkDragContext * context, gint x, gint y,
			GtkSelectionData * data, guint info, guint time, ToolbarAdapter * adapter) {
		XOJ_CHECK_TYPE_OBJ(adapter, ToolbarAdapter);

		// TODO !!!!!!!!!!!!!!!!!!!!!!!
		ToolItemDragDropData * d = (ToolItemDragDropData *) gtk_selection_data_get_data(data);
		g_return_if_fail(ToolitemDragDrop::checkToolItemDragDropData(d));

		int pos = gtk_toolbar_get_drop_index(toolbar, x, y);

		if (d->type == TOOL_ITEM_ITEM) {

			bool horizontal = gtk_toolbar_get_orientation(toolbar) == GTK_ORIENTATION_HORIZONTAL;
			GtkToolItem * it = d->item->createItem(horizontal);
			gtk_widget_show_all(GTK_WIDGET(it));
			gtk_toolbar_insert(toolbar, it, pos);

			d->item->setUsed(true);

			ToolbarData * tb = adapter->window->getSelectedToolbar();
			const char * name = adapter->window->getToolbarName(toolbar);

			const char * id = d->item->getId().c_str();

			printf("id=%s; name=%s\n", id, name);

			tb->insertItem(name, id, pos);
		} else {
			g_warning("toolbarDragDataReceivedCb: ToolItemType %i not handled!", d->type);
		}

		// TODO !!!!!!!!!
		//adapter->listener->toolbarDataChanged();
	}

private:
	XOJ_TYPE_ATTRIB;

	GtkWidget * w;
	String toolbarName;
	MainWindow * window;

	GtkToolItem * spacerItem;
	ToolbarListener * listener;
	ToolMenuHandler * toolHandler;

public:

	/**
	 * TODO: LOW PRIO, not really a good solution...
	 */
	static AbstractToolItem * currentDragItem;
};

#endif /* __TOOLBARADAPTER_H__ */
