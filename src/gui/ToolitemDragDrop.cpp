
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

bool ToolitemDragDrop::isToolItemEnabled(ToolItemDragDropData * d) {
	if(!checkToolItemDragDropData(d)) {
		g_warning("ToolitemDragDrop::isToolItemEnabled data is not an instance of ToolItemDragDropData!");
		return false;
	}

	return d->item->isEnabled();
}

void ToolitemDragDrop::attachMetadata(GtkWidget * w, int id, String type) {

}

ToolItemDragDropData * ToolitemDragDrop::metadataGetMetadata(GtkWidget * w) {
	const int * ptr = (const int *) g_object_get_data(G_OBJECT(w), ATTACH_DRAG_DROP_DATA);

	if (ptr == NULL) {
		g_warning("ToolitemDragDrop::metadataGetMetadata Could not get Metadata %s from %s\n", ATTACH_DRAG_DROP_DATA, g_type_name(G_TYPE_FROM_INSTANCE(w)));
		return NULL;
	}

	if(!checkToolItemDragDropData((ToolItemDragDropData *)ptr)) {
		g_warning("ToolitemDragDrop::metadataGetMetadata data is not an instance of ToolItemDragDropData!");

		return NULL;
	}

	return (ToolItemDragDropData *)ptr;
}


