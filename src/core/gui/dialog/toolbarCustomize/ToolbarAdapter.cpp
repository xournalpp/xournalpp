#include "ToolbarAdapter.h"

#include <utility>  // for move

#include <glib-object.h>  // for G_CALLBACK

#include "control/Control.h"                           // for Control
#include "gui/MainWindow.h"                            // for MainWindow
#include "gui/ToolitemDragDrop.h"                      // for ToolItemDragDr...
#include "gui/toolbarMenubar/AbstractToolItem.h"       // for AbstractToolItem
#include "gui/toolbarMenubar/ColorToolItem.h"          // for ColorToolItem
#include "gui/toolbarMenubar/ToolMenuHandler.h"        // for ToolMenuHandler
#include "gui/toolbarMenubar/icon/ColorIcon.h"         // for ColorIcon
#include "gui/toolbarMenubar/model/ToolbarData.h"      // for ToolbarData
#include "util/Assert.h"                               // for xoj_assert
#include "util/NamedColor.h"                           // for NamedColor
#include "util/PlaceholderString.h"                    // for PlaceholderString
#include "util/gtk4_helper.h"
#include "util/i18n.h"

#include "ToolItemDragCurrentData.h"  // for ToolItemDragCu...
#include "ToolbarDragDropHelper.h"    // for dragDestAddToo...

using std::string;

ToolbarAdapter::ToolbarAdapter(GtkWidget* toolbar, string toolbarName, ToolMenuHandler* toolHandler,
                               MainWindow* window):
        palette(toolHandler->getControl()->getPalette()) {
    this->w = toolbar;
    g_object_ref(this->w);
    this->toolbarName = std::move(toolbarName);
    this->window = window;

    // prepare drag & drop
    gtk_drag_dest_set(toolbar, GTK_DEST_DEFAULT_ALL, nullptr, 0, GDK_ACTION_MOVE);
    ToolbarDragDropHelper::dragDestAddToolbar(toolbar);

    g_signal_connect(toolbar, "drag_motion", G_CALLBACK(toolbarDragMotionCb), this);
    g_signal_connect(toolbar, "drag_leave", G_CALLBACK(toolbarDragLeafeCb), this);
    g_signal_connect(toolbar, "drag_data_received", G_CALLBACK(toolbarDragDataReceivedCb), this);

    showToolbar();
    prepareToolItems();

    gtk_widget_add_css_class(w, "editing");
}

ToolbarAdapter::~ToolbarAdapter() {
    // remove drag & drop handler
    g_signal_handlers_disconnect_by_func(this->w, (gpointer)toolbarDragMotionCb, this);
    g_signal_handlers_disconnect_by_func(this->w, (gpointer)toolbarDragLeafeCb, this);
    g_signal_handlers_disconnect_by_func(this->w, (gpointer)toolbarDragDataReceivedCb, this);

    cleanupToolbars();

    gtk_widget_remove_css_class(w, "editing");

    g_object_unref(this->w);
}

void ToolbarAdapter::cleanupToolbars() {
    GtkToolbar* tb = GTK_TOOLBAR(this->w);
    if (gtk_toolbar_get_n_items(tb) == 0) {
        gtk_widget_hide(this->w);
    } else {
        for (int i = 0; i < gtk_toolbar_get_n_items(tb); i++) {
            GtkToolItem* it = gtk_toolbar_get_nth_item(tb, i);
            cleanToolItem(it);
        }
    }
}

void ToolbarAdapter::prepareToolItems() {
    GtkToolbar* tb = GTK_TOOLBAR(this->w);

    for (int i = 0; i < gtk_toolbar_get_n_items(tb); i++) {
        GtkToolItem* it = gtk_toolbar_get_nth_item(tb, i);
        prepareToolItem(it);
    }
}

