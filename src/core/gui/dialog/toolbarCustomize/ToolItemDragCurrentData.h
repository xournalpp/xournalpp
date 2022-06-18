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

#include <gtk/gtk.h>  // for GtkWidget

#include "gui/ToolitemDragDrop.h"  // for ToolItemDragDropData (ptr only)

class AbstractToolItem;
struct NamedColor;

class ToolItemDragCurrentData {
private:
    ToolItemDragCurrentData();
    virtual ~ToolItemDragCurrentData();

public:
    static void clearData();
    static void setData(ToolItemDragDropData* d);
    static void setData(ToolItemType type, int id, AbstractToolItem* item);
    static void setData(GtkWidget* widget);
    static void setDataColor(int id, const NamedColor* namedColor);
    static ToolItemDragDropData* getData();

private:
    static ToolItemDragDropData* data;
};
