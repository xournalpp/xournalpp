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
#include "gui/toolbarMenubar/AbstractToolItem.h"            // for AbstractT...
#include "gui/toolbarMenubar/ToolMenuHandler.h"             // for ToolMenuH...
#include "gui/toolbarMenubar/icon/ColorIcon.h"              // for ColorIcon
#include "gui/toolbarMenubar/icon/ToolbarSeparatorImage.h"  // for getNewToo...
#include "gui/toolbarMenubar/model/ColorPalette.h"          // for Palette
#include "util/Assert.h"                                    // for xoj_assert
#include "util/Color.h"                                     // for Color
#include "util/EnumIndexedArray.h"
#include "util/NamedColor.h"  // for NamedColor
#include "util/i18n.h"        // for _
#include "util/raii/GObjectSPtr.h"

#include "ToolItemGType.h"
#include "ToolbarDragDropHandler.h"  // for ToolbarDr...

class GladeSearchpath;

constexpr auto UI_FILE = "toolbarCustomizeDialog.ui";
constexpr auto UI_DIALOG_NAME = "DialogCustomizeToolbar";

ToolbarCustomizeDialog::ToolbarCustomizeDialog(GladeSearchpath* gladeSearchPath, MainWindow* win,
                                               ToolbarDragDropHandler* handler):
        handler(handler), palette(handler->getControl()->getPalette()) {
    Builder builder(gladeSearchPath, UI_FILE);
    window.reset(GTK_WINDOW(builder.get(UI_DIALOG_NAME)));
    notebook = GTK_NOTEBOOK(builder.get("notebook"));

    using Cat = AbstractToolItem::Category;
    EnumIndexedArray<std::string, Cat> labels;
    labels[Cat::AUDIO] = C_("Item category in toolbar customization dialog", "Audio");
    labels[Cat::COLORS] = C_("Item category in toolbar customization dialog", "Colors");
    labels[Cat::FILES] = C_("Item category in toolbar customization dialog", "Files");
    labels[Cat::MISC] = C_("Item category in toolbar customization dialog", "Miscellaneous");
    labels[Cat::NAVIGATION] = C_("Item category in toolbar customization dialog", "Navigation");
    labels[Cat::SELECTION] = C_("Item category in toolbar customization dialog", "Selection");
    labels[Cat::TOOLS] = C_("Item category in toolbar customization dialog", "Tools");
    labels[Cat::SEPARATORS] = C_("Item category in toolbar customization dialog", "Separators");
    labels[Cat::PLUGINS] = C_("Item category in toolbar customization dialog", "Plugins");
    EnumIndexedArray<GtkWidget*, Cat> tabs;

    for (std::underlying_type_t<Cat> n = 0; n < xoj::to_underlying(Cat::ENUMERATOR_COUNT); n++) {
        Cat c = static_cast<Cat>(n);
        tabs[c] = gtk_list_box_new();
        GtkWidget* w = gtk_scrolled_window_new();
        gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(w), tabs[c]);
        gtk_notebook_append_page(notebook, w, gtk_label_new(labels[c].c_str()));
    }

    auto addEntry = [&tabs](std::string name, GtkWidget* icon, const char* id, Cat c) {
        xoj_assert(icon && id && !name.empty());

        GtkBox* box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2));
        gtk_box_append(box, icon);
        gtk_box_append(box, gtk_label_new(name.c_str()));

        GtkDragSource* source = gtk_drag_source_new();
        gtk_drag_source_set_actions(source, GDK_ACTION_COPY);
        gtk_image_set_pixel_size(GTK_IMAGE(icon), 24);
        gtk_drag_source_set_icon(source, gtk_widget_paintable_new(icon), 12, 12);

        xoj::util::GObjectSPtr<GdkContentProvider> prov(
                gdk_content_provider_new_typed(xoj::dnd::get_tool_item_gtype(), id), xoj::util::adopt);
        xoj_assert(prov);
        gtk_drag_source_set_content(source, prov.get());

        gtk_widget_add_controller(GTK_WIDGET(box), GTK_EVENT_CONTROLLER(source));

        GtkWidget* row = gtk_list_box_row_new();
        gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), GTK_WIDGET(box));
        gtk_list_box_row_set_selectable(GTK_LIST_BOX_ROW(row), false);
        gtk_list_box_append(GTK_LIST_BOX(tabs[c]), row);
    };

    for (auto&& item: win->getToolMenuHandler()->getToolItems()) {
        addEntry(item->getToolDisplayName(), item->getNewToolIcon(), item->getId().c_str(), item->getCategory());
    }

    const Palette& palette = handler->getControl()->getPalette();
    for (size_t i = 0; i < palette.size(); i++) {
        const NamedColor& namedColor = palette.getColorAt(i);
        std::string id = "COLOR(" + std::to_string(i) + ")";
        addEntry(namedColor.getName(), ColorIcon::newGtkImage(namedColor.getColor(), true), id.c_str(), Cat::COLORS);
    }

    addEntry(_("Separator"), ToolbarSeparatorImage::newImage(SeparatorType::SEPARATOR), "SEPARATOR", Cat::SEPARATORS);
    addEntry(_("Spacer"), ToolbarSeparatorImage::newImage(SeparatorType::SPACER), "SPACER", Cat::SEPARATORS);


    GtkDropTarget* target = gtk_drop_target_new(xoj::dnd::get_tool_item_gtype(), GDK_ACTION_MOVE);
    g_signal_connect(target, "drop", G_CALLBACK(+[](GtkDropTarget* t, const GValue* v, double, double, gpointer) {
                         if (GdkDrop* d = gtk_drop_target_get_drop(t)) {
                             if (gdk_drop_get_drag(d) && G_VALUE_HOLDS(v, xoj::dnd::get_tool_item_gtype())) {
                                 // This is an in-app drap-n-drop. We only accept those
                                 return true;  // Just accept the drop so the toolbar can remove the corresponding item
                             }
                         }
                         return false;
                     }),
                     nullptr);
    gtk_widget_add_controller(GTK_WIDGET(window.get()), GTK_EVENT_CONTROLLER(target));

    g_signal_connect_swapped(builder.get("btClose"), "clicked", G_CALLBACK(gtk_window_close), window.get());
}

ToolbarCustomizeDialog::~ToolbarCustomizeDialog() { handler->toolbarConfigDialogClosed(); }
