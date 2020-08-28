#include "ToolbarCustomizeDialog.h"

#include <config.h>

#include "gui/MainWindow.h"
#include "gui/toolbarMenubar/AbstractToolItem.h"
#include "gui/toolbarMenubar/ToolMenuHandler.h"
#include "gui/toolbarMenubar/icon/ColorSelectImage.h"
#include "gui/toolbarMenubar/icon/ToolbarSeparatorImage.h"
#include "gui/toolbarMenubar/model/ToolbarData.h"
#include "gui/toolbarMenubar/model/ToolbarModel.h"

#include "CustomizeableColorList.h"
#include "ToolItemDragCurrentData.h"
#include "ToolbarDragDropHandler.h"
#include "ToolbarDragDropHelper.h"
#include "Util.h"
#include "i18n.h"

using ToolItemDragData = struct _ToolItemDragData;

struct _ToolItemDragData {
    ToolbarCustomizeDialog* dlg;
    GdkPixbuf* icon;
    AbstractToolItem* item;
    GtkWidget* ebox;
};

ToolbarCustomizeDialog::ToolbarCustomizeDialog(GladeSearchpath* gladeSearchPath, MainWindow* win,
                                               ToolbarDragDropHandler* handler):
        GladeGui(gladeSearchPath, "toolbarCustomizeDialog.glade", "DialogCustomizeToolbar") {
    this->win = win;
    this->handler = handler;
    this->colorList = new CustomizeableColorList();

    rebuildIconview();
    rebuildColorIcons();

    GtkWidget* target = get("viewport1");

    // prepare drag & drop
    gtk_drag_dest_set(target, GTK_DEST_DEFAULT_ALL, nullptr, 0, GDK_ACTION_MOVE);
    ToolbarDragDropHelper::dragDestAddToolbar(target);

    g_signal_connect(target, "drag-data-received", G_CALLBACK(dragDataReceived), this);

    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    gtk_widget_show(box);

    GtkWidget* label = gtk_label_new(_("Separator"));
    gtk_widget_show(label);
    gtk_box_pack_end(GTK_BOX(box), label, false, false, 0);

    GtkWidget* ebox = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(ebox), box);
    gtk_widget_show(ebox);

    GtkWidget* separator = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_size_request(separator, 2, 22);
    gtk_widget_show(separator);
    gtk_box_pack_end(GTK_BOX(box), separator, false, false, 0);

    // make ebox a drag source
    gtk_drag_source_set(ebox, GDK_BUTTON1_MASK, &ToolbarDragDropHelper::dropTargetEntry, 1, GDK_ACTION_MOVE);
    ToolbarDragDropHelper::dragSourceAddToolbar(ebox);

    g_signal_connect(ebox, "drag-begin", G_CALLBACK(toolitemDragBeginSeparator), nullptr);
    g_signal_connect(ebox, "drag-end", G_CALLBACK(toolitemDragEndSeparator), nullptr);

    g_signal_connect(ebox, "drag-data-get", G_CALLBACK(toolitemDragDataGetSeparator), nullptr);

    // init separator
    GtkWidget* tbSeparator = get("tbSeparator");
    gtk_grid_attach(GTK_GRID(tbSeparator), ebox, 0, 0, 1, 1);
}

ToolbarCustomizeDialog::~ToolbarCustomizeDialog() {
    freeIconview();
    freeColorIconview();

    // We can only delete this list at the end, it would be better to delete this list
    // after a refresh and after drag_end is called...
    for (ToolItemDragData* data: this->itemDatalist) {
        g_object_unref(data->icon);
        g_free(data);
    }

    delete this->colorList;
    this->colorList = nullptr;
}

void ToolbarCustomizeDialog::toolitemDragBeginSeparator(GtkWidget* widget, GdkDragContext* context, void* unused) {
    ToolItemDragCurrentData::setData(TOOL_ITEM_SEPARATOR, -1, nullptr);

    GtkWidget* icon = ToolbarSeparatorImage::newSepeartorImage();
    gtk_drag_set_icon_pixbuf(context, ToolbarDragDropHelper::getImagePixbuf(GTK_IMAGE(icon)), -2, -2);
    g_object_unref(icon);
}

void ToolbarCustomizeDialog::toolitemDragEndSeparator(GtkWidget* widget, GdkDragContext* context, void* unused) {
    ToolItemDragCurrentData::clearData();
}

