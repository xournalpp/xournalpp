
#include "ToolitemDragDrop.h"
#include "dialog/toolbarCustomize/ToolbarDragDropHelper.h"
#include "dialog/toolbarCustomize/ToolbarAdapter.h"

#include <string.h>

const char * ATTACH_DRAG_DROP_DATA = "XOJ_DRAG_DROP_DATA";


void ToolitemDragDrop::attachMetadata(GtkWidget * w, int id, AbstractToolItem * ait) {
	ToolItemDragDropData * d = g_new(ToolItemDragDropData, 1);
	d->identify = ToolItemDragDropData_Identify;
	d->id = id;
	d->item = ait;
	d->type = TOOL_ITEM_ITEM;

	g_object_set_data_full(G_OBJECT(w), ATTACH_DRAG_DROP_DATA, d, (GDestroyNotify) g_free);
}

ToolItemDragDropData * ToolitemDragDrop::ToolItemDragDropData_new(AbstractToolItem * item) {
	ToolItemDragDropData * d = g_new(ToolItemDragDropData, 1);
	d->identify = ToolItemDragDropData_Identify;
	d->id = -1;
	d->item = item;
	d->type = TOOL_ITEM_ITEM;

	return d;
}


bool ToolitemDragDrop::checkToolItemDragDropData(ToolItemDragDropData * d) {
	return d->identify == ToolItemDragDropData_Identify;
}


void ToolitemDragDrop::attachMetadata(GtkWidget * w, int id, String type) {

}


//
//void ToolitemDragDrop::attachMetadata(GtkWidget * w, int id, AbstractToolItem * ait) {
//	int * idData = (int *)g_new(int, 1);
//	*idData = id;
//
//	g_object_set_data_full(G_OBJECT(w), ATTACH_DRAG_DROP_ID, idData, (GDestroyNotify) g_free);
//	g_object_set_data_full(G_OBJECT(w), ATTACH_DRAG_DROP_TOOL_ITEM, ait, NULL);
//}
//
//void ToolitemDragDrop::attachMetadata(GtkWidget * w, int id, String type) {
//	int * idData = (int *)g_new(int, 1);
//	*idData = id;
//
//	g_object_set_data_full(G_OBJECT(w), ATTACH_DRAG_DROP_ID, idData, (GDestroyNotify) g_free);
//	g_object_set_data_full(G_OBJECT(w), ATTACH_DRAG_DROP_TYPE, g_strdup(type.c_str()), (GDestroyNotify) g_free);
//}

//int ToolitemDragDrop::metadataGetDragDropId(GtkWidget * w) {
//	const int * ptr = (const int *) g_object_get_data(G_OBJECT(w), ATTACH_DRAG_DROP_ID);
//
//	if (ptr == NULL) {
//		g_warning("Could not get Metadata %s from %s\n", ATTACH_DRAG_DROP_ID, g_type_name(G_TYPE_FROM_INSTANCE(w)));
//		return -1;
//	}
//
//	return *ptr;
//}


//for(GSList * l = this->dragDropData; l!=NULL; l=l->next) {
//	AbstractItemSelectionData * sd = (AbstractItemSelectionData *)l->data;
//
//	gtk_tool_item_set_use_drag_window(GTK_TOOL_ITEM(sd->w), false);
//
//	gtk_drag_source_unset(sd->w);
//
//	g_signal_handlers_disconnect_by_func(sd->w, (gpointer) toolitemDragBegin, this);
//	g_signal_handlers_disconnect_by_func(sd->w, (gpointer) toolitemDragEnd, this);
//
//
//	g_signal_handlers_disconnect_by_func(sd->w, (gpointer) toolitemDragDataGet, sd);
//
//	delete sd;
//}
//
//g_slist_free(this->dragDropData);
//this->dragDropData = NULL;


/**
 * Drag a Toolitem from toolbar
 */