void ToolbarAdapter::cleanToolItem(GtkToolItem* it) {
    GdkWindow* window = gtk_widget_get_window(GTK_WIDGET(it));

    if (window) {
        gdk_window_set_cursor(window, nullptr);
    }

    gtk_tool_item_set_use_drag_window(it, false);
    gtk_drag_source_unset(GTK_WIDGET(it));

    g_signal_handlers_disconnect_by_func(it, (gpointer)toolitemDragBegin, nullptr);
    g_signal_handlers_disconnect_by_func(it, (gpointer)toolitemDragEnd, nullptr);
    g_signal_handlers_disconnect_by_func(it, (gpointer)toolitemDragDataGet, this);
}

void ToolbarAdapter::prepareToolItem(GtkToolItem* it) {
    gtk_tool_item_set_use_drag_window(it, true);

    /*
     * Set cursor of drag drop to hand. Note: the tool item must be realized for
     * this to work!
     */
    {
        gtk_widget_realize(GTK_WIDGET(it));
        GdkDisplay* display = gtk_widget_get_display(GTK_WIDGET(it));
        GdkCursor* cursor = gdk_cursor_new_for_display(display, GDK_HAND2);
        xoj_assert(cursor);
        GdkWindow* window = gtk_widget_get_window(GTK_WIDGET(it));
        xoj_assert(window);
        gdk_window_set_cursor(window, cursor);
        g_object_unref(cursor);
    }

    gtk_drag_source_set(GTK_WIDGET(it), GDK_BUTTON1_MASK, &ToolbarDragDropHelper::dropTargetEntry, 1, GDK_ACTION_MOVE);
    ToolbarDragDropHelper::dragSourceAddToolbar(GTK_WIDGET(it));

    g_signal_connect(it, "drag-begin", G_CALLBACK(toolitemDragBegin), nullptr);
    g_signal_connect(it, "drag-end", G_CALLBACK(toolitemDragEnd), nullptr);
    g_signal_connect(it, "drag-data-get", G_CALLBACK(toolitemDragDataGet), this);
}

void ToolbarAdapter::showToolbar() {
    // force the toolbar to be shown even if empty
    gtk_widget_show(this->w);
}

/**
 * Drag a Toolitem from toolbar
 */
void ToolbarAdapter::toolitemDragBegin(GtkWidget* widget, GdkDragContext* context, void* unused) {
    ToolItemDragDropData* data = ToolitemDragDrop::metadataGetMetadata(widget);
    g_return_if_fail(data != nullptr);
    ToolItemDragCurrentData::setData(data);

    auto* icon = ToolitemDragDrop::getIcon(data);
    g_object_ref_sink(icon);
    ToolbarDragDropHelper::gdk_context_set_icon_from_image(context, icon);
    g_object_unref(icon);

    gtk_widget_hide(widget);
}

/**
 * Drag a Toolitem from toolbar STOPPED
 */
void ToolbarAdapter::toolitemDragEnd(GtkWidget* widget, GdkDragContext* context, void* unused) {
    ToolItemDragCurrentData::clearData();
    gtk_widget_show(widget);
}

/**
 * Remove a toolbar item from the tool where it was
 */
void ToolbarAdapter::removeFromToolbar(AbstractToolItem* item, const string& toolbarName, int id) {
    ToolbarData* d = this->window->getSelectedToolbar();
    if (d->removeItemByID(toolbarName, id)) {
        if (item != nullptr) {
            g_message(
                    "%s",
                    FS(_F("Removed tool item {1} from Toolbar {2} ID {3}") % item->getId() % toolbarName % id).c_str());
        } else {
            g_message("%s", FS(_F("Removed tool item from Toolbar {1} ID {2}") % toolbarName % id).c_str());
        }
    } else {
        if (item != nullptr) {
            g_message("%s", FS(_F("Could not remove tool item {1} from Toolbar {2} on position {3}") % item->getId() %
                               toolbarName % id)
                                    .c_str());
        } else {
            g_message("%s",
                      FS(_F("Could not remove tool item from Toolbar {1} on position {2}") % toolbarName % id).c_str());
        }
    }
}

