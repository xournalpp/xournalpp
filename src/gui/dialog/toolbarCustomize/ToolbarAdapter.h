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

#include "AbstractItemSelectionData.h"
#include "ToolbarListener.h"
#include "ToolbarDragDropHelper.h"
#include "../../toolbarMenubar/AbstractToolItem.h"

class ToolbarAdapter {
public:
	ToolbarAdapter(GtkWidget * w, ToolbarListener * listener) {
		XOJ_INIT_TYPE( ToolbarAdapter);

		this->w = w;
		this->listener = listener;

		// prepare drag & drop
		gtk_drag_dest_set(w, GTK_DEST_DEFAULT_ALL, NULL, 0, GDK_ACTION_MOVE);
		ToolbarDragDropHelper::dragDestAddToolbar(w);

		g_signal_connect(w, "drag_motion", G_CALLBACK(toolbarDragMotionCb), this);
		g_signal_connect(w, "drag_leave", G_CALLBACK(toolbarDragLeafeCb), this);
		g_signal_connect(w, "drag_data_received", G_CALLBACK(toolbarDragDataReceivedCb), this);

		showToolbar();

		GtkToolItem * toolbarItemContents = gtk_tool_button_new_from_stock(GTK_STOCK_JUSTIFY_CENTER);
		GtkToolbar * tb = GTK_TOOLBAR(this->w);

		gtk_widget_show(GTK_WIDGET(toolbarItemContents));
		gtk_toolbar_insert(tb, toolbarItemContents, 0);


//
//
//		GtkToolItem * toolItemToolbar = gtk_tool_item_new();
//
//
//
//
//		GtkWidget * ebox = gtk_event_box_new();
//		gtk_container_add(GTK_CONTAINER(ebox), GTK_WIDGET(toolbarItemContents));
//
//		/* make ebox a drag source */
//		gtk_drag_source_set(ebox, GDK_BUTTON1_MASK, &ToolbarDragDropHelper::dropTargetEntry, 1, GDK_ACTION_MOVE);
//		ToolbarDragDropHelper::dragSourceAddToolbar(ebox);
//
//		g_signal_connect(ebox, "drag-begin", G_CALLBACK(TESTtoolitemDragBegin), NULL);
//		g_signal_connect(ebox, "drag-end", G_CALLBACK(TESTtoolitemDragEnd), NULL);
//
//		gtk_container_add(GTK_CONTAINER(toolItemToolbar), ebox);
//
//
//
//		gtk_widget_show_all(GTK_WIDGET(toolItemToolbar));
//
//
//
////		AbstractItemSelectionData * sd = new AbstractItemSelectionData(item);
////		g_signal_connect(test, "drag-data-get", G_CALLBACK(toolitemDragDataGet), sd);
//
//		gtk_toolbar_insert(tb, toolItemToolbar, 0);


//		  g_signal_connect (toolbar, "drag_drop",
//				    G_CALLBACK (toolbar_drag_drop), label);

	}


	static void TESTtoolitemDragBegin(GtkWidget * widget, GdkDragContext * context, void * data) {
		gtk_widget_hide(widget);
	}

	static void TESTtoolitemDragEnd(GtkWidget * widget, GdkDragContext * context, void * data) {
		gtk_widget_show(widget);
	}


	~ToolbarAdapter() {
		XOJ_CHECK_TYPE( ToolbarAdapter);

		// remove drag & drop handler
		g_signal_handlers_disconnect_by_func(this->w, (gpointer) toolbarDragMotionCb, this);
		g_signal_handlers_disconnect_by_func(this->w, (gpointer) toolbarDragLeafeCb, this);
		g_signal_handlers_disconnect_by_func(this->w, (gpointer) toolbarDragDataReceivedCb, this);

		XOJ_RELEASE_TYPE(ToolbarAdapter);
	}

private:

	void cleanupToolbars() {
		GtkWidget * w = GTK_WIDGET(this->spacerItem);
		GtkWidget * parent = gtk_widget_get_parent(w);
		gtk_container_remove(GTK_CONTAINER(parent), w);

		//		GtkToolbar * tb = GTK_TOOLBAR(this->w);
		//		if (gtk_toolbar_get_n_items(tb) == 0) {
		//			gtk_widget_hide(GTK_WIDGET(tb));
		//		}
	}

	void showToolbar() {
		gtk_widget_show(this->w);

		GtkToolbar * tb = GTK_TOOLBAR(this->w);
		gtk_toolbar_set_icon_size(tb, GTK_ICON_SIZE_SMALL_TOOLBAR);

		GtkToolItem * it = gtk_tool_item_new();
		this->spacerItem = it;
		gtk_toolbar_insert(tb, it, 0);

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
	static bool toolbarDragMotionCb(GtkToolbar * toolbar, GdkDragContext * context, gint x, gint y, guint time, ToolbarAdapter * adapter) {
		XOJ_CHECK_TYPE_OBJ(adapter, ToolbarAdapter);

		GdkAtom target = gtk_drag_dest_find_target(GTK_WIDGET(toolbar), context, NULL);
		if (target != ToolbarDragDropHelper::atomToolItem) {
			gdk_drag_status(context, (GdkDragAction) 0, time);
			return false;
		}

		gint ipos = gtk_toolbar_get_drop_index(toolbar, x, y);
		GtkOrientation orientation = gtk_toolbar_get_orientation(toolbar);

		// TODO: !!!
		//		gtk_toolbar_set_drop_highlight_item(toolbar, dlg->currentDragItem->createItem(orientation == GTK_ORIENTATION_HORIZONTAL), ipos);

		gdk_drag_status(context, context->suggested_action, time);

		return true;
	}

	static void toolbarDragLeafeCb(GtkToolbar * toolbar, GdkDragContext * context, guint time, ToolbarAdapter * adapter) {
		XOJ_CHECK_TYPE_OBJ(adapter, ToolbarAdapter);

		gtk_toolbar_set_drop_highlight_item(toolbar, NULL, -1);
	}

	static void toolbarDragDataReceivedCb(GtkToolbar * toolbar, GdkDragContext * context, gint x, gint y, GtkSelectionData * data, guint info, guint time,
			ToolbarAdapter * adapter) {
		XOJ_CHECK_TYPE_OBJ(adapter, ToolbarAdapter);

		AbstractItemSelectionData * d = (AbstractItemSelectionData *) gtk_selection_data_get_data(data);

		int pos = gtk_toolbar_get_drop_index(toolbar, x, y);

		bool horizontal = gtk_toolbar_get_orientation(toolbar) == GTK_ORIENTATION_HORIZONTAL;

		GtkToolItem * it = d->item->createItem(horizontal);
		gtk_widget_show_all(GTK_WIDGET(it));
		gtk_toolbar_insert(toolbar, it, pos);

		d->item->setUsed(true);

		adapter->listener->toolbarDataChanged();

		// TODO: !!!
		//		ToolbarData * tb = d->dlg->win->getSelectedToolbar();
		//		String name = d->dlg->win->getToolbarName(toolbar);
		//
		//		tb->addItem(name, d->item->getId(), pos);
		//
		//		dlg->prepareToolbarsItemsDrag();
	}

private:
	XOJ_TYPE_ATTRIB;

	GtkWidget * w;
	GtkToolItem * spacerItem;
	ToolbarListener * listener;
};

#endif /* __TOOLBARADAPTER_H__ */
