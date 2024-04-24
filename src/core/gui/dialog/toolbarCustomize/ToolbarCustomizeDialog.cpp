#include "ToolbarCustomizeDialog.h"

#include <cstddef>  // for size_t
#include <limits>   // for numeric_l...
#include <string>   // for allocator
#include <vector>   // for vector

#include <gdk-pixbuf/gdk-pixbuf.h>  // for GdkPixbuf
#include <glib-object.h>            // for G_CALLBACK

#include "control/Control.h"            // for Control
#include "control/settings/Settings.h"  // for Settings
#include "gui/Builder.h"
#include "gui/MainWindow.h"                                 // for MainWindow
#include "gui/ToolitemDragDrop.h"                           // for ToolItemD...
#include "gui/toolbarMenubar/AbstractToolItem.h"            // for AbstractT...
#include "gui/toolbarMenubar/ToolMenuHandler.h"             // for ToolMenuH...
#include "gui/toolbarMenubar/icon/ColorIcon.h"              // for ColorIcon
#include "gui/toolbarMenubar/icon/ToolbarSeparatorImage.h"  // for getNewToo...
#include "gui/toolbarMenubar/model/ColorPalette.h"          // for Palette
#include "util/Assert.h"                                    // for xoj_assert
#include "util/Color.h"                                     // for Color
#include "util/EnumIndexedArray.h"
#include "util/GListView.h"   // for GListView
#include "util/NamedColor.h"  // for NamedColor
#include "util/i18n.h"  // for _
#include "util/raii/GObjectSPtr.h"

#include "ToolItemDragCurrentData.h"  // for ToolItemD...
#include "ToolbarDragDropHandler.h"   // for ToolbarDr...
#include "ToolbarDragDropHelper.h"    // for dragSourc...

class GladeSearchpath;

/*
 * struct used for data necessary for dragging
 * during toolbar customization
 */
struct ToolbarCustomizeDialog::ToolItemDragData {
    ToolbarCustomizeDialog* dlg;
    GtkWidget* icon;  ///< Currently must be an GtkImage
    AbstractToolItem* item;
    xoj::util::WidgetSPtr ebox;
};

struct ToolbarCustomizeDialog::ColorToolItemDragData {
    ToolbarCustomizeDialog* dlg;
    size_t paletteColorIndex;
    xoj::util::WidgetSPtr ebox;
};

// Separator and spacer
struct ToolbarCustomizeDialog::SeparatorData {
    ToolItemType type;
    SeparatorType separator;
    const char* label;
};

std::array<ToolbarCustomizeDialog::SeparatorData, 2> ToolbarCustomizeDialog::separators = {
        ToolbarCustomizeDialog::SeparatorData{TOOL_ITEM_SEPARATOR, SeparatorType::SEPARATOR, _("Separator")},
        ToolbarCustomizeDialog::SeparatorData{TOOL_ITEM_SPACER, SeparatorType::SPACER, _("Spacer")}};


constexpr auto UI_FILE = "toolbarCustomizeDialog.glade";
constexpr auto UI_DIALOG_NAME = "DialogCustomizeToolbar";

