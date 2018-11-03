#include "ToolbarAdapter.h"

#include "Util.h"

#include "control/Control.h"
#include "gui/toolbarMenubar/AbstractToolItem.h"
#include "gui/toolbarMenubar/ColorToolItem.h"
#include "gui/toolbarMenubar/model/ToolbarData.h"
#include "gui/ToolitemDragDrop.h"
#include "gui/widgets/SelectColor.h"
#include "ToolbarDragDropHelper.h"
#include "ToolItemDragCurrentData.h"

#include <i18n.h>

#include <iostream>
using std::cout;
using std::endl;

ToolbarAdapter::ToolbarAdapter(GtkWidget* toolbar, string toolbarName, ToolMenuHandler* toolHandler, MainWindow* window)
{
	XOJ_INIT_TYPE(ToolbarAdapter);

	this->w = toolbar;
	this->toolbarName = toolbarName;
	this->toolHandler = toolHandler;
	this->window = window;

	// prepare drag & drop
	gtk_drag_dest_set(toolbar, GTK_DEST_DEFAULT_ALL, NULL, 0, GDK_ACTION_MOVE);
	ToolbarDragDropHelper::dragDestAddToolbar(toolbar);

	g_signal_connect(toolbar, "drag_motion", G_CALLBACK(toolbarDragMotionCb), this);
	g_signal_connect(toolbar, "drag_leave", G_CALLBACK(toolbarDragLeafeCb), this);
	g_signal_connect(toolbar, "drag_data_received", G_CALLBACK(toolbarDragDataReceivedCb), this);

	showToolbar();
	prepareToolItems();
}

ToolbarAdapter::~ToolbarAdapter()
{
	XOJ_CHECK_TYPE(ToolbarAdapter);

	// remove drag & drop handler
	g_signal_handlers_disconnect_by_func(this->w, (gpointer) toolbarDragMotionCb, this);
	g_signal_handlers_disconnect_by_func(this->w, (gpointer) toolbarDragLeafeCb, this);
	g_signal_handlers_disconnect_by_func(this->w, (gpointer) toolbarDragDataReceivedCb, this);

	cleanupToolbars();

	XOJ_RELEASE_TYPE(ToolbarAdapter);
}

void ToolbarAdapter::cleanupToolbars()
{
	XOJ_CHECK_TYPE(ToolbarAdapter);

	GtkWidget* w = GTK_WIDGET(this->spacerItem);
	GtkWidget* parent = gtk_widget_get_parent(w);
	gtk_container_remove(GTK_CONTAINER(parent), w);

	GtkToolbar* tb = GTK_TOOLBAR(this->w);
	if (gtk_toolbar_get_n_items(tb) == 0)
	{
		gtk_widget_hide(this->w);
	}
	else
	{
		for (int i = 0; i < gtk_toolbar_get_n_items(tb); i++)
		{
			GtkToolItem* it = gtk_toolbar_get_nth_item(tb, i);
			cleanToolItem(it);
		}
	}
}

void ToolbarAdapter::prepareToolItems()
{
	XOJ_CHECK_TYPE(ToolbarAdapter);

	GtkToolbar* tb = GTK_TOOLBAR(this->w);

	for (int i = 0; i < gtk_toolbar_get_n_items(tb); i++)
	{
		GtkToolItem* it = gtk_toolbar_get_nth_item(tb, i);
		prepareToolItem(it);
	}
}

void ToolbarAdapter::cleanToolItem(GtkToolItem* it)
{
	XOJ_CHECK_TYPE(ToolbarAdapter);

	ToolItemDragDropData* data = ToolitemDragDrop::metadataGetMetadata(GTK_WIDGET( it));
	if (data)
	{
		gtk_widget_set_sensitive(GTK_WIDGET(it), ToolitemDragDrop::isToolItemEnabled(data));
	}

	gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(it)), NULL);

	gtk_tool_item_set_use_drag_window(it, false);
	gtk_drag_source_unset(GTK_WIDGET(it));

	g_signal_handlers_disconnect_by_func(it, (gpointer) toolitemDragBegin, NULL);
	g_signal_handlers_disconnect_by_func(it, (gpointer) toolitemDragEnd, NULL);
	g_signal_handlers_disconnect_by_func(it, (gpointer) toolitemDragDataGet, this);
}

void ToolbarAdapter::prepareToolItem(GtkToolItem* it)
{
	XOJ_CHECK_TYPE(ToolbarAdapter);

	// if disable drag an drop is not possible
	gtk_widget_set_sensitive(GTK_WIDGET(it), true);

	gtk_tool_item_set_use_drag_window(it, true);

	GdkScreen* screen = gtk_widget_get_screen(GTK_WIDGET(it));
	GdkCursor* cursor = gdk_cursor_new_for_display(gdk_screen_get_display(screen), GDK_HAND2);
	gdk_window_set_cursor(GTK_WIDGET(it)->window, cursor);
	gdk_cursor_unref(cursor);

	gtk_drag_source_set(GTK_WIDGET(it), GDK_BUTTON1_MASK, &ToolbarDragDropHelper::dropTargetEntry, 1, GDK_ACTION_MOVE);
	ToolbarDragDropHelper::dragSourceAddToolbar(GTK_WIDGET(it));

	g_signal_connect(it, "drag-begin", G_CALLBACK(toolitemDragBegin), NULL);
	g_signal_connect(it, "drag-end", G_CALLBACK(toolitemDragEnd), NULL);
	g_signal_connect(it, "drag-data-get", G_CALLBACK(toolitemDragDataGet), this);
}