void ToolbarAdapter::toolitemDragDataGet(GtkWidget* widget, GdkDragContext* context, GtkSelectionData* selection_data,
                                         guint info, guint time, ToolbarAdapter* adapter) {
    ToolItemDragDropData* data = ToolitemDragDrop::metadataGetMetadata(widget);

    g_return_if_fail(data != nullptr);

    // GtkToolbar* tb = GTK_TOOLBAR(adapter->w);
    int position = -1;
    // for (int i = 0; i < gtk_toolbar_get_n_items(tb); i++) {
    //     GtkToolItem* it = gtk_toolbar_get_nth_item(tb, i);
    //
    //     if (static_cast<void*>(it) == static_cast<void*>(widget)) {
    //         adapter->cleanToolItem(it);
    //         gtk_container_remove(GTK_CONTAINER(tb), GTK_WIDGET(it));
    //         position = i;
    //         break;
    //     }
    // }

    g_return_if_fail(position != -1);

    adapter->removeFromToolbar(data->item, adapter->toolbarName, data->id);

    gtk_selection_data_set(selection_data, ToolbarDragDropHelper::atomToolItem, 0,
                           reinterpret_cast<const guchar*>(data), sizeof(ToolItemDragDropData));
}

/**
 * A tool item was dragged to the toolbar
 */
auto ToolbarAdapter::toolbarDragMotionCb(GtkWidget* toolbar, GdkDragContext* context, gint x, gint y, guint time,
                                         ToolbarAdapter* adapter) -> bool {
    GdkAtom target = gtk_drag_dest_find_target(toolbar, context, nullptr);
    if (target != ToolbarDragDropHelper::atomToolItem) {
        gdk_drag_status(context, static_cast<GdkDragAction>(0), time);
        return false;
    }

    GtkOrientation orientation = gtk_orientable_get_orientation(GTK_ORIENTABLE(toolbar));

    bool horizontal = orientation == GTK_ORIENTATION_HORIZONTAL;
    gint ipos = toolbarGetDropIndex(toolbar, x, y, horizontal);

    gdk_drag_status(context, gdk_drag_context_get_suggested_action(context), time);

    const ToolItemDragDropData* d = ToolItemDragCurrentData::getData();

    if (d == nullptr) {
        g_warning("ToolbarAdapter.cpp, ToolItemDragDropData == nullptr");
        return false;
    }

    if (d->type == TOOL_ITEM_ITEM) {
        GtkWidget* iconWidget = d->item->getNewToolIcon();
        GtkToolItem* it = gtk_tool_button_new(iconWidget, "");
        // gtk_toolbar_set_drop_highlight_item(toolbar, it, ipos);
    } else if (d->type == TOOL_ITEM_SEPARATOR) {
        GtkToolItem* it = gtk_separator_tool_item_new();
        // gtk_toolbar_set_drop_highlight_item(toolbar, it, ipos);
    } else if (d->type == TOOL_ITEM_SPACER) {
        GtkToolItem* it = gtk_separator_tool_item_new();
        gtk_tool_item_set_expand(it, true);
        // gtk_toolbar_set_drop_highlight_item(toolbar, it, ipos);
    } else if (d->type == TOOL_ITEM_COLOR) {
        auto namedColor = adapter->palette.getColorAt(d->paletteColorIndex);
        GtkWidget* iconWidget = ColorIcon::newGtkImage(namedColor.getColor(), 16, true);
        GtkToolItem* it = gtk_tool_button_new(iconWidget, "");
        // gtk_toolbar_set_drop_highlight_item(toolbar, it, ipos);
    } else {
        g_warning("ToolbarAdapter::toolbarDragMotionCb Unhandled type %i", d->type);
    }

    return true;
}

void ToolbarAdapter::toolbarDragLeafeCb(GtkWidget* toolbar, GdkDragContext* context, guint time,
                                        ToolbarAdapter* adapter) {
    // gtk_toolbar_set_drop_highlight_item(toolbar, nullptr, -1);
}

