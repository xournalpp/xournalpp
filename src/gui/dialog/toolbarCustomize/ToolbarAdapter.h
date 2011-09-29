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
#include "../../toolbarMenubar/ToolMenuHandler.h"
#include "../../MainWindow.h"
#include "../../toolbarMenubar/model/ToolbarData.h"

#include "../../../util/Util.h"

class ToolbarAdapter {
public:
	ToolbarAdapter(GtkWidget * toolbar, String toolbarName, ToolbarListener * listener, ToolMenuHandler * toolHandler, MainWindow * window) {
		XOJ_INIT_TYPE( ToolbarAdapter);

		this->w = toolbar;
		this->toolbarName = toolbarName;
		this->listener = listener;
		this->toolHandler = toolHandler;
		this->window = window;

		this->dragDropData = NULL;

		// prepare drag & drop
		gtk_drag_dest_set(toolbar, GTK_DEST_DEFAULT_ALL, NULL, 0, GDK_ACTION_MOVE);
		ToolbarDragDropHelper::dragDestAddToolbar(toolbar);

		g_signal_connect(toolbar, "drag_motion", G_CALLBACK(toolbarDragMotionCb), this);
		g_signal_connect(toolbar, "drag_leave", G_CALLBACK(toolbarDragLeafeCb), this);
		g_signal_connect(toolbar, "drag_data_received", G_CALLBACK(toolbarDragDataReceivedCb), this);

		showToolbar();

		this->prepareToolbarItems();

		// TODO: cursor wider löschen
		// gdk_window_set_cursor (gtk_widget_get_window (GTK_WIDGET(item)), NULL);

	}

	~ToolbarAdapter() {
		XOJ_CHECK_TYPE( ToolbarAdapter);

		// remove drag & drop handler
		g_signal_handlers_disconnect_by_func(this->w, (gpointer) toolbarDragMotionCb, this);
		g_signal_handlers_disconnect_by_func(this->w, (gpointer) toolbarDragLeafeCb, this);
		g_signal_handlers_disconnect_by_func(this->w, (gpointer) toolbarDragDataReceivedCb, this);


		// TODO: !!!! auch beim verschrieben in den DIalog die Handler entfernen!
		cleanupToolbars();

		XOJ_RELEASE_TYPE(ToolbarAdapter);
	}

private:

	/**
	 * Drag a Toolitem from toolbar
	 */
	static void toolitemDragBegin(GtkWidget * widget, GdkDragContext * context, ToolbarAdapter * adapter) {
		XOJ_CHECK_TYPE_OBJ(adapter, ToolbarAdapter);
		AbstractToolItem * ait = adapter->toolHandler->getItemFor(widget);

		g_return_if_fail(ait != NULL);

		ToolbarAdapter::currentDragItem = ait;

		GtkWidget * icon = ait->getNewToolIcon();
		gtk_drag_set_icon_pixbuf(context, ToolbarDragDropHelper::getImagePixbuf(GTK_IMAGE(icon)), -2, -2);
		g_object_ref_sink(icon);
		gtk_widget_hide(widget);
	}

	/**
	 * Drag a Toolitem from toolbar STOPPED
	 */
	static void toolitemDragEnd(GtkWidget * widget, GdkDragContext * context, ToolbarAdapter * adapter) {
		XOJ_CHECK_TYPE_OBJ(adapter, ToolbarAdapter);

		ToolbarAdapter::currentDragItem = NULL;

		gtk_widget_show(widget);
	}

	static void toolitemDragDataGet(GtkWidget * widget, GdkDragContext * context, GtkSelectionData * selection_data, guint info, guint time,
			AbstractItemSelectionData * item) {
		if (item == NULL) {
			printf("toolitemDragDataGet: ait == NULL\n");
			return;
		}
		XOJ_CHECK_TYPE_OBJ(item->item, AbstractToolItem);

		gtk_selection_data_set(selection_data, ToolbarDragDropHelper::atomToolItem, 0, (const guchar *) item, sizeof(AbstractItemSelectionData));
	}
	//
	//	/**
	//	 * A tool item was dragged to the dialog
	//	 */
	//	void ToolbarCustomizeDialog::dragDataReceived(GtkWidget * widget, GdkDragContext * dragContext, gint x, gint y, GtkSelectionData * data, guint info,
	//			guint time, ToolbarCustomizeDialog * dlg) {
	//
	//		XOJ_CHECK_TYPE_OBJ(dlg, ToolbarCustomizeDialog);
	//
	//		if (gtk_selection_data_get_data_type(data) != ToolbarDragDropHelper::atomToolItem) {
	//			gtk_drag_finish(dragContext, false, false, time);
	//			return;
	//		}
	//
	//		AbstractItemSelectionData * d = NULL;
	//		d = (AbstractItemSelectionData *) gtk_selection_data_get_data(data);
	//
	//		AbstractToolItem * item = d->item;
	//
	//		if (item->isUsed()) {
	//			dlg->removeFromToolbar(item);
	//			dlg->rebuildIconview();
	//		}
	//
	//		gtk_drag_finish(dragContext, true, false, time);
	//	}
	//
	//

