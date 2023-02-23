#include "ToolbarCustomizeDialog.h"

#include <cstddef>  // for size_t
#include <limits>   // for numeric_l...
#include <string>   // for allocator
#include <vector>   // for vector

#include <gdk-pixbuf/gdk-pixbuf.h>  // for GdkPixbuf
#include <glib-object.h>            // for G_CALLBACK

#include "control/Control.h"                                // for Control
#include "control/settings/Settings.h"                      // for Settings
#include "gui/MainWindow.h"                                 // for MainWindow
#include "gui/ToolitemDragDrop.h"                           // for ToolItemD...
#include "gui/toolbarMenubar/AbstractToolItem.h"            // for AbstractT...
#include "gui/toolbarMenubar/ToolMenuHandler.h"             // for ToolMenuH...
#include "gui/toolbarMenubar/icon/ColorSelectImage.h"       // for ColorSele...
#include "gui/toolbarMenubar/icon/ToolbarSeparatorImage.h"  // for getNewToo...
#include "gui/toolbarMenubar/model/ColorPalette.h"          // for Palette
#include "util/Color.h"                                     // for Color
#include "util/GListView.h"                                 // for GListView
#include "util/NamedColor.h"                                // for NamedColor
#include "util/i18n.h"                                      // for _

#include "ToolItemDragCurrentData.h"  // for ToolItemD...
#include "ToolbarDragDropHandler.h"   // for ToolbarDr...
#include "ToolbarDragDropHelper.h"    // for dragSourc...

class GladeSearchpath;

/*
 * struct used for data necessary for dragging
 * during toolbar customization
 */
struct _ToolItemDragData {
    ToolbarCustomizeDialog* dlg;
    GtkWidget* icon;  ///< Currently must be an GtkImage
    AbstractToolItem* item;
    GtkWidget* ebox;
};

struct _ColorToolItemDragData {
    ToolbarCustomizeDialog* dlg;
    GdkPixbuf* icon;
    const NamedColor* namedColor;
    GtkWidget* ebox;
};

// Separator and spacer
struct _SeparatorData {
    ToolItemType type;
    int pos;
    SeparatorType separator;
    const char* label;
};

SeparatorData dataSeparator = {TOOL_ITEM_SEPARATOR, 0, SeparatorType::SEPARATOR, _("Separator")};
SeparatorData dataSpacer = {TOOL_ITEM_SPACER, 1, SeparatorType::SPACER, _("Spacer")};

ToolbarCustomizeDialog::ToolbarCustomizeDialog(GladeSearchpath* gladeSearchPath, MainWindow* win,
                                               ToolbarDragDropHandler* handler):
        GladeGui(gladeSearchPath, "toolbarCustomizeDialog.glade", "DialogCustomizeToolbar") {
    this->win = win;
    this->handler = handler;

    rebuildIconview();
    rebuildColorIcons();

    GtkWidget* target = get("viewport1");

    // prepare drag & drop
    gtk_drag_dest_set(target, GTK_DEST_DEFAULT_ALL, nullptr, 0, GDK_ACTION_MOVE);
    ToolbarDragDropHelper::dragDestAddToolbar(target);

    g_signal_connect(target, "drag-data-received", G_CALLBACK(dragDataReceived), this);

    // init separator and spacer
    GtkWidget* tbSeparators = get("tbSeparator");

    for (SeparatorData* data: {&dataSeparator, &dataSpacer}) {
        GtkWidget* icon = ToolbarSeparatorImage::newImage(data->separator);
        g_return_if_fail(icon != nullptr);
        GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
        gtk_widget_show(box);

        GtkWidget* label = gtk_label_new(data->label);
        gtk_widget_show(label);
        gtk_box_pack_end(GTK_BOX(box), label, false, false, 0);

        gtk_widget_show(icon);
        gtk_box_pack_end(GTK_BOX(box), icon, false, false, 0);

        GtkWidget* ebox = gtk_event_box_new();
        gtk_container_add(GTK_CONTAINER(ebox), box);
        gtk_widget_show(ebox);

        // make ebox a drag source
        gtk_drag_source_set(ebox, GDK_BUTTON1_MASK, &ToolbarDragDropHelper::dropTargetEntry, 1, GDK_ACTION_MOVE);
        ToolbarDragDropHelper::dragSourceAddToolbar(ebox);

        g_signal_connect(ebox, "drag-begin", G_CALLBACK(toolitemDragBeginSeparator), data);
        g_signal_connect(ebox, "drag-end", G_CALLBACK(toolitemDragEndSeparator), data);
        g_signal_connect(ebox, "drag-data-get", G_CALLBACK(toolitemDragDataGetSeparator), data);
        gtk_grid_attach(GTK_GRID(tbSeparators), ebox, data->pos, 0, 1, 1);
    }
}