void ToolbarAdapter::showToolbar()
{
	XOJ_CHECK_TYPE(ToolbarAdapter);

	gtk_widget_show(this->w);

	GtkToolbar* tb = GTK_TOOLBAR(this->w);
	gtk_toolbar_set_icon_size(tb, GTK_ICON_SIZE_SMALL_TOOLBAR);

	GtkToolItem* it = gtk_tool_item_new();
	this->spacerItem = it;
	gtk_toolbar_insert(tb, it, 0);

	GtkOrientation orientation = gtk_toolbar_get_orientation(tb);
	if (orientation == GTK_ORIENTATION_HORIZONTAL)
	{
		GtkAllocation alloc = { 0, 0, 0, 20 };
		gtk_widget_set_allocation(GTK_WIDGET(it), &alloc);
	}
	else if (orientation == GTK_ORIENTATION_VERTICAL)
	{
		GtkAllocation alloc = { 0, 0, 20, 0 };
		gtk_widget_set_allocation(GTK_WIDGET(it), &alloc);
	}
}

/**
 * Drag a Toolitem from toolbar
 */
void ToolbarAdapter::toolitemDragBegin(GtkWidget* widget, GdkDragContext* context, void* unused)
{
	ToolItemDragDropData* data = ToolitemDragDrop::metadataGetMetadata(widget);

	g_return_if_fail(data != NULL);

	ToolItemDragCurrentData::setData(data);

	GtkWidget* icon = ToolitemDragDrop::getIcon(data);

	gtk_drag_set_icon_pixbuf(context, ToolbarDragDropHelper::getImagePixbuf(GTK_IMAGE(icon)), -2, -2);

	gtk_widget_unref(icon);

	gtk_widget_hide(widget);
}

/**
 * Drag a Toolitem from toolbar STOPPED
 */
void ToolbarAdapter::toolitemDragEnd(GtkWidget* widget, GdkDragContext* context, void* unused)
{
	ToolItemDragCurrentData::clearData();
	gtk_widget_show(widget);
}

/**
 * Remove a toolbar item from the tool where it was
 */
void ToolbarAdapter::removeFromToolbar(AbstractToolItem* item, string toolbarName, int id)
{
	XOJ_CHECK_TYPE(ToolbarAdapter);

	ToolbarData* d = this->window->getSelectedToolbar();
	if (d->removeItemByID(toolbarName, id))
	{
		if (item != NULL)
		{
			cout << _F("Removed tool item {1} from Toolbar {2} ID {3}") % item->getId() % toolbarName % id << endl;
		}
		else
		{
			cout << _F("Removed tool item from Toolbar {1} ID {2}") % toolbarName % id << endl;
		}
	}
	else
	{
		if (item != NULL)
		{
			cout << _F("Could not remove tool item {1} from Toolbar {2} on position {3}")
						% item->getId() % toolbarName % id << endl;
		}
		else
		{
			cout << _F("Could not remove tool item from Toolbar {1} on position {2}")
						% toolbarName % id << endl;
		}
	}
}

void ToolbarAdapter::toolitemDragDataGet(GtkWidget* widget, GdkDragContext* context, GtkSelectionData* selection_data,
								guint info, guint time, ToolbarAdapter* adapter)
{
	XOJ_CHECK_TYPE_OBJ(adapter, ToolbarAdapter);

	ToolItemDragDropData* data = ToolitemDragDrop::metadataGetMetadata(widget);

	g_return_if_fail(data != NULL);

	GtkToolbar* tb = GTK_TOOLBAR(adapter->w);
	int position = -1;
	for (int i = 0; i < gtk_toolbar_get_n_items(tb); i++)
	{
		GtkToolItem* it = gtk_toolbar_get_nth_item(tb, i);

		if ((void*) it == (void*) widget)
		{
			adapter->cleanToolItem(it);
			gtk_container_remove(GTK_CONTAINER(tb), GTK_WIDGET(it));
			position = i;
			break;
		}
	}

	g_return_if_fail(position != -1);

	adapter->removeFromToolbar(data->item, adapter->toolbarName, data->id);

	gtk_selection_data_set(selection_data, ToolbarDragDropHelper::atomToolItem, 0,
						   (const guchar*) data, sizeof(ToolItemDragDropData));
}

/**
 * A tool item was dragged to the toolbar
 */
