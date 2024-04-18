#include "ToolbarManageDialog.h"

#include <string>  // for allocator, string
#include <vector>  // for vector

#include <glib-object.h>  // for g_signal_connect
#include <pango/pango.h>  // for PANGO_WEIGHT_NORMAL

#include "gui/Builder.h"
#include "gui/toolbarMenubar/model/ToolbarData.h"   // for ToolbarData
#include "gui/toolbarMenubar/model/ToolbarModel.h"  // for ToolbarModel
#include "util/Assert.h"                            // for xoj_assert
#include "util/i18n.h"                              // for _

class GladeSearchpath;

constexpr auto UI_FILE = "toolbarManageDialog.ui";
constexpr auto UI_DIALOG_NAME = "manageToolbarsDialog";

enum { COLUMN_STRING, COLUMN_BOLD, COLUMN_POINTER, COLUMN_EDITABLE, N_COLUMNS };

static xoj::util::GObjectSPtr<GtkListStore> createModel(ToolbarModel* tb) {
    xoj_assert(tb);
    GtkTreeIter iter;
    GtkListStore* model = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING, G_TYPE_INT, G_TYPE_POINTER, G_TYPE_BOOLEAN);
    gtk_list_store_append(model, &iter);
    gtk_list_store_set(model, &iter, COLUMN_STRING, _("Predefined"), COLUMN_BOLD, PANGO_WEIGHT_BOLD, COLUMN_POINTER,
                       nullptr, COLUMN_EDITABLE, false, -1);

    for (const auto& data: tb->getToolbars()) {
        if (data->isPredefined()) {
            gtk_list_store_append(model, &iter);
            gtk_list_store_set(model, &iter, COLUMN_STRING, data->getName().c_str(), COLUMN_BOLD, PANGO_WEIGHT_NORMAL,
                               COLUMN_POINTER, data.get(), COLUMN_EDITABLE, false, -1);
        }
    }

    gtk_list_store_append(model, &iter);
    gtk_list_store_set(model, &iter, COLUMN_STRING, _("Customized"), COLUMN_BOLD, PANGO_WEIGHT_BOLD, COLUMN_POINTER,
                       nullptr, COLUMN_EDITABLE, false, -1);

    for (const auto& data: tb->getToolbars()) {
        if (!data->isPredefined()) {
            gtk_list_store_append(model, &iter);
            gtk_list_store_set(model, &iter, COLUMN_STRING, data->getName().c_str(), COLUMN_BOLD, PANGO_WEIGHT_NORMAL,
                               COLUMN_POINTER, data.get(), COLUMN_EDITABLE, true, -1);
        }
    }
    return xoj::util::GObjectSPtr<GtkListStore>(model, xoj::util::adopt);
}

ToolbarManageDialog::ToolbarManageDialog(GladeSearchpath* gladeSearchPath, ToolbarModel* tbModel,
                                         std::function<void()> callback):
        tbModel(tbModel), model(createModel(tbModel)), callback(callback) {
    Builder builder(gladeSearchPath, UI_FILE);
    window.reset(GTK_WINDOW(builder.get(UI_DIALOG_NAME)));
    tree = GTK_TREE_VIEW(builder.get("toolbarList"));
    copyButton = builder.get("btCopy");
    deleteButton = builder.get("btDelete");

    gtk_tree_view_set_model(tree, GTK_TREE_MODEL(this->model.get()));

    GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn* column =
            gtk_tree_view_column_new_with_attributes(_("Toolbars"), renderer, "text", COLUMN_STRING, "weight",
                                                     COLUMN_BOLD, "editable", COLUMN_EDITABLE, nullptr);

    gtk_tree_view_append_column(tree, column);

    GtkTreeSelection* select = gtk_tree_view_get_selection(tree);
    gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);
    g_signal_connect(select, "changed", G_CALLBACK(treeSelectionChangedCallback), this);

    g_signal_connect(renderer, "edited", G_CALLBACK(treeCellEditedCallback), this);

    g_signal_connect(builder.get("btNew"), "clicked", G_CALLBACK(buttonNewCallback), this);
    g_signal_connect(deleteButton, "clicked", G_CALLBACK(buttonDeleteCallback), this);
    g_signal_connect(copyButton, "clicked", G_CALLBACK(buttonCopyCallback), this);

    g_signal_connect_swapped(builder.get("btClose"), "clicked", G_CALLBACK(gtk_window_close), this->window.get());
}