ToolbarCustomizeDialog::~ToolbarCustomizeDialog() {
    freeIconview();
    freeColorIconview();

    /* We can only delete this list at the end, it would be better to delete this list
     * after a refresh and after drag_end is called...
     */
    for (ToolItemDragData* data: this->itemDatalist) {
        if (data->icon != nullptr) {
            g_object_unref(data->icon);
        }
        g_object_unref(data->ebox);
        g_free(data);
    }
}

void ToolbarCustomizeDialog::toolitemDragBeginSeparator(GtkWidget* widget, GdkDragContext* context, void* data) {
    SeparatorData* sepData = static_cast<SeparatorData*>(data);
    ToolItemDragCurrentData::setData(sepData->type, -1, nullptr);
    GdkPixbuf* pixbuf = ToolbarSeparatorImage::getNewToolPixbuf(sepData->separator);
    gtk_drag_set_icon_pixbuf(context, pixbuf, -2, -2);
    g_object_unref(pixbuf);
}

void ToolbarCustomizeDialog::toolitemDragEndSeparator(GtkWidget* widget, GdkDragContext* context, void* unused) {
    ToolItemDragCurrentData::clearData();
}

void ToolbarCustomizeDialog::toolitemDragDataGetSeparator(GtkWidget* widget, GdkDragContext* context,
                                                          GtkSelectionData* selection_data, guint info, guint time,
                                                          void* data) {
    SeparatorData* sepData = static_cast<SeparatorData*>(data);
    ToolItemDragDropData* it = ToolitemDragDrop::ToolItemDragDropData_new(nullptr);
    it->type = sepData->type;

    gtk_selection_data_set(selection_data, ToolbarDragDropHelper::atomToolItem, 0, reinterpret_cast<const guchar*>(it),
                           sizeof(ToolItemDragDropData));

    g_free(it);
}

/**
 * Drag a Toolitem from dialog
 */
void ToolbarCustomizeDialog::toolitemDragBegin(GtkWidget* widget, GdkDragContext* context, ToolItemDragData* data) {
    ToolItemDragCurrentData::setData(TOOL_ITEM_ITEM, -1, data->item);
    if (data->icon) {
        ToolbarDragDropHelper::gdk_context_set_icon_from_image(context, data->icon);
    }
    gtk_widget_hide(data->ebox);
}

/**
 * Drag a Toolitem from dialog STOPPED
 */
void ToolbarCustomizeDialog::toolitemDragEnd(GtkWidget* widget, GdkDragContext* context, ToolItemDragData* data) {
    ToolItemDragCurrentData::clearData();
    gtk_widget_show(data->ebox);
}

void ToolbarCustomizeDialog::toolitemDragDataGet(GtkWidget* widget, GdkDragContext* context,
                                                 GtkSelectionData* selection_data, guint info, guint time,
                                                 ToolItemDragData* data) {
    g_return_if_fail(data != nullptr);
    g_return_if_fail(data->item != nullptr);

    data->item->setUsed(true);
    data->dlg->rebuildIconview();

    ToolItemDragDropData* it = ToolitemDragDrop::ToolItemDragDropData_new(data->item);

    gtk_selection_data_set(selection_data, ToolbarDragDropHelper::atomToolItem, 0, reinterpret_cast<const guchar*>(it),
                           sizeof(ToolItemDragDropData));

    g_free(it);
}

/**
 * Drag a Toolitem from dialog
 */
void ToolbarCustomizeDialog::toolitemColorDragBegin(GtkWidget* widget, GdkDragContext* context,
                                                    ColorToolItemDragData* data) {
    ToolItemDragCurrentData::setDataColor(-1, data->namedColor);

    GdkPixbuf* image = ColorSelectImage::newColorIconPixbuf(data->namedColor->getColor(), 16, true);

    gtk_drag_set_icon_pixbuf(context, image, -2, -2);
}

