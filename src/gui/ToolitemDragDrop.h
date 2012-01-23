/*
 * Xournal++
 *
 * Helper for Toolbar Drag & Drop implementation
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __TOOLBARDRAGDROP_H__
#define __TOOLBARDRAGDROP_H__

#include <gtk/gtk.h>
#include <String.h>

class AbstractToolItem;

enum ToolItemType {
	TOOL_ITEM_SEPARATOR = 0,
	TOO_ITEM_SPACER,
	TOOL_ITEM_ITEM,
	TOOL_ITEM_COLOR
};

#define ToolItemDragDropData_Identify 0xFA090201

struct ToolItemDragDropData {
	int identify;
	ToolItemType type;
	int id;
	AbstractToolItem * item;
	int color;
};

class ToolitemDragDrop {
private:
	ToolitemDragDrop();
	virtual ~ToolitemDragDrop();

public:
	static void attachMetadata(GtkWidget * w, int id, AbstractToolItem * ait);
	static void attachMetadata(GtkWidget * w, int id, ToolItemType type);
	static void attachMetadataColor(GtkWidget * w, int id, int color, AbstractToolItem * item);

public:
	static ToolItemDragDropData * ToolItemDragDropData_new(AbstractToolItem * item);
	static bool checkToolItemDragDropData(ToolItemDragDropData * d);
	static bool isToolItemEnabled(ToolItemDragDropData * d);

	static ToolItemDragDropData * metadataGetMetadata(GtkWidget * w);

	static GtkWidget * getIcon(ToolItemDragDropData * data);

public:
	static void removeFromToolbarForMove(GtkWidget * widget);
};

#endif /* __TOOLBARDRAGDROP_H__ */