void ToolbarAdapter::toolbarDragDataReceivedCb(GtkWidget* toolbar, GdkDragContext* context, gint x, gint y,
                                               GtkSelectionData* data, guint info, guint time,
                                               ToolbarAdapter* adapter) {
    auto* d = reinterpret_cast<ToolItemDragDropData const*>(gtk_selection_data_get_data(data));
    g_return_if_fail(ToolitemDragDrop::checkToolItemDragDropData(d));

    bool horizontal = gtk_orientable_get_orientation(GTK_ORIENTABLE(toolbar)) == GTK_ORIENTATION_HORIZONTAL;
    gint pos = toolbarGetDropIndex(toolbar, x, y, horizontal);

    if (d->type == TOOL_ITEM_ITEM) {
        auto it = d->item->createItem(horizontal);

        gtk_widget_show_all(it.get());
        // gtk_toolbar_insert(toolbar, it, pos);
        // adapter->prepareToolItem(it);

        ToolbarData* tb = adapter->window->getSelectedToolbar();
        const char* name = adapter->window->getToolbarName(toolbar);

        string id = d->item->getId();

        int newId = tb->insertItem(name, id, pos);
        ToolitemDragDrop::attachMetadata(it.get(), newId, d->item);
    } else if (d->type == TOOL_ITEM_COLOR) {
        auto namedColor = adapter->palette.getColorAt(d->paletteColorIndex);
        auto item = std::make_unique<ColorToolItem>(namedColor);

        // auto it = item->createItem(horizontal);

        // gtk_toolbar_insert(toolbar, GTK_TOOL_ITEM(it.get()), pos);
        // adapter->prepareToolItem(GTK_TOOL_ITEM(it.get()));
        //
        // ToolbarData* tb = adapter->window->getSelectedToolbar();
        // const char* name = adapter->window->getToolbarName(toolbar);
        //
        // string id = item->getId();
        //
        // int newId = tb->insertItem(name, id, pos);
        // ToolitemDragDrop::attachMetadataColor(it.get(), newId, d->paletteColorIndex, item.get());

        adapter->window->getToolMenuHandler()->addColorToolItem(std::move(item));
    } else if (d->type == TOOL_ITEM_SEPARATOR) {
        GtkToolItem* it = gtk_separator_tool_item_new();
        gtk_widget_show_all(GTK_WIDGET(it));
        // gtk_toolbar_insert(toolbar, it, pos);
        adapter->prepareToolItem(it);

        ToolbarData* tb = adapter->window->getSelectedToolbar();
        const char* name = adapter->window->getToolbarName(toolbar);

        int newId = tb->insertItem(name, "SEPARATOR", pos);
        ToolitemDragDrop::attachMetadata(GTK_WIDGET(it), newId, TOOL_ITEM_SEPARATOR);
    } else if (d->type == TOOL_ITEM_SPACER) {
        GtkToolItem* it = gtk_separator_tool_item_new();
        gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(it), false);
        gtk_tool_item_set_expand(it, true);
        gtk_widget_show_all(GTK_WIDGET(it));
        // gtk_toolbar_insert(toolbar, it, pos);
        adapter->prepareToolItem(it);

        ToolbarData* tb = adapter->window->getSelectedToolbar();
        const char* name = adapter->window->getToolbarName(toolbar);

        int newId = tb->insertItem(name, "SPACER", pos);
        ToolitemDragDrop::attachMetadata(GTK_WIDGET(it), newId, TOOL_ITEM_SPACER);
    } else {
        g_warning("toolbarDragDataReceivedCb: ToolItemType %i not handled!", d->type);
    }
}

auto ToolbarAdapter::toolbarGetDropIndex(GtkWidget* toolbar, gint x, gint y, bool horizontal) -> gint {
    return 0;
    /*
     * gtk_toolbar_get_drop_index does not use the correct coordinate system for the y coordinate
     * since the y coordinate is only relevant in vertical toolbars the transformation is NOT required for horizontal
     * toolbars
     */
    // if (horizontal) {
    //     return gtk_toolbar_get_drop_index(toolbar, x, y);
    // } else {
    //     gint wx = 0;
    //     gint wy = 0;
    //     gtk_widget_translate_coordinates(GTK_WIDGET(toolbar), gtk_widget_get_toplevel(GTK_WIDGET(toolbar)), x, y,
    //     &wx,
    //                                      &wy);
    //     return gtk_toolbar_get_drop_index(toolbar, x, wy);
    // }
}
