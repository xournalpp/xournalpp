#include "ToolItemDragCurrentData.h"
#include <Stacktrace.h>

ToolItemDragDropData * ToolItemDragCurrentData::data = NULL;

void ToolItemDragCurrentData::clearData() {
	g_free(data);
	data = NULL;
}

void ToolItemDragCurrentData::setData(GtkWidget * widget) {
	data = g_new(ToolItemDragDropData, 1);

	ToolItemDragDropData * d = ToolitemDragDrop::metadataGetMetadata(widget);
	if(d == NULL) {
		g_warning("ToolItemDragCurrentData::setData(GtkWidget * widget) could not get data!");
		Stacktrace::printStracktrace();
		return;
	}

	*data = *d;
}

void ToolItemDragCurrentData::setData(ToolItemType type, int id, AbstractToolItem * item) {
	g_return_if_fail(item != NULL || type != TOOL_ITEM_ITEM);

	data = ToolitemDragDrop::ToolItemDragDropData_new(item);
	data->type = type;
	data->id = id;
}

void ToolItemDragCurrentData::setDataColor(int id, int color) {
	data = ToolitemDragDrop::ToolItemDragDropData_new(NULL);
	data->type = TOOL_ITEM_COLOR;
	data->id = id;
	data->color = color;
}

void ToolItemDragCurrentData::setData(ToolItemDragDropData * d) {
	data = g_new(ToolItemDragDropData, 1);
	*data = *d;
}

ToolItemDragDropData * ToolItemDragCurrentData::getData() {
	return data;
}