/**
 * Drag a Toolitem from dialog STOPPED
 */
void ToolbarCustomizeDialog::toolitemColorDragEnd(GtkWidget* widget, GdkDragContext* context,
                                                  ColorToolItemDragData* data) {
    ToolItemDragCurrentData::clearData();
    gtk_widget_show(widget);

    data->dlg->rebuildColorIcons();
}

void ToolbarCustomizeDialog::toolitemColorDragDataGet(GtkWidget* widget, GdkDragContext* context,
                                                      GtkSelectionData* selection_data, guint info, guint time,
                                                      ColorToolItemDragData* data) {
    ToolItemDragCurrentData::setDataColor(-1, data->namedColor);

    ToolItemDragDropData* it = ToolitemDragDrop::ToolItemDragDropData_new(nullptr);
    it->type = TOOL_ITEM_COLOR;
    it->namedColor = data->namedColor;

    gtk_selection_data_set(selection_data, ToolbarDragDropHelper::atomToolItem, 0, reinterpret_cast<const guchar*>(it),
                           sizeof(ToolItemDragDropData));

    g_free(it);
}

/**
 * A tool item was dragged to the dialog
 */
void ToolbarCustomizeDialog::dragDataReceived(GtkWidget* widget, GdkDragContext* dragContext, gint x, gint y,
                                              GtkSelectionData* data, guint info, guint time,
                                              ToolbarCustomizeDialog* dlg) {
    if (gtk_selection_data_get_data_type(data) != ToolbarDragDropHelper::atomToolItem) {
        gtk_drag_finish(dragContext, false, false, time);
        return;
    }

    auto* d = reinterpret_cast<ToolItemDragDropData const*>(gtk_selection_data_get_data(data));
    g_return_if_fail(ToolitemDragDrop::checkToolItemDragDropData(d));

    if (d->type == TOOL_ITEM_ITEM) {
        d->item->setUsed(false);
        dlg->rebuildIconview();
    } else if (d->type == TOOL_ITEM_SEPARATOR) {
        /*
         * There is always a seperator shown in the dialog.
         * Hence dragging a separator into the dialog does not
         * require any action.
         */
    } else if (d->type == TOOL_ITEM_SPACER) {
        /*
         * There is always a spacer shown in the dialog.
         * Hence dragging a spacer into the dialog does not
         * require any action.
         */
    } else if (d->type == TOOL_ITEM_COLOR) {
        /*
         * The dialog always contains the full palette of colors.
         * Hence dragging a color toolitem into the dialog does note
         * require any action.
         */
    } else {
        g_warning("ToolbarCustomizeDialog::dragDataReceived unhandled type: %i", d->type);
    }

    gtk_drag_finish(dragContext, true, false, time);
}

/**
 * clear the icon list
 */
void ToolbarCustomizeDialog::freeIconview() {
    GtkGrid* table = GTK_GRID(get("tbDefaultTools"));

    GList* children = gtk_container_get_children(GTK_CONTAINER(table));
    for (auto& w: GListView<GtkWidget>(children)) {
        gtk_container_remove(GTK_CONTAINER(table), &w);
    }
    g_list_free(children);
}

/**
 * builds up the icon list
 */
void ToolbarCustomizeDialog::rebuildIconview() {
    freeIconview();

    GtkGrid* table = GTK_GRID(get("tbDefaultTools"));

    int i = 0;
    for (AbstractToolItem* item: *this->win->getToolMenuHandler()->getToolItems()) {
        if (item->isUsed()) {
            continue;
        }

        std::string name = item->getToolDisplayName();
        GtkWidget* icon = item->getNewToolIcon(); /* floating */
        g_return_if_fail(icon != nullptr);

        GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
        gtk_widget_show(box);

        GtkWidget* label = gtk_label_new(name.c_str());
        gtk_widget_show(label);
        gtk_box_pack_end(GTK_BOX(box), label, false, false, 0);

        GtkWidget* ebox = gtk_event_box_new();
        gtk_container_add(GTK_CONTAINER(ebox), box);
        gtk_widget_show(ebox);

        gtk_widget_show(icon);

        gtk_box_pack_end(GTK_BOX(box), icon, false, false, 0);

        // make ebox a drag source
        gtk_drag_source_set(ebox, GDK_BUTTON1_MASK, &ToolbarDragDropHelper::dropTargetEntry, 1, GDK_ACTION_MOVE);
        ToolbarDragDropHelper::dragSourceAddToolbar(ebox);

        ToolItemDragData* data = g_new(ToolItemDragData, 1);
        data->dlg = this;
        data->icon = GTK_WIDGET(g_object_ref(icon));
        data->item = item;
        // store reference to ebox
        data->ebox = ebox;
        g_object_ref(ebox);

        this->itemDatalist.push_front(data);

        g_signal_connect(ebox, "drag-begin", G_CALLBACK(toolitemDragBegin), data);
        g_signal_connect(ebox, "drag-end", G_CALLBACK(toolitemDragEnd), data);

        g_signal_connect(ebox, "drag-data-get", G_CALLBACK(toolitemDragDataGet), data);

        const int x = i % 3;
        const int y = i / 3;
        gtk_grid_attach(table, ebox, x, y, 1, 1);

        i++;
    }
}

