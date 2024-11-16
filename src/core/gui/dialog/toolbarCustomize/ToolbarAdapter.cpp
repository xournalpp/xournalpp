#include "ToolbarAdapter.h"

#include <utility>  // for move

#include <glib-object.h>  // for G_CALLBACK

#include "control/Control.h"                           // for Control
#include "control/settings/Settings.h"                 // for Settings
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

    // // prepare drag & drop
    // gtk_drag_dest_set(toolbar, GTK_DEST_DEFAULT_ALL, nullptr, 0, GDK_ACTION_MOVE);
    // ToolbarDragDropHelper::dragDestAddToolbar(toolbar);
    //
    // g_signal_connect(toolbar, "drag_motion", G_CALLBACK(toolbarDragMotionCb), this);
    // g_signal_connect(toolbar, "drag_leave", G_CALLBACK(toolbarDragLeafeCb), this);
    // g_signal_connect(toolbar, "drag_data_received", G_CALLBACK(toolbarDragDataReceivedCb), this);
    //
    // showToolbar();
    // prepareToolItems();
    //
    // gtk_widget_add_css_class(w, "editing");
}

ToolbarAdapter::~ToolbarAdapter() {
    // remove drag & drop handler
    // g_signal_handlers_disconnect_by_func(this->w, (gpointer)toolbarDragMotionCb, this);
    // g_signal_handlers_disconnect_by_func(this->w, (gpointer)toolbarDragLeafeCb, this);
    // g_signal_handlers_disconnect_by_func(this->w, (gpointer)toolbarDragDataReceivedCb, this);
    //
    // cleanupToolbars();
    //
    // gtk_widget_remove_css_class(w, "editing");
    //
    // g_object_unref(this->w);
}

/**
 * A tool item was dragged to the toolbar
 */