void ToolbarCustomizeDialog::toolitemDragDataGetSeparator(GtkWidget* widget, GdkDragContext* context,
                                                          GtkSelectionData* selection_data, guint info, guint time,
                                                          void* unused) {

    ToolItemDragDropData* it = ToolitemDragDrop::ToolItemDragDropData_new(nullptr);
    it->type = TOOL_ITEM_SEPARATOR;

    gtk_selection_data_set(selection_data, ToolbarDragDropHelper::atomToolItem, 0, reinterpret_cast<const guchar*>(it),
                           sizeof(ToolItemDragDropData));

    g_free(it);
}

/**
 * Drag a Toolitem from dialog
 */
void ToolbarCustomizeDialog::toolitemDragBegin(GtkWidget* widget, GdkDragContext* context, ToolItemDragData* data) {
    ToolItemDragCurrentData::setData(TOOL_ITEM_ITEM, -1, data->item);

    gtk_drag_set_icon_pixbuf(context, data->icon, -2, -2);
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
void ToolbarCustomizeDialog::toolitemColorDragBegin(GtkWidget* widget, GdkDragContext* context, void* data) {
    Color color = GPOINTER_TO_UINT(data);
    ToolItemDragCurrentData::setDataColor(-1, color);

    GdkPixbuf* image = ColorSelectImage::newColorIconPixbuf(color, 32, true);

    gtk_drag_set_icon_pixbuf(context, image, -2, -2);

    g_object_unref(image);
    gtk_widget_hide(widget);
}

/**
 * Drag a Toolitem from dialog STOPPED
 */
void ToolbarCustomizeDialog::toolitemColorDragEnd(GtkWidget* widget, GdkDragContext* context,
                                                  ToolbarCustomizeDialog* dlg) {
    ToolItemDragCurrentData::clearData();
    gtk_widget_show(widget);

    dlg->rebuildColorIcons();
}

void ToolbarCustomizeDialog::toolitemColorDragDataGet(GtkWidget* widget, GdkDragContext* context,
                                                      GtkSelectionData* selection_data, guint info, guint time,
                                                      void* data) {

    Color color = GPOINTER_TO_UINT(data);

    ToolItemDragDropData* it = ToolitemDragDrop::ToolItemDragDropData_new(nullptr);
    it->color = color;
    it->type = TOOL_ITEM_COLOR;

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
        // simple ignore the separator
    } else if (d->type == TOOL_ITEM_COLOR) {
        dlg->win->getToolMenuHandler()->removeColorToolItem(d->item);
        dlg->rebuildColorIcons();
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
    for (GList* l = children; l != nullptr; l = l->next) {
        auto* w = static_cast<GtkWidget*>(l->data);
        gtk_container_remove(GTK_CONTAINER(table), w);
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

        string name = item->getToolDisplayName();
        GtkWidget* icon = item->getNewToolIcon();
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
        data->icon = ToolbarDragDropHelper::getImagePixbuf(GTK_IMAGE(icon));
        data->item = item;
        data->ebox = ebox;

        this->itemDatalist.push_front(data);

        g_signal_connect(ebox, "drag-begin", G_CALLBACK(toolitemDragBegin), data);
        g_signal_connect(ebox, "drag-end", G_CALLBACK(toolitemDragEnd), data);

        g_signal_connect(ebox, "drag-data-get", G_CALLBACK(toolitemDragDataGet), data);

        int x = i % 3;
        int y = i / 3;
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
    for (GList* l = children; l != nullptr; l = l->next) {
        auto* w = static_cast<GtkWidget*>(l->data);
        gtk_container_remove(GTK_CONTAINER(table), w);
    }

    g_list_free(children);
}

void ToolbarCustomizeDialog::rebuildColorIcons() {
    GtkGrid* table = GTK_GRID(get("tbColor"));
    g_return_if_fail(table != nullptr);

    freeColorIconview();

    ToolMenuHandler* tmh = this->win->getToolMenuHandler();

    int i = 0;
    for (XojColor* color: *this->colorList->getPredefinedColors()) {
        if (tmh->isColorInUse(color->getColor())) {
            continue;
        }

        GtkWidget* icon = ColorSelectImage::newColorIcon(color->getColor(), 16, true);

        GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
        gtk_widget_show(box);

        GtkWidget* label = gtk_label_new(color->getName().c_str());
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

        g_signal_connect(ebox, "drag-begin", G_CALLBACK(toolitemColorDragBegin), GUINT_TO_POINTER(color->getColor()));
        g_signal_connect(ebox, "drag-end", G_CALLBACK(toolitemColorDragEnd), this);
        g_signal_connect(ebox, "drag-data-get", G_CALLBACK(toolitemColorDragDataGet),
                         GUINT_TO_POINTER(color->getColor()));

        int x = i % 5;
        int y = i / 5;
        i++;
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
