#include "ToolbarManageDialog.h"

#include <string>  // for allocator, string
#include <vector>  // for vector

#include <glib-object.h>  // for g_signal_connect
#include <pango/pango.h>  // for PANGO_WEIGHT_NORMAL

#include "gui/toolbarMenubar/model/ToolbarData.h"   // for ToolbarData
#include "gui/toolbarMenubar/model/ToolbarModel.h"  // for ToolbarModel
#include "util/i18n.h"                              // for _

class GladeSearchpath;

enum { COLUMN_STRING, COLUMN_BOLD, COLUMN_POINTER, COLUMN_EDITABLE, N_COLUMNS };

ToolbarManageDialog::ToolbarManageDialog(GladeSearchpath* gladeSearchPath, ToolbarModel* model):
        GladeGui(gladeSearchPath, "toolbarManageDialog.glade", "DialogManageToolbar"), tbModel(model) {
    GtkTreeIter iter;
    this->model = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING, G_TYPE_INT, G_TYPE_POINTER, G_TYPE_BOOLEAN);
    gtk_list_store_append(this->model, &iter);
    gtk_list_store_set(this->model, &iter, COLUMN_STRING, _("Predefined"), COLUMN_BOLD, PANGO_WEIGHT_BOLD,
                       COLUMN_POINTER, nullptr, COLUMN_EDITABLE, false, -1);

    for (ToolbarData* data: *model->getToolbars()) {
        if (data->isPredefined()) {
            gtk_list_store_append(this->model, &iter);
            gtk_list_store_set(this->model, &iter, COLUMN_STRING, data->getName().c_str(), COLUMN_BOLD,
                               PANGO_WEIGHT_NORMAL, COLUMN_POINTER, data, COLUMN_EDITABLE, false, -1);
        }
    }

    gtk_list_store_append(this->model, &iter);
    gtk_list_store_set(this->model, &iter, COLUMN_STRING, _("Customized"), COLUMN_BOLD, PANGO_WEIGHT_BOLD,
                       COLUMN_POINTER, nullptr, COLUMN_EDITABLE, false, -1);

    for (ToolbarData* data: *model->getToolbars()) {
        if (!data->isPredefined()) {
            gtk_list_store_append(this->model, &iter);
            gtk_list_store_set(this->model, &iter, COLUMN_STRING, data->getName().c_str(), COLUMN_BOLD,
                               PANGO_WEIGHT_NORMAL, COLUMN_POINTER, data, COLUMN_EDITABLE, true, -1);
        }
    }

    GtkWidget* tree = get("toolbarList");
    gtk_tree_view_set_model(GTK_TREE_VIEW(tree), GTK_TREE_MODEL(this->model));

    GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn* column =
            gtk_tree_view_column_new_with_attributes(_("Toolbars"), renderer, "text", COLUMN_STRING, "weight",
                                                     COLUMN_BOLD, "editable", COLUMN_EDITABLE, nullptr);

    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

    GtkTreeSelection* select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
    gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);
    g_signal_connect(G_OBJECT(select), "changed", G_CALLBACK(treeSelectionChangedCallback), this);

    g_signal_connect(renderer, "edited", (GCallback)treeCellEditedCallback, this);

    g_signal_connect(get("btNew"), "clicked", G_CALLBACK(buttonNewCallback), this);
    g_signal_connect(get("btDelete"), "clicked", G_CALLBACK(buttonDeleteCallback), this);
    g_signal_connect(get("btCopy"), "clicked", G_CALLBACK(buttonCopyCallback), this);

    entrySelected(nullptr);
}

ToolbarManageDialog::~ToolbarManageDialog() {
    g_object_unref(this->model);
    this->tbModel = nullptr;
}

void ToolbarManageDialog::buttonNewCallback(GtkButton* button, ToolbarManageDialog* dlg) {
    auto* data = new ToolbarData(false);
    data->setName(_("New"));
    data->setId("custom");
    dlg->tbModel->initCopyNameId(data);
    dlg->addToolbarData(data);
}

