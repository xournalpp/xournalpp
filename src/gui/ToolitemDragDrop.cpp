#include "ToolitemDragDrop.h"
#include "dialog/toolbarCustomize/ToolbarDragDropHelper.h"
#include "dialog/toolbarCustomize/ToolbarAdapter.h"

#include <string.h>

const char* ATTACH_DRAG_DROP_DATA = "XOJ_DRAG_DROP_DATA";

void ToolitemDragDrop::attachMetadata(GtkWidget* w, int id,
                                      AbstractToolItem* ait)
{
	ToolItemDragDropData* d = g_new(ToolItemDragDropData, 1);
	d->identify = ToolItemDragDropData_Identify;
	d->id = id;
	d->item = ait;
	d->type = TOOL_ITEM_ITEM;
	d->color = 0;

	g_object_set_data_full(G_OBJECT(w), ATTACH_DRAG_DROP_DATA, d,
	                       (GDestroyNotify) g_free);
}

ToolItemDragDropData* ToolitemDragDrop::ToolItemDragDropData_new(
    AbstractToolItem* item)
{
	ToolItemDragDropData* d = g_new(ToolItemDragDropData, 1);
	d->identify = ToolItemDragDropData_Identify;
	d->id = -1;
	d->item = item;
	d->type = TOOL_ITEM_ITEM;
	d->color = 0;

	return d;
}

void ToolitemDragDrop::attachMetadata(GtkWidget* w, int id, ToolItemType type)
{
	ToolItemDragDropData* d = g_new(ToolItemDragDropData, 1);
	d->identify = ToolItemDragDropData_Identify;
	d->id = id;
	d->item = NULL;
	d->type = type;
	d->color = 0;

	g_object_set_data_full(G_OBJECT(w), ATTACH_DRAG_DROP_DATA, d,
	                       (GDestroyNotify) g_free);
}

void ToolitemDragDrop::attachMetadataColor(GtkWidget* w, int id, int color,
                                           AbstractToolItem* item)
{
	ToolItemDragDropData* d = g_new(ToolItemDragDropData, 1);
	d->identify = ToolItemDragDropData_Identify;
	d->id = id;
	d->item = item;
	d->type = TOOL_ITEM_COLOR;
	d->color = color;

	g_object_set_data_full(G_OBJECT(w), ATTACH_DRAG_DROP_DATA, d,
	                       (GDestroyNotify) g_free);
}

GtkWidget* ToolitemDragDrop::getIcon(ToolItemDragDropData* data)
{
	if (data->type == TOOL_ITEM_ITEM || (data->type == TOOL_ITEM_COLOR &&
	                                     data->item != NULL))
	{
		return data->item->getNewToolIcon();
	}
	else if (data->type == TOOL_ITEM_COLOR)
	{
		GdkPixbuf* pixbuf = ToolbarDragDropHelper::getColorImage(data->color);
		GtkWidget* w = gtk_image_new_from_pixbuf(pixbuf);
		gdk_pixbuf_unref(pixbuf);
		return w;
	}
	else if (data->type == TOOL_ITEM_SEPARATOR)
	{
		return Util::newSepeartorImage();
	}

	g_warning("ToolitemDragDrop::getIcon unhandled type: %i\n", data->type);
	return gtk_image_new();
}

bool ToolitemDragDrop::checkToolItemDragDropData(ToolItemDragDropData* d)
{
	return d->identify == ToolItemDragDropData_Identify;
}

bool ToolitemDragDrop::isToolItemEnabled(ToolItemDragDropData* d)
{
	if (!checkToolItemDragDropData(d))
	{
		g_warning("ToolitemDragDrop::isToolItemEnabled data is not an instance of ToolItemDragDropData!");
		return false;
	}

	if (d->type == TOOL_ITEM_SEPARATOR)
	{
		return true;
	}

	if (d->type == TOOL_ITEM_COLOR && d->item == NULL)
	{
		return true;
	}

	g_return_val_if_fail(d->item != NULL, true);

	return d->item->isEnabled();
}

ToolItemDragDropData* ToolitemDragDrop::metadataGetMetadata(GtkWidget* w)
{
	const int* ptr = (const int*) g_object_get_data(G_OBJECT(w),
	                                                ATTACH_DRAG_DROP_DATA);

	if (ptr == NULL)
	{
		g_warning("ToolitemDragDrop::metadataGetMetadata Could not get Metadata %s from %s\n",
		          ATTACH_DRAG_DROP_DATA,
		          g_type_name(G_TYPE_FROM_INSTANCE(w)));
		return NULL;
	}

	if (!checkToolItemDragDropData((ToolItemDragDropData*) ptr))
	{
		g_warning("ToolitemDragDrop::metadataGetMetadata data is not an instance of ToolItemDragDropData!");

		return NULL;
	}

	return (ToolItemDragDropData*) ptr;
}