bool ToolbarAdapter::toolbarDragMotionCb(GtkToolbar* toolbar, GdkDragContext* context,
								gint x, gint y, guint time, ToolbarAdapter* adapter)
{
	XOJ_CHECK_TYPE_OBJ(adapter, ToolbarAdapter);

	GdkAtom target = gtk_drag_dest_find_target(GTK_WIDGET(toolbar), context, NULL);
	if (target != ToolbarDragDropHelper::atomToolItem)
	{
		gdk_drag_status(context, (GdkDragAction) 0, time);
		return false;
	}

	gint ipos = gtk_toolbar_get_drop_index(toolbar, x, y);
	GtkOrientation orientation = gtk_toolbar_get_orientation(toolbar);
	gdk_drag_status(context, context->suggested_action, time);

	ToolItemDragDropData* d = ToolItemDragCurrentData::getData();

	g_return_val_if_fail(d != NULL, NULL);

	if (d->type == TOOL_ITEM_ITEM)
	{
		gtk_toolbar_set_drop_highlight_item(toolbar,
											d->item->createTmpItem(orientation == GTK_ORIENTATION_HORIZONTAL), ipos);
	}
	else if (d->type == TOOL_ITEM_SEPARATOR)
	{
		GtkToolItem* it = gtk_separator_tool_item_new();
		gtk_toolbar_set_drop_highlight_item(toolbar, it, ipos);
	}
	else if (d->type == TOOL_ITEM_COLOR)
	{
		GtkWidget* iconWidget = selectcolor_new(d->color);
		selectcolor_set_size(iconWidget, 16);
		selectcolor_set_circle(iconWidget, true);
		GtkToolItem* it = gtk_tool_button_new(iconWidget, "");
		gtk_toolbar_set_drop_highlight_item(toolbar, it, ipos);
	}
	else
	{
		g_warning("ToolbarAdapter::toolbarDragMotionCb Unhandled type %i", d->type);
	}

	return true;
}

void ToolbarAdapter::toolbarDragLeafeCb(GtkToolbar* toolbar, GdkDragContext* context, guint time, ToolbarAdapter* adapter)
{
	XOJ_CHECK_TYPE_OBJ(adapter, ToolbarAdapter);

	gtk_toolbar_set_drop_highlight_item(toolbar, NULL, -1);
}

void ToolbarAdapter::toolbarDragDataReceivedCb(GtkToolbar* toolbar, GdkDragContext* context, gint x, gint y,
									  GtkSelectionData* data, guint info, guint time, ToolbarAdapter* adapter)
{
	XOJ_CHECK_TYPE_OBJ(adapter, ToolbarAdapter);

	ToolItemDragDropData* d = (ToolItemDragDropData*) gtk_selection_data_get_data( data);
	g_return_if_fail(ToolitemDragDrop::checkToolItemDragDropData(d));

	int pos = gtk_toolbar_get_drop_index(toolbar, x, y);

	if (d->type == TOOL_ITEM_ITEM)
	{
		bool horizontal = gtk_toolbar_get_orientation(toolbar) == GTK_ORIENTATION_HORIZONTAL;
		GtkToolItem* it = d->item->createItem(horizontal);

		adapter->prepareToolItem(it);

		gtk_widget_show_all(GTK_WIDGET(it));
		gtk_toolbar_insert(toolbar, it, pos);

		ToolbarData* tb = adapter->window->getSelectedToolbar();
		const char* name = adapter->window->getToolbarName(toolbar);

		string id = d->item->getId();

		int newId = tb->insertItem(name, id, pos);
		ToolitemDragDrop::attachMetadata(GTK_WIDGET(it), newId, d->item);
	}
	else if (d->type == TOOL_ITEM_COLOR)
	{
		ColorToolItem* item = new ColorToolItem(adapter->window->getControl(),
												adapter->window->getControl()->getToolHandler(),
												GTK_WINDOW(adapter->window->getWindow()), d->color);

		bool horizontal = gtk_toolbar_get_orientation(toolbar) == GTK_ORIENTATION_HORIZONTAL;
		GtkToolItem* it = item->createItem(horizontal);

		adapter->prepareToolItem(it);

		gtk_widget_show_all(GTK_WIDGET(it));
		gtk_toolbar_insert(toolbar, it, pos);

		ToolbarData* tb = adapter->window->getSelectedToolbar();
		const char* name = adapter->window->getToolbarName(toolbar);

		string id = item->getId();

		int newId = tb->insertItem(name, id, pos);
		cout << "TOOL_ITEM_COLOR attach metadata" << endl;
		ToolitemDragDrop::attachMetadataColor(GTK_WIDGET(it), newId, d->color, item);

		adapter->window->getToolMenuHandler()->addColorToolItem(item);
	}
	else if (d->type == TOOL_ITEM_SEPARATOR)
	{
		GtkToolItem* it = gtk_separator_tool_item_new();
		gtk_widget_show_all(GTK_WIDGET(it));
		gtk_toolbar_insert(toolbar, it, pos);

		adapter->prepareToolItem(it);

		ToolbarData* tb = adapter->window->getSelectedToolbar();
		const char* name = adapter->window->getToolbarName(toolbar);

		int newId = tb->insertItem(name, "SEPARATOR", pos);
		ToolitemDragDrop::attachMetadata(GTK_WIDGET(it), newId, TOOL_ITEM_SEPARATOR);
	}
	else
	{
		g_warning("toolbarDragDataReceivedCb: ToolItemType %i not handled!", d->type);
	}
}

