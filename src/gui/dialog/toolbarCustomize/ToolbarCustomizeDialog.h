/*
 * Xournal++
 *
 * Toolbar edit dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <list>
#include <string>
#include <vector>

#include "gui/GladeGui.h"
#include "gui/toolbarMenubar/model/ColorPalette.h"

class AbstractToolItem;
class MainWindow;
class ToolbarDragDropHandler;

typedef struct _ToolItemDragData ToolItemDragData;
struct _ToolItemDragData;
typedef struct _ColorToolItemDragData ColorToolItemDragData;
struct _ColorToolItemDragData;

class ToolbarCustomizeDialog: public GladeGui {
public:
    ToolbarCustomizeDialog(GladeSearchpath* gladeSearchPath, MainWindow* win, ToolbarDragDropHandler* handler);
    virtual ~ToolbarCustomizeDialog();

public:
    virtual void show(GtkWindow* parent);

    void rebuildIconview();
    void rebuildColorIcons();

private:
    static void dragDataReceived(GtkWidget* widget, GdkDragContext* dragContext, gint x, gint y, GtkSelectionData* data,
                                 guint info, guint time, ToolbarCustomizeDialog* dlg);
    static void toolbarDragLeafeCb(GtkToolbar* toolbar, GdkDragContext* context, guint time,
                                   ToolbarCustomizeDialog* dlg);
    static void toolbarDragDataReceivedCb(GtkToolbar* toolbar, GdkDragContext* context, gint x, gint y,
                                          GtkSelectionData* data, guint info, guint time, ToolbarCustomizeDialog* dlg);

    static void toolitemDragBegin(GtkWidget* widget, GdkDragContext* context, ToolItemDragData* data);
    static void toolitemDragEnd(GtkWidget* widget, GdkDragContext* context, ToolItemDragData* data);
    static void toolitemDragDataGet(GtkWidget* widget, GdkDragContext* context, GtkSelectionData* selection_data,
                                    guint info, guint time, ToolItemDragData* data);

    static void toolitemColorDragBegin(GtkWidget* widget, GdkDragContext* context, ColorToolItemDragData* data);
    static void toolitemColorDragEnd(GtkWidget* widget, GdkDragContext* context, ColorToolItemDragData* data);
    static void toolitemColorDragDataGet(GtkWidget* widget, GdkDragContext* context, GtkSelectionData* selection_data,
                                         guint info, guint time, ColorToolItemDragData* data);

    static void toolitemDragBeginSeparator(GtkWidget* widget, GdkDragContext* context, void* unused);
    static void toolitemDragEndSeparator(GtkWidget* widget, GdkDragContext* context, void* unused);
    static void toolitemDragDataGetSeparator(GtkWidget* widget, GdkDragContext* context,
                                             GtkSelectionData* selection_data, guint info, guint time, void* unused);

    void freeIconview();
    void freeColorIconview();

private:
    static void windowResponseCb(GtkDialog* dialog, int response, ToolbarCustomizeDialog* dlg);

private:
    std::list<ToolItemDragData*> itemDatalist;

    MainWindow* win;

    ToolbarDragDropHandler* handler;
};