ToolbarCustomizeDialog::ToolbarCustomizeDialog(GladeSearchpath* gladeSearchPath, MainWindow* win,
                                               ToolbarDragDropHandler* handler):
        //     itemData(buildToolDataVector(win->getToolMenuHandler()->getToolItems())),
        //     colorItemData(buildColorDataVector(handler->getControl()->getPalette())),
        palette(handler->getControl()->getPalette()) {
    // Builder builder(gladeSearchPath, UI_FILE);
    // window.reset(GTK_WINDOW(builder.get(UI_DIALOG_NAME)));
    // notebook = GTK_NOTEBOOK(builder.get("notebook"));
    //
    // using Cat = AbstractToolItem::Category;
    // EnumIndexedArray<std::string, Cat> labels;
    // labels[Cat::AUDIO] = C_("Item category in toolbar customization dialog", "Audio");
    // labels[Cat::COLORS] = C_("Item category in toolbar customization dialog", "Colors");
    // labels[Cat::FILES] = C_("Item category in toolbar customization dialog", "Files");
    // labels[Cat::MISC] = C_("Item category in toolbar customization dialog", "Miscellaneous");
    // labels[Cat::NAVIGATION] = C_("Item category in toolbar customization dialog", "Navigation");
    // labels[Cat::SELECTION] = C_("Item category in toolbar customization dialog", "Selection");
    // labels[Cat::TOOLS] = C_("Item category in toolbar customization dialog", "Tools");
    // labels[Cat::SEPARATORS] = C_("Item category in toolbar customization dialog", "Separators");
    // labels[Cat::PLUGINS] = C_("Item category in toolbar customization dialog", "Plugins");
    // EnumIndexedArray<GtkWidget*, Cat> tabs;

    // for (std::underlying_type_t<Cat> n = 0; n < xoj::to_underlying(Cat::ENUMERATOR_COUNT); n++) {
    //     Cat c = static_cast<Cat>(n);
    //     tabs[c] = gtk_list_box_new();
    //     GtkWidget* w = gtk_scrolled_window_new();
    //     gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(w), tabs[c]);
    //     gtk_notebook_append_page(notebook, w, gtk_label_new(labels[c].c_str()));
    // }
    //
    // auto addEntry = [&tabs](GtkWidget* w, Cat c) {
    //     GtkWidget* row = gtk_list_box_row_new();
    //     gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), w);
    //     gtk_list_box_row_set_selectable(GTK_LIST_BOX_ROW(row), false);
    //     gtk_list_box_append(GTK_LIST_BOX(tabs[c]), row);
    // };
    //
    // for (auto& data: itemData) {
    //     addEntry(data.ebox.get(), data.item->getCategory());
    // }
    //
    // for (auto& data: colorItemData) {
    //     addEntry(data.ebox.get(), Cat::COLORS);
    // }
    //
    // // init separator and spacer
    // for (SeparatorData& data: separators) {
    //     GtkBox* box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2));
    //     gtk_box_append(box, ToolbarSeparatorImage::newImage(data.separator));
    //     gtk_box_append(box, gtk_label_new(data.label));
    //
    //     GtkWidget* ebox = gtk_event_box_new();
    //     gtk_container_add(GTK_CONTAINER(ebox), GTK_WIDGET(box));
    //     gtk_widget_show_all(ebox);
    //
    //     // make ebox a drag source
    //     gtk_drag_source_set(ebox, GDK_BUTTON1_MASK, &ToolbarDragDropHelper::dropTargetEntry, 1, GDK_ACTION_MOVE);
    //     ToolbarDragDropHelper::dragSourceAddToolbar(ebox);
    //
    //     g_signal_connect(ebox, "drag-begin", G_CALLBACK(toolitemDragBeginSeparator), &data);
    //     g_signal_connect(ebox, "drag-end", G_CALLBACK(toolitemDragEndSeparator), &data);
    //     g_signal_connect(ebox, "drag-data-get", G_CALLBACK(toolitemDragDataGetSeparator), &data);
    //
    //     addEntry(ebox, Cat::SEPARATORS);
    // }
    //
    // GtkWidget* target = GTK_WIDGET(notebook);  // builder.get("viewport1");
    // // prepare drag & drop
    // gtk_drag_dest_set(target, GTK_DEST_DEFAULT_ALL, nullptr, 0, GDK_ACTION_MOVE);
    // ToolbarDragDropHelper::dragDestAddToolbar(target);
    //
    // g_signal_connect(target, "drag-data-received", G_CALLBACK(dragDataReceived), this);
    //
    // g_signal_connect_swapped(builder.get("btClose"), "clicked", G_CALLBACK(gtk_window_close), window.get());
    // g_signal_connect_swapped(window.get(), "delete-event",
    //                          G_CALLBACK(+[](ToolbarDragDropHandler* h) { h->toolbarConfigDialogClosed(); }),
    //                          handler);
}

ToolbarCustomizeDialog::~ToolbarCustomizeDialog() = default;

