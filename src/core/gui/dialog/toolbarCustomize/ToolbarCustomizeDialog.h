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

#include <array>
#include <memory>
#include <vector>

#include <gdk/gdk.h>  // for GdkDragContext
#include <glib.h>     // for guint, gint
#include <gtk/gtk.h>  // for GtkWidget, GtkSelectionData, GtkToolbar

#include "util/raii/GtkWindowUPtr.h"

class AbstractToolItem;
struct Palette;
class MainWindow;
class ToolbarDragDropHandler;
class GladeSearchpath;

class ToolbarCustomizeDialog {
public:
    ToolbarCustomizeDialog(GladeSearchpath* gladeSearchPath, MainWindow* win, ToolbarDragDropHandler* handler);
    ~ToolbarCustomizeDialog();

public:
    inline GtkWindow* getWindow() const { return window.get(); }

    void show(GtkWindow* parent);

private:
    struct ToolItemDragData;
    struct ColorToolItemDragData;
    struct SeparatorData;

    std::vector<ToolItemDragData> buildToolDataVector(const std::vector<std::unique_ptr<AbstractToolItem>>& tools);
    std::vector<ColorToolItemDragData> buildColorDataVector(const Palette& palette);

    // void rebuildIconview();


private:
    // static void dragDataReceived(GtkWidget* widget, GdkDragContext* dragContext, gint x, gint y, GtkSelectionData*
    // data,
    //                              guint info, guint time, ToolbarCustomizeDialog* dlg);
    // static void toolbarDragLeafeCb(GtkToolbar* toolbar, GdkDragContext* context, guint time,
    //                                ToolbarCustomizeDialog* dlg);
    // static void toolbarDragDataReceivedCb(GtkToolbar* toolbar, GdkDragContext* context, gint x, gint y,
    //                                       GtkSelectionData* data, guint info, guint time, ToolbarCustomizeDialog*
    //                                       dlg);
    //
    // static void toolitemDragBegin(GtkWidget* widget, GdkDragContext* context, ToolItemDragData* data);
    // static void toolitemDragEnd(GtkWidget* widget, GdkDragContext* context, ToolItemDragData* data);
    // static void toolitemDragDataGet(GtkWidget* widget, GdkDragContext* context, GtkSelectionData* selection_data,
    //                                 guint info, guint time, ToolItemDragData* data);
    //
    // static void toolitemColorDragBegin(GtkWidget* widget, GdkDragContext* context, ColorToolItemDragData* data);
    // static void toolitemColorDragEnd(GtkWidget* widget, GdkDragContext* context, ColorToolItemDragData* data);
    // static void toolitemColorDragDataGet(GtkWidget* widget, GdkDragContext* context, GtkSelectionData*
    // selection_data,
    //                                      guint info, guint time, ColorToolItemDragData* data);
    //
    // static void toolitemDragBeginSeparator(GtkWidget* widget, GdkDragContext* context, void* unused);
    // static void toolitemDragEndSeparator(GtkWidget* widget, GdkDragContext* context, void* unused);
    // static void toolitemDragDataGetSeparator(GtkWidget* widget, GdkDragContext* context,
    //                                          GtkSelectionData* selection_data, guint info, guint time, void* unused);

    // void freeIconview();

private:
    /**
     * @brief Stores the widget and other data associated to each drag-able toolbar item
     * Modifications are not allowed to avoid vector reallocation and pointer invalidation
     * (pointers are feed to g_signal's callback data)
     */
    const std::vector<ToolItemDragData> itemData;
    /**
     * @brief Stores the widget and other data associated to each drag-able color item
     */
    const std::vector<ColorToolItemDragData> colorItemData;
    static std::array<SeparatorData, 2> separators;

    xoj::util::GtkWindowUPtr window;
    GtkNotebook* notebook;
};
