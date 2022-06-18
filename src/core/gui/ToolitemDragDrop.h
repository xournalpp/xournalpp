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

#include <gdk-pixbuf/gdk-pixbuf.h>  // for GdkPixbuf
#include <gtk/gtk.h>                // for GtkWidget

class AbstractToolItem;
struct NamedColor;

enum ToolItemType { TOOL_ITEM_SEPARATOR = 0, TOOL_ITEM_SPACER, TOOL_ITEM_ITEM, TOOL_ITEM_COLOR };

#define ToolItemDragDropData_Identify 0xFA090201

struct ToolItemDragDropData {
    unsigned int identify;
    ToolItemType type;
    int id;
    AbstractToolItem* item;
    const NamedColor* namedColor;
};

class ToolitemDragDrop {
private:
    ToolitemDragDrop();
    virtual ~ToolitemDragDrop();

public:
    static void attachMetadata(GtkWidget* w, int id, AbstractToolItem* ait);
    static void attachMetadata(GtkWidget* w, int id, ToolItemType type);
    static void attachMetadataColor(GtkWidget* w, int id, const NamedColor* namedColor, AbstractToolItem* item);

public:
    static ToolItemDragDropData* ToolItemDragDropData_new(AbstractToolItem* item);
    static bool checkToolItemDragDropData(ToolItemDragDropData const* d);
    static bool isToolItemEnabled(ToolItemDragDropData* d);

    static ToolItemDragDropData* metadataGetMetadata(GtkWidget* w);

    /**
     * Returns: (transfer floating)
     */
    static GtkWidget* getIcon(ToolItemDragDropData* data);
    /**
     * @brief Get Pixbuf for the respective tool contained in the DragDrop data
     *
     * @param data DragDropData relevant during toolbar customization
     * @return GdkPixbuf* of the dragged tool
     */
    static GdkPixbuf* getPixbuf(ToolItemDragDropData* data);

public:
    static void removeFromToolbarForMove(GtkWidget* widget);
};