ToolbarManageDialog::~ToolbarManageDialog() { this->callback(); }

void ToolbarManageDialog::buttonNewCallback(GtkButton* button, ToolbarManageDialog* dlg) {
    auto data = std::make_unique<ToolbarData>(false);
    data->setName(_("New"));
    data->setId("custom");
    dlg->tbModel->initCopyNameId(data.get());
    dlg->addToolbarData(std::move(data));
}

void ToolbarManageDialog::buttonDeleteCallback(GtkButton* button, ToolbarManageDialog* dlg) {
    ToolbarData* selected = dlg->getSelectedEntry();
    if (selected == nullptr) {
        return;
    }

    GtkTreeModel* model = GTK_TREE_MODEL(dlg->model.get());
    GtkTreeIter iter;
    if (gtk_tree_model_get_iter_first(model, &iter)) {
        do {
            ToolbarData* data = nullptr;
            gtk_tree_model_get(model, &iter, COLUMN_POINTER, &data, -1);

            if (data == selected) {
                gtk_list_store_remove(dlg->model.get(), &iter);
                break;
            }
        } while (gtk_tree_model_iter_next(model, &iter));
    }

    dlg->tbModel->remove(selected);
    dlg->updateSelectionData();
}

void ToolbarManageDialog::buttonCopyCallback(GtkButton* button, ToolbarManageDialog* dlg) {
    ToolbarData* selected = dlg->getSelectedEntry();
    if (selected == nullptr) {
        return;
    }

    auto data = std::make_unique<ToolbarData>(*selected);
    dlg->tbModel->initCopyNameId(data.get());
    dlg->addToolbarData(std::move(data));
}

void ToolbarManageDialog::addToolbarData(std::unique_ptr<ToolbarData> tbd) {
    auto* data = this->tbModel->add(std::move(tbd));

    GtkTreeIter iter;
    gtk_list_store_append(this->model.get(), &iter);
    gtk_list_store_set(this->model.get(), &iter, COLUMN_STRING, data->getName().c_str(), COLUMN_BOLD,
                       PANGO_WEIGHT_NORMAL, COLUMN_POINTER, data, COLUMN_EDITABLE, true, -1);

    GtkTreePath* path = gtk_tree_model_get_path(GTK_TREE_MODEL(this->model.get()), &iter);
    GtkTreeViewColumn* column = gtk_tree_view_get_column(tree, 0);

    gtk_tree_view_set_cursor(tree, path, column, true);
}

void ToolbarManageDialog::treeCellEditedCallback(GtkCellRendererText* renderer, gchar* pathString, gchar* newText,
                                                 ToolbarManageDialog* dlg) {
    GtkTreeIter iter;
    ToolbarData* data = nullptr;

    gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(dlg->model.get()), &iter, pathString);
    gtk_tree_model_get(GTK_TREE_MODEL(dlg->model.get()), &iter, COLUMN_POINTER, &data, -1);
    if (data) {
        gtk_list_store_set(dlg->model.get(), &iter, COLUMN_STRING, newText, -1);
        data->setName(newText);
    }
}

void ToolbarManageDialog::entrySelected(ToolbarData* data) {
    if (data == nullptr) {
        gtk_widget_set_sensitive(copyButton, false);
        gtk_widget_set_sensitive(deleteButton, false);
    } else {
        gtk_widget_set_sensitive(copyButton, true);
        gtk_widget_set_sensitive(deleteButton, !data->isPredefined());
    }
}

auto ToolbarManageDialog::getSelectedEntry() -> ToolbarData* {
    GtkTreeIter iter;
    GtkTreeModel* model = nullptr;
    ToolbarData* data = nullptr;

    GtkTreeSelection* selection = gtk_tree_view_get_selection(tree);
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