// auto ToolbarAdapter::toolbarDragMotionCb(GtkWidget* toolbar, GdkDragContext* context, gint x, gint y, guint time,
//                                          ToolbarAdapter* adapter) -> bool {
//     GdkAtom target = gtk_drag_dest_find_target(toolbar, context, nullptr);
//     if (target != ToolbarDragDropHelper::atomToolItem) {
//         gdk_drag_status(context, static_cast<GdkDragAction>(0), time);
//         return false;
//     }
//
//     GtkOrientation orientation = gtk_orientable_get_orientation(GTK_ORIENTABLE(toolbar));
//
//     bool horizontal = orientation == GTK_ORIENTATION_HORIZONTAL;
//     gint ipos = toolbarGetDropIndex(toolbar, x, y, horizontal);
//
//     gdk_drag_status(context, gdk_drag_context_get_suggested_action(context), time);
//
//     const ToolItemDragDropData* d = ToolItemDragCurrentData::getData();
//
//     if (d == nullptr) {
//         g_warning("ToolbarAdapter.cpp, ToolItemDragDropData == nullptr");
//         return false;
//     }
//
//     if (d->type == TOOL_ITEM_ITEM) {
//         GtkWidget* iconWidget = d->item->getNewToolIcon();
//         GtkToolItem* it = gtk_tool_button_new(iconWidget, "");
//         // gtk_toolbar_set_drop_highlight_item(toolbar, it, ipos);
//     } else if (d->type == TOOL_ITEM_SEPARATOR) {
//         GtkToolItem* it = gtk_separator_tool_item_new();
//         // gtk_toolbar_set_drop_highlight_item(toolbar, it, ipos);
//     } else if (d->type == TOOL_ITEM_SPACER) {
//         GtkToolItem* it = gtk_separator_tool_item_new();
//         gtk_tool_item_set_expand(it, true);
//         // gtk_toolbar_set_drop_highlight_item(toolbar, it, ipos);
//     } else if (d->type == TOOL_ITEM_COLOR) {
//         auto namedColor = adapter->palette.getColorAt(d->paletteColorIndex);
//         GtkWidget* iconWidget = ColorIcon::newGtkImage(namedColor.getColor(), 16, true);
//         GtkToolItem* it = gtk_tool_button_new(iconWidget, "");
//         // gtk_toolbar_set_drop_highlight_item(toolbar, it, ipos);
//     } else {
//         g_warning("ToolbarAdapter::toolbarDragMotionCb Unhandled type %i", d->type);
//     }
//
//     return true;
// }
//
// void ToolbarAdapter::toolbarDragLeafeCb(GtkWidget* toolbar, GdkDragContext* context, guint time,
//                                         ToolbarAdapter* adapter) {
//     // gtk_toolbar_set_drop_highlight_item(toolbar, nullptr, -1);
// }
//
// void ToolbarAdapter::toolbarDragDataReceivedCb(GtkWidget* toolbar, GdkDragContext* context, gint x, gint y,
//                                                GtkSelectionData* data, guint info, guint time,
//                                                ToolbarAdapter* adapter) {
//     auto* d = reinterpret_cast<ToolItemDragDropData const*>(gtk_selection_data_get_data(data));
//     g_return_if_fail(ToolitemDragDrop::checkToolItemDragDropData(d));
//
//     bool horizontal = gtk_orientable_get_orientation(GTK_ORIENTABLE(toolbar)) == GTK_ORIENTATION_HORIZONTAL;
//     gint pos = toolbarGetDropIndex(toolbar, x, y, horizontal);
//
//     if (d->type == TOOL_ITEM_ITEM) {
//         auto it = d->item->createItem(horizontal);
//
//         gtk_widget_show_all(it.get());
//         // gtk_toolbar_insert(toolbar, it, pos);
//         // adapter->prepareToolItem(it);
//
//         ToolbarData* tb = adapter->window->getSelectedToolbar();
//         const char* name = adapter->window->getToolbarName(toolbar);
//
//         string id = d->item->getId();
//
//         int newId = tb->insertItem(name, id, pos);
//         ToolitemDragDrop::attachMetadata(it.get(), newId, d->item);
//     } else if (d->type == TOOL_ITEM_COLOR) {
//         auto namedColor = adapter->palette.getColorAt(d->paletteColorIndex);
//         auto item = std::make_unique<ColorToolItem>(namedColor);
//
//         // auto it = item->createItem(horizontal);
//
//         // gtk_toolbar_insert(toolbar, GTK_TOOL_ITEM(it.get()), pos);
//         // adapter->prepareToolItem(GTK_TOOL_ITEM(it.get()));
//         //
//         // ToolbarData* tb = adapter->window->getSelectedToolbar();
//         // const char* name = adapter->window->getToolbarName(toolbar);
//         //
//         // string id = item->getId();
//         //
//         // int newId = tb->insertItem(name, id, pos);
//         // ToolitemDragDrop::attachMetadataColor(it.get(), newId, d->paletteColorIndex, item.get());
//
//         adapter->window->getToolMenuHandler()->addColorToolItem(std::move(item));
//     } else if (d->type == TOOL_ITEM_SEPARATOR) {
//         GtkToolItem* it = gtk_separator_tool_item_new();
//         gtk_widget_show_all(GTK_WIDGET(it));
//         // gtk_toolbar_insert(toolbar, it, pos);
//         adapter->prepareToolItem(it);
//
//         ToolbarData* tb = adapter->window->getSelectedToolbar();
//         const char* name = adapter->window->getToolbarName(toolbar);
//
//         int newId = tb->insertItem(name, "SEPARATOR", pos);
//         ToolitemDragDrop::attachMetadata(GTK_WIDGET(it), newId, TOOL_ITEM_SEPARATOR);
//     } else if (d->type == TOOL_ITEM_SPACER) {
//         GtkToolItem* it = gtk_separator_tool_item_new();
//         gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(it), false);
//         gtk_tool_item_set_expand(it, true);
//         gtk_widget_show_all(GTK_WIDGET(it));
//         // gtk_toolbar_insert(toolbar, it, pos);
//         adapter->prepareToolItem(it);
//
//         ToolbarData* tb = adapter->window->getSelectedToolbar();
//         const char* name = adapter->window->getToolbarName(toolbar);
//
//         int newId = tb->insertItem(name, "SPACER", pos);
//         ToolitemDragDrop::attachMetadata(GTK_WIDGET(it), newId, TOOL_ITEM_SPACER);
//     } else {
//         g_warning("toolbarDragDataReceivedCb: ToolItemType %i not handled!", d->type);
//     }
// }
//
// auto ToolbarAdapter::toolbarGetDropIndex(GtkWidget* toolbar, gint x, gint y, bool horizontal) -> gint {
//     return 0;
//     /*
//      * gtk_toolbar_get_drop_index does not use the correct coordinate system for the y coordinate
//      * since the y coordinate is only relevant in vertical toolbars the transformation is NOT required for horizontal
//      * toolbars
//      */
//     // if (horizontal) {
//     //     return gtk_toolbar_get_drop_index(toolbar, x, y);
//     // } else {
//     //     gint wx = 0;
//     //     gint wy = 0;
//     //     gtk_widget_translate_coordinates(GTK_WIDGET(toolbar), gtk_widget_get_toplevel(GTK_WIDGET(toolbar)), x, y,
//     //     &wx,
//     //                                      &wy);
//     //     return gtk_toolbar_get_drop_index(toolbar, x, wy);
//     // }
// }
