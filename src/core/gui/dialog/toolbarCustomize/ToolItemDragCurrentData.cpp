#include "ToolItemDragCurrentData.h"

#include <glib.h>  // for g_malloc, g_malloc_n, g_free, g_new

#include "util/Stacktrace.h"  // for Stacktrace

class AbstractToolItem;
struct NamedColor;

std::unique_ptr<ToolItemDragDropData> ToolItemDragCurrentData::data = nullptr;

void ToolItemDragCurrentData::clearData() { data.reset(); }

void ToolItemDragCurrentData::setData(GtkWidget* widget) {
    data = std::make_unique<ToolItemDragDropData>();

    ToolItemDragDropData* d = ToolitemDragDrop::metadataGetMetadata(widget);
    if (d == nullptr) {
        g_warning("ToolItemDragCurrentData::setData(GtkWidget * widget) could not get data!");
        Stacktrace::printStacktrace();
        return;
    }

    *data = *d;
}

void ToolItemDragCurrentData::setData(ToolItemType type, int id, AbstractToolItem* item) {
    g_return_if_fail(item != nullptr || type != TOOL_ITEM_ITEM);

    data = ToolitemDragDrop::ToolItemDragDropData_new(item);
    data->type = type;
    data->id = id;
}

void ToolItemDragCurrentData::setDataColor(int id, size_t paletteColorIndex) {
    data = ToolitemDragDrop::ToolItemDragDropData_new(nullptr);
    data->type = TOOL_ITEM_COLOR;
    data->id = id;
    data->paletteColorIndex = paletteColorIndex;
}

void ToolItemDragCurrentData::setData(ToolItemDragDropData* d) {
    data = std::make_unique<ToolItemDragDropData>();
    *data = *d;
}

auto ToolItemDragCurrentData::getData() -> const ToolItemDragDropData* { return data.get(); }