/**
 * clear the icon list
 */
void ToolbarCustomizeDialog::freeColorIconview() {
    GtkGrid* table = GTK_GRID(get("tbColor"));

    GList* children = gtk_container_get_children(GTK_CONTAINER(table));
    for (auto& w: GListView<GtkWidget>(children)) {
        gtk_container_remove(GTK_CONTAINER(table), &w);
    }

    g_list_free(children);
}

void ToolbarCustomizeDialog::rebuildColorIcons() {
    GtkGrid* table = GTK_GRID(get("tbColor"));
    g_return_if_fail(table != nullptr);

    freeColorIconview();

    const Palette& palette = this->win->getToolMenuHandler()->getControl()->getSettings()->getColorPalette();

    for (size_t i{}; i < palette.size(); i++) {
        // namedColor needs to be a pointer to pass it into a ColorToolItemDragData
        const NamedColor* namedColor = &(palette.getColorAt(i));
        const Color c = namedColor->getColor();
        GtkWidget* icon = ColorSelectImage::newColorIcon(c, 16, true);

        GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
        gtk_widget_show(box);

        GtkWidget* label = gtk_label_new(namedColor->getName().c_str());
        gtk_widget_show(label);
        gtk_box_pack_end(GTK_BOX(box), label, false, false, 0);

        GtkWidget* ebox = gtk_event_box_new();
        gtk_container_add(GTK_CONTAINER(ebox), box);
        gtk_widget_show(ebox);

        gtk_widget_show(icon);

        gtk_box_pack_end(GTK_BOX(box), icon, false, false, 0);

        // make ebox a drag source
        gtk_drag_source_set(ebox, GDK_BUTTON1_MASK, &ToolbarDragDropHelper::dropTargetEntry, 1, GDK_ACTION_MOVE);
        ToolbarDragDropHelper::dragSourceAddToolbar(ebox);


        ColorToolItemDragData* data = g_new(ColorToolItemDragData, 1);
        data->dlg = this;
        data->icon = nullptr;

        /*
         * Since namedColor actually is a const reference, the reference will be valid even once namedColor goes out of
         * scope
         */
        data->namedColor = namedColor;
        data->ebox = ebox;

        g_signal_connect(ebox, "drag-begin", G_CALLBACK(toolitemColorDragBegin), data);
        g_signal_connect(ebox, "drag-end", G_CALLBACK(toolitemColorDragEnd), data);
        g_signal_connect(ebox, "drag-data-get", G_CALLBACK(toolitemColorDragDataGet), data);

        if (i >= std::numeric_limits<int>::max())
            g_error("Int overflow because of two many colors defined in Palette");
        const int ii = static_cast<int>(i);

        // In the dialog 5 colors are shown per row
        const int x = ii % 5;
        const int y = ii / 5;

        gtk_grid_attach(table, ebox, x, y, 1, 1);
    }

    gtk_widget_show_all(GTK_WIDGET(table));
}

void ToolbarCustomizeDialog::windowResponseCb(GtkDialog* dialog, int response, ToolbarCustomizeDialog* dlg) {
    gtk_widget_hide(GTK_WIDGET(dialog));

    dlg->handler->toolbarConfigDialogClosed();
}

/**
 * Displays the dialog
 */
void ToolbarCustomizeDialog::show(GtkWindow* parent) {
    g_signal_connect(this->window, "response", G_CALLBACK(windowResponseCb), this);

    gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);

    gtk_widget_show_all(this->window);
}