// void ToolbarCustomizeDialog::toolitemDragBeginSeparator(GtkWidget* widget, GdkDragContext* context, void* data) {
//     SeparatorData* sepData = static_cast<SeparatorData*>(data);
//     ToolItemDragCurrentData::setData(sepData->type, -1, nullptr);
//     GdkPixbuf* pixbuf = ToolbarSeparatorImage::getNewToolPixbuf(sepData->separator);
//     gtk_drag_set_icon_pixbuf(context, pixbuf, -2, -2);
//     g_object_unref(pixbuf);
// }
//
// void ToolbarCustomizeDialog::toolitemDragEndSeparator(GtkWidget* widget, GdkDragContext* context, void* unused) {
//     ToolItemDragCurrentData::clearData();
// }
//
// void ToolbarCustomizeDialog::toolitemDragDataGetSeparator(GtkWidget* widget, GdkDragContext* context,
//                                                           GtkSelectionData* selection_data, guint info, guint time,
//                                                           void* data) {
//     SeparatorData* sepData = static_cast<SeparatorData*>(data);
//     auto it = ToolitemDragDrop::ToolItemDragDropData_new(nullptr);
//     it->type = sepData->type;
//
//     gtk_selection_data_set(selection_data, ToolbarDragDropHelper::atomToolItem, 0,
//                            reinterpret_cast<const guchar*>(it.get()), sizeof(ToolItemDragDropData));
// }
//
// /**
//  * Drag a Toolitem from dialog
//  */
// void ToolbarCustomizeDialog::toolitemDragBegin(GtkWidget* widget, GdkDragContext* context, ToolItemDragData* data) {
//     ToolItemDragCurrentData::setData(TOOL_ITEM_ITEM, -1, data->item);
//     if (data->icon) {
//         ToolbarDragDropHelper::gdk_context_set_icon_from_image(context, data->icon);
//     }
// }
//
// /**
//  * Drag a Toolitem from dialog STOPPED
//  */
// void ToolbarCustomizeDialog::toolitemDragEnd(GtkWidget* widget, GdkDragContext* context, ToolItemDragData* data) {
//     ToolItemDragCurrentData::clearData();
// }
//
// void ToolbarCustomizeDialog::toolitemDragDataGet(GtkWidget* widget, GdkDragContext* context,
//                                                  GtkSelectionData* selection_data, guint info, guint time,
//                                                  ToolItemDragData* data) {
//     g_return_if_fail(data != nullptr);
//     g_return_if_fail(data->item != nullptr);
//
//     auto it = ToolitemDragDrop::ToolItemDragDropData_new(data->item);
//
//     gtk_selection_data_set(selection_data, ToolbarDragDropHelper::atomToolItem, 0,
//                            reinterpret_cast<const guchar*>(it.get()), sizeof(ToolItemDragDropData));
// }
//
// /**
//  * Drag a ColorToolitem from dialog
//  */
// void ToolbarCustomizeDialog::toolitemColorDragBegin(GtkWidget* widget, GdkDragContext* context,
//                                                     ColorToolItemDragData* data) {
//     ToolItemDragCurrentData::setDataColor(-1, data->paletteColorIndex);
//
//     auto namedColor = data->dlg->palette.getColorAt(data->paletteColorIndex);
//     auto image = ColorIcon::newGdkPixbuf(namedColor.getColor(), 16, true);
//     gtk_drag_set_icon_pixbuf(context, image.get(), -2, -2);
// }
//
// /**
//  * Drag a ColorToolitem from dialog STOPPED
//  */
// void ToolbarCustomizeDialog::toolitemColorDragEnd(GtkWidget* widget, GdkDragContext* context,
//                                                   ColorToolItemDragData* data) {
//     ToolItemDragCurrentData::clearData();
// }
//
// void ToolbarCustomizeDialog::toolitemColorDragDataGet(GtkWidget* widget, GdkDragContext* context,
//                                                       GtkSelectionData* selection_data, guint info, guint time,
//                                                       ColorToolItemDragData* data) {
//     ToolItemDragCurrentData::setDataColor(-1, data->paletteColorIndex);
//
//     auto it = ToolitemDragDrop::ToolItemDragDropData_new(nullptr);
//     it->type = TOOL_ITEM_COLOR;
//     it->paletteColorIndex = data->paletteColorIndex;
//
//     gtk_selection_data_set(selection_data, ToolbarDragDropHelper::atomToolItem, 0,
//                            reinterpret_cast<const guchar*>(it.get()), sizeof(ToolItemDragDropData));
// }
//
// /**
//  * A tool item was dragged to the dialog
//  */
// void ToolbarCustomizeDialog::dragDataReceived(GtkWidget* widget, GdkDragContext* dragContext, gint x, gint y,
//                                               GtkSelectionData* data, guint info, guint time,
//                                               ToolbarCustomizeDialog* dlg) {
//     if (gtk_selection_data_get_data_type(data) != ToolbarDragDropHelper::atomToolItem) {
//         gtk_drag_finish(dragContext, false, false, time);
//         return;
//     }
//
//     auto* d = reinterpret_cast<ToolItemDragDropData const*>(gtk_selection_data_get_data(data));
//     g_return_if_fail(ToolitemDragDrop::checkToolItemDragDropData(d));
//
//     gtk_drag_finish(dragContext, true, false, time);
// }
//
// /**
//  * builds up the icon list
//  */
// auto ToolbarCustomizeDialog::buildToolDataVector(const std::vector<std::unique_ptr<AbstractToolItem>>& tools)
//         -> std::vector<ToolItemDragData> {
//     // By reserving, we ensure no reallocation is done, so the pointer `&data` used below is not invalidated
//     std::vector<ToolItemDragData> database;
//     database.reserve(tools.size());
//     for (auto&& item: tools) {
//         std::string name = item->getToolDisplayName();
//         GtkWidget* icon = item->getNewToolIcon(); /* floating */
//         xoj_assert(icon);
//
//         GtkBox* box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2));
//         gtk_box_append(box, icon);
//         gtk_box_append(box, gtk_label_new(name.c_str()));
//
//         GtkWidget* ebox = gtk_event_box_new();
//         gtk_container_add(GTK_CONTAINER(ebox), GTK_WIDGET(box));
//         gtk_widget_show_all(GTK_WIDGET(ebox));
//
//         auto& data = database.emplace_back();
//         data.dlg = this;
//         data.icon = icon;
//         data.item = item.get();
//         data.ebox.reset(ebox, xoj::util::adopt);
//
//         // make ebox a drag source
//         gtk_drag_source_set(ebox, GDK_BUTTON1_MASK, &ToolbarDragDropHelper::dropTargetEntry, 1, GDK_ACTION_MOVE);
//         ToolbarDragDropHelper::dragSourceAddToolbar(ebox);
//
//         g_signal_connect(ebox, "drag-begin", G_CALLBACK(toolitemDragBegin), &data);
//         g_signal_connect(ebox, "drag-end", G_CALLBACK(toolitemDragEnd), &data);
//         g_signal_connect(ebox, "drag-data-get", G_CALLBACK(toolitemDragDataGet), &data);
//     }
//     return database;
// }
//
// auto ToolbarCustomizeDialog::buildColorDataVector(const Palette& palette) -> std::vector<ColorToolItemDragData> {
//     // By reserving, we ensure no reallocation is done, so the pointer `&data` used below is not invalidated
//     std::vector<ColorToolItemDragData> database;
//     database.reserve(palette.size());
//     for (size_t paletteColorIndex = 0; paletteColorIndex < palette.size(); paletteColorIndex++) {
//         // namedColor needs to be a pointer to pass it into a ColorToolItemDragData
//         const NamedColor& namedColor = palette.getColorAt(paletteColorIndex);
//
//         GtkBox* box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2));
//         gtk_box_append(box, ColorIcon::newGtkImage(namedColor.getColor(), 16, true));
//         gtk_box_append(box, gtk_label_new(namedColor.getName().c_str()));
//
//         GtkWidget* ebox = gtk_event_box_new();
//         gtk_container_add(GTK_CONTAINER(ebox), GTK_WIDGET(box));
//         gtk_widget_show_all(GTK_WIDGET(ebox));
//
//         // make ebox a drag source
//         gtk_drag_source_set(ebox, GDK_BUTTON1_MASK, &ToolbarDragDropHelper::dropTargetEntry, 1, GDK_ACTION_MOVE);
//         ToolbarDragDropHelper::dragSourceAddToolbar(ebox);
//
//         auto& data = database.emplace_back();
//         data.dlg = this;
//         data.paletteColorIndex = paletteColorIndex;
//         data.ebox.reset(ebox, xoj::util::ref);
//
//         g_signal_connect(ebox, "drag-begin", G_CALLBACK(toolitemColorDragBegin), &data);
//         g_signal_connect(ebox, "drag-end", G_CALLBACK(toolitemColorDragEnd), &data);
//         g_signal_connect(ebox, "drag-data-get", G_CALLBACK(toolitemColorDragDataGet), &data);
//     }
//     return database;
// }

void ToolbarCustomizeDialog::show(GtkWindow* parent) {
    // gtk_window_set_transient_for(this->window.get(), parent);
    //
    // gtk_window_set_position(this->window.get(), GTK_WIN_POS_CENTER_ON_PARENT);
    // gtk_widget_show_all(GTK_WIDGET(this->window.get()));
}