void ToolitemDragDrop::toolitemDragBegin(GtkWidget * widget, GdkDragContext * context, void * data) {
//	AbstractToolItem * ait = metadataGetDragDropAbstractToolItem(widget);
//
//	g_return_if_fail(ait != NULL);
//
//	ToolbarAdapter::currentDragItem = ait;
//
//	GtkWidget * icon = ait->getNewToolIcon();
//	gtk_drag_set_icon_pixbuf(context, ToolbarDragDropHelper::getImagePixbuf(GTK_IMAGE(icon)), -2, -2);
//	g_object_ref_sink(icon);
//	gtk_widget_hide(widget);
}

/**
 * Drag a Toolitem from toolbar STOPPED
 */
void ToolitemDragDrop::toolitemDragEnd(GtkWidget * widget, GdkDragContext * context, void * data) {
	ToolbarAdapter::currentDragItem = NULL;

	gtk_widget_show(widget);
}

void ToolitemDragDrop::toolitemDragDataGet(GtkWidget * widget, GdkDragContext * context,
		GtkSelectionData * selection_data, guint info, guint time, void * data) {

	// TODO: !!!!!!!!!!!
//	ToolitemDragDrop::removeFromParentForMove(widget);
//	gtk_selection_data_set(selection_data, ToolbarDragDropHelper::atomToolItem, 0, (const guchar *) &widget,
//			sizeof(GtkWidget *));
}

// TODO: !!!!!!!!!!!
//void ToolitemDragDrop::removeFromParentForMove(GtkWidget * widget) {
//	char * type = (char *) g_object_get_data(G_OBJECT(widget), ATTACH_DRAG_DROP_SOURCE_TYPE);
//
//	if (type == NULL) {
//		g_warning("Could not get Metadata %s from %s\n", ATTACH_DRAG_DROP_SOURCE_TYPE, g_type_name(G_TYPE_FROM_INSTANCE(widget)));
//		return;
//	}
//
//	if(strcmp(SOURCE_TYPE_TOOLBAR, type) == 0) {
//		// remove widget from parent
//		GtkWidget * parent = gtk_widget_get_parent(widget);
//		gtk_container_remove(GTK_CONTAINER(parent), widget);
//
//		int id = ToolitemDragDrop::metadataGetDragDropId(widget);
//		AbstractToolItem * ptr = (AbstractToolItem *) g_object_get_data(G_OBJECT(w), ATTACH_DRAG_DROP_TOOL_ITEM);
//		if(ptr) {
//			ptr->setUsed(false);
//		}
//
//
//	} else if(strcmp(SOURCE_TYPE_DIALOG, type) == 0) {
//
//	} else {
//		g_warning("ToolitemDragDrop::removeFromParentForMove unknown source: \"%s\"", type);
//	}
//
//	// TODO !!!!! remove from parent
//}

void prepareToolbarItems() {
	//	GtkToolbar * tb = GTK_TOOLBAR(this->w);
	//	for (int i = 0; i < gtk_toolbar_get_n_items(tb); i++) {
	//		GtkToolItem * it = gtk_toolbar_get_nth_item(tb, i);
	//
	//		gtk_widget_set_sensitive(GTK_WIDGET(it), true);
	//
	//		gtk_tool_item_set_use_drag_window(it, true);
	//		gtk_drag_source_set(GTK_WIDGET(it), GDK_BUTTON1_MASK, &ToolbarDragDropHelper::dropTargetEntry, 1, GDK_ACTION_MOVE);
	//		ToolbarDragDropHelper::dragSourceAddToolbar(GTK_WIDGET(it));
	//
	//		g_signal_connect(it, "drag-begin", G_CALLBACK(toolitemDragBegin), this);
	//		g_signal_connect(it, "drag-end", G_CALLBACK(toolitemDragEnd), this);
	//
	//		AbstractToolItem * ait = this->toolHandler->getItemFor(GTK_WIDGET(it));
	//
	//		AbstractItemSelectionData * sd = new AbstractItemSelectionData(ait, this->toolbarName, ToolMenuHandler::metadataGetDragDropId(GTK_WIDGET(it)), GTK_WIDGET(it));
	//		g_signal_connect(it, "drag-data-get", G_CALLBACK(toolitemDragDataGet), sd);
	//
	//		this->dragDropData =  g_slist_prepend(this->dragDropData, sd);
	//	}
}
