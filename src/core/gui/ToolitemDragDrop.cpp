#include "ToolitemDragDrop.h"

#include <glib-object.h>  // for g_object_...
#include <glib.h>         // for g_malloc

#include "gui/toolbarMenubar/AbstractToolItem.h"            // for AbstractT...
#include "gui/toolbarMenubar/icon/ToolbarSeparatorImage.h"  // for getNewToo...

struct NamedColor;

const char* ATTACH_DRAG_DROP_DATA = "XOJ_DRAG_DROP_DATA";

void ToolitemDragDrop::attachMetadata(GtkWidget* w, int id, AbstractToolItem* ait) {
    ToolItemDragDropData* d = g_new(ToolItemDragDropData, 1);
    d->identify = ToolItemDragDropData_Identify;
    d->id = id;
    d->item = ait;
    d->type = TOOL_ITEM_ITEM;
    d->namedColor = nullptr;

    g_object_set_data_full(G_OBJECT(w), ATTACH_DRAG_DROP_DATA, d, static_cast<GDestroyNotify>(g_free));
}

auto ToolitemDragDrop::ToolItemDragDropData_new(AbstractToolItem* item) -> ToolItemDragDropData* {
    ToolItemDragDropData* d = g_new(ToolItemDragDropData, 1);
    d->identify = ToolItemDragDropData_Identify;
    d->id = -1;
    d->item = item;
    d->type = TOOL_ITEM_ITEM;
    d->namedColor = nullptr;

    return d;
}

void ToolitemDragDrop::attachMetadata(GtkWidget* w, int id, ToolItemType type) {
    ToolItemDragDropData* d = g_new(ToolItemDragDropData, 1);
    d->identify = ToolItemDragDropData_Identify;
    d->id = id;
    d->item = nullptr;
    d->type = type;
    d->namedColor = nullptr;

    g_object_set_data_full(G_OBJECT(w), ATTACH_DRAG_DROP_DATA, d, static_cast<GDestroyNotify>(g_free));
}

void ToolitemDragDrop::attachMetadataColor(GtkWidget* w, int id, const NamedColor* namedColor, AbstractToolItem* item) {
    ToolItemDragDropData* d = g_new(ToolItemDragDropData, 1);
    d->identify = ToolItemDragDropData_Identify;
    d->id = id;
    d->item = item;
    d->type = TOOL_ITEM_COLOR;
    d->namedColor = namedColor;

    g_object_set_data_full(G_OBJECT(w), ATTACH_DRAG_DROP_DATA, d, static_cast<GDestroyNotify>(g_free));
}

auto ToolitemDragDrop::getIcon(ToolItemDragDropData* data) -> GtkWidget* {
    if (data->type == TOOL_ITEM_ITEM || data->type == TOOL_ITEM_COLOR) {
        return data->item->getNewToolIcon();
    }
    if (data->type == TOOL_ITEM_SEPARATOR) {
        return ToolbarSeparatorImage::newImage();
    }

    g_warning("ToolitemDragDrop::getIcon unhandled type: %i\n", data->type);
    return gtk_image_new();
}

auto ToolitemDragDrop::getPixbuf(ToolItemDragDropData* data) -> GdkPixbuf* {
    if (data->type == TOOL_ITEM_ITEM || data->type == TOOL_ITEM_COLOR) {
        return data->item->getNewToolPixbuf();
    }
    if (data->type == TOOL_ITEM_SEPARATOR) {
        return ToolbarSeparatorImage::getNewToolPixbuf();
    }
    g_error("ToolitemDragDrop::getIcon unhandled type: %i\n", data->type);
}

auto ToolitemDragDrop::checkToolItemDragDropData(ToolItemDragDropData const* d) -> bool {
    return d->identify == ToolItemDragDropData_Identify;
}

auto ToolitemDragDrop::isToolItemEnabled(ToolItemDragDropData* d) -> bool {
    if (!checkToolItemDragDropData(d)) {
        g_warning("ToolitemDragDrop::isToolItemEnabled data is not an instance of ToolItemDragDropData!");
        return false;
    }
    if (d->type == TOOL_ITEM_SEPARATOR) {
        return true;
    }
    if (d->type == TOOL_ITEM_SPACER) {
        return true;
    }
    if (d->type == TOOL_ITEM_COLOR && d->item == nullptr) {
        return true;
    }

    g_return_val_if_fail(d->item != nullptr, true);

    return d->item->isEnabled();
}

auto ToolitemDragDrop::metadataGetMetadata(GtkWidget* w) -> ToolItemDragDropData* {
    void* ptr = g_object_get_data(G_OBJECT(w), ATTACH_DRAG_DROP_DATA);
    if (ptr == nullptr) {
        g_warning("ToolitemDragDrop::metadataGetMetadata Could not get Metadata %s from %s\n", ATTACH_DRAG_DROP_DATA,
                  g_type_name(G_TYPE_FROM_INSTANCE(w)));
        return nullptr;
    }
    if (!checkToolItemDragDropData(static_cast<ToolItemDragDropData const*>(ptr))) {
        g_warning("ToolitemDragDrop::metadataGetMetadata data is not an instance of ToolItemDragDropData!");
        return nullptr;
    }
    return static_cast<ToolItemDragDropData*>(ptr);
}
