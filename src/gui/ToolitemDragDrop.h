/*
 * Xournal++
 *
 * Helper for Toolbar Drag & Drop implementation
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <StringUtils.h>

#include <gtk/gtk.h>

class AbstractToolItem;

enum ToolItemType
{
	TOOL_ITEM_SEPARATOR = 0,
	TOO_ITEM_SPACER,
	TOOL_ITEM_ITEM,
	TOOL_ITEM_COLOR
};

#define ToolItemDragDropData_Identify 0xFA090201

struct ToolItemDragDropData
{
	int identify;
	ToolItemType type;
	int id;
	AbstractToolItem* item;
	int color;
};

class ToolitemDragDrop
{
private:
	ToolitemDragDrop();
	virtual ~ToolitemDragDrop();

public:
	static void attachMetadata(GtkWidget* w, int id, AbstractToolItem* ait);
	static void attachMetadata(GtkWidget* w, int id, ToolItemType type);
	static void attachMetadataColor(GtkWidget* w, int id, int color, AbstractToolItem* item);

public:
	static ToolItemDragDropData* ToolItemDragDropData_new(AbstractToolItem* item);
	static bool checkToolItemDragDropData(ToolItemDragDropData* d);
	static bool isToolItemEnabled(ToolItemDragDropData* d);

	static ToolItemDragDropData* metadataGetMetadata(GtkWidget* w);

	static GtkWidget* getIcon(ToolItemDragDropData* data);

public:
	static void removeFromToolbarForMove(GtkWidget* widget);
};