void ToolbarManageDialog::buttonDeleteCallback(GtkButton* button, ToolbarManageDialog* dlg) {
    ToolbarData* selected = dlg->getSelectedEntry();
    if (selected == nullptr) {
        return;
    }

    dlg->tbModel->remove(selected);
    GtkTreeIter iter;
    if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(dlg->model), &iter)) {
        do {
            ToolbarData* data = nullptr;
            gtk_tree_model_get(GTK_TREE_MODEL(dlg->model), &iter, COLUMN_POINTER, &data, -1);

            if (data == selected) {
                gtk_list_store_remove(dlg->model, &iter);
                break;
            }
        } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(dlg->model), &iter));
    }

    dlg->updateSelectionData();
    delete selected;
}

void ToolbarManageDialog::buttonCopyCallback(GtkButton* button, ToolbarManageDialog* dlg) {
    ToolbarData* selected = dlg->getSelectedEntry();
    if (selected == nullptr) {
        return;
    }

    auto* data = new ToolbarData(*selected);
    dlg->tbModel->initCopyNameId(data);
    dlg->addToolbarData(data);
}

void ToolbarManageDialog::addToolbarData(ToolbarData* data) {
    this->tbModel->add(data);
    GtkTreeIter iter;
    gtk_list_store_append(this->model, &iter);
    gtk_list_store_set(this->model, &iter, COLUMN_STRING, data->getName().c_str(), COLUMN_BOLD, PANGO_WEIGHT_NORMAL,
                       COLUMN_POINTER, data, COLUMN_EDITABLE, true, -1);

    GtkWidget* tree = get("toolbarList");

    GtkTreePath* path = gtk_tree_model_get_path(GTK_TREE_MODEL(this->model), &iter);
    GtkTreeViewColumn* column = gtk_tree_view_get_column(GTK_TREE_VIEW(tree), 0);

    gtk_tree_view_set_cursor(GTK_TREE_VIEW(tree), path, column, true);
}

void ToolbarManageDialog::treeCellEditedCallback(GtkCellRendererText* renderer, gchar* pathString, gchar* newText,
                                                 ToolbarManageDialog* dlg) {
    GtkTreeIter iter;
    ToolbarData* data = nullptr;

    gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(dlg->model), &iter, pathString);
    gtk_tree_model_get(GTK_TREE_MODEL(dlg->model), &iter, COLUMN_POINTER, &data, -1);
    if (data) {
        gtk_list_store_set(dlg->model, &iter, COLUMN_STRING, newText, -1);
        data->setName(newText);
    }
}

void ToolbarManageDialog::entrySelected(ToolbarData* data) {
    GtkWidget* btCopy = get("btCopy");
    GtkWidget* btDelete = get("btDelete");

    if (data == nullptr) {
        gtk_widget_set_sensitive(btCopy, false);
        gtk_widget_set_sensitive(btDelete, false);
    } else {
        gtk_widget_set_sensitive(btCopy, true);
        gtk_widget_set_sensitive(btDelete, !data->isPredefined());
    }
}

auto ToolbarManageDialog::getSelectedEntry() -> ToolbarData* {
    GtkTreeIter iter;
    GtkTreeModel* model = nullptr;
    ToolbarData* data = nullptr;

    GtkWidget* tree = get("toolbarList");

    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
    if (selection == nullptr) {
        return nullptr;
    }

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, COLUMN_POINTER, &data, -1);
        return data;
    }


    return nullptr;
}

void ToolbarManageDialog::updateSelectionData() { entrySelected(getSelectedEntry()); }

void ToolbarManageDialog::treeSelectionChangedCallback(GtkTreeSelection* selection, ToolbarManageDialog* dlg) {
    dlg->updateSelectionData();
}

void ToolbarManageDialog::show(GtkWindow* parent) {
    gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);
    gtk_dialog_run(GTK_DIALOG(this->window));
    gtk_widget_hide(this->window);
}
