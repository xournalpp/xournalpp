/*
 * Xournal++
 *
 * Toolbar drag & drop helper class
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "../../ToolitemDragDrop.h"
#include <gtk/gtk.h>

class ToolItemDragCurrentData
{
private:
	ToolItemDragCurrentData();
	virtual ~ToolItemDragCurrentData();

public:
	static void clearData();
	static void setData(ToolItemDragDropData* d);
	static void setData(ToolItemType type, int id, AbstractToolItem* item);
	static void setData(GtkWidget* widget);
	static void setDataColor(int id, int color);
	static ToolItemDragDropData* getData();

private:
	static ToolItemDragDropData* data;
};