	void prepareToolbarItems() {
		GtkToolbar * tb = GTK_TOOLBAR(this->w);
		for (int i = 0; i < gtk_toolbar_get_n_items(tb); i++) {
			GtkToolItem * it = gtk_toolbar_get_nth_item(tb, i);

			gtk_widget_set_sensitive(GTK_WIDGET(it), true);

			gtk_tool_item_set_use_drag_window(it, true);
			gtk_drag_source_set(GTK_WIDGET(it), GDK_BUTTON1_MASK, &ToolbarDragDropHelper::dropTargetEntry, 1, GDK_ACTION_MOVE);
			ToolbarDragDropHelper::dragSourceAddToolbar(GTK_WIDGET(it));

			g_signal_connect(it, "drag-begin", G_CALLBACK(toolitemDragBegin), this);
			g_signal_connect(it, "drag-end", G_CALLBACK(toolitemDragEnd), this);

			AbstractToolItem * ait = this->toolHandler->getItemFor(GTK_WIDGET(it));

			AbstractItemSelectionData * sd = new AbstractItemSelectionData(ait, this->toolbarName, ToolMenuHandler::metadataGetDragDropId(GTK_WIDGET(it)), GTK_WIDGET(it));
			g_signal_connect(it, "drag-data-get", G_CALLBACK(toolitemDragDataGet), sd);

			this->dragDropData =  g_slist_prepend(this->dragDropData, sd);
		}
	}

	void cleanupToolbars() {
		GtkWidget * w = GTK_WIDGET(this->spacerItem);
		GtkWidget * parent = gtk_widget_get_parent(w);
		gtk_container_remove(GTK_CONTAINER(parent), w);


		for(GSList * l = this->dragDropData; l!=NULL; l=l->next) {
			AbstractItemSelectionData * sd = (AbstractItemSelectionData *)l->data;

			gtk_tool_item_set_use_drag_window(GTK_TOOL_ITEM(sd->w), false);

			gtk_drag_source_unset(sd->w);

			g_signal_handlers_disconnect_by_func(sd->w, (gpointer) toolitemDragBegin, this);
			g_signal_handlers_disconnect_by_func(sd->w, (gpointer) toolitemDragEnd, this);


			g_signal_handlers_disconnect_by_func(sd->w, (gpointer) toolitemDragDataGet, sd);

			delete sd;
		}

		g_slist_free(this->dragDropData);
		this->dragDropData = NULL;
	}

	void showToolbar() {
		gtk_widget_show(this->w);

		GtkToolbar * tb = GTK_TOOLBAR(this->w);
		gtk_toolbar_set_icon_size(tb, GTK_ICON_SIZE_SMALL_TOOLBAR);

		GtkToolItem * it = gtk_tool_item_new();
		this->spacerItem = it;
		gtk_toolbar_insert(tb, it, 0);
		ToolMenuHandler::attachMetadata(GTK_WIDGET(it), -2);

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
		gdk_drag_status(context, context->suggested_action, time);

		g_return_val_if_fail(ToolbarAdapter::currentDragItem != NULL, true);

		gtk_toolbar_set_drop_highlight_item(toolbar, ToolbarAdapter::currentDragItem->createTmpItem(orientation == GTK_ORIENTATION_HORIZONTAL), ipos);

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

		// Not needed anymore
		//		bool horizontal = gtk_toolbar_get_orientation(toolbar) == GTK_ORIENTATION_HORIZONTAL;
		//		GtkToolItem * it = d->item->createItem(horizontal);
		//		gtk_widget_show_all(GTK_WIDGET(it));
		//		gtk_toolbar_insert(toolbar, it, pos);

		d->item->setUsed(true);

		ToolbarData * tb = adapter->window->getSelectedToolbar();
		String name = adapter->window->getToolbarName(toolbar);

		tb->addItem(name, d->item->getId(), pos);

		adapter->listener->toolbarDataChanged();
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
