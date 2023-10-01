#include "Workspace.h"

#include <cstdint>  // for int64_t
#include <memory>   // for std::make_unique and std::make_shared
#include <set>
#include <string>  // for string

#include <config-features.h>
#include <gdk/gdk.h>      // for gdk_display_get...
#include <glib-object.h>  // for G_CALLBACK, g_s...
#include <gtk/gtk.h>      // for gtk_toggle_tool_button_new...

#include "control/Control.h"            // for Control
#include "control/settings/Settings.h"  // for Settings
#include "control/workspace/WorkspaceHandler.h"
#include "gui/GladeGui.h"         // for GladeGui
#include "model/Document.h"       // for Document
#include "model/XojPage.h"        // for XojPage
#include "pdf/base/XojPdfPage.h"  // for XojPdfPageSPtr
#include "util/Util.h"            // for npos
#include "util/XojMsgBox.h"
#include "util/i18n.h"  // for _, FC, _F

enum
{
  COL_NAME = 0,
  COL_FULLPATH,
  NUM_COLS
};

Workspace::Workspace(GladeGui* gui, Control* control, WorkspaceHandler* workspaceHandler):
        control(control), gui(gui), workspaceHandler(workspaceHandler) {

    this->workspaceSidebar = gui->get("workspaceSidebar");

    //this->folderPath = "/home/teogalletta/Documents/my-obsidian-vault/100 university";
    this->createViewAndStore();

    gtk_container_add(GTK_CONTAINER(this->workspaceSidebar), treeView);

    //gtk_viewport_set_shadow_type(GTK_VIEWPORT(gui->get("workspaceSidebarContainer")), GTK_SHADOW_IN);

    gtk_widget_show_all(this->workspaceSidebar);

    workspaceHandler->addListener(this);
    for (const auto& path: workspaceHandler->getFoldersPaths())
        addFolder(path);
}


Workspace::~Workspace() {}

void Workspace::fillTreeFromFolderPath(GtkTreeStore* store, GtkTreeIter* parent, std::string folderPath) {

    if (!fs::is_directory(fs::path(folderPath))) {
        g_warning("The workspace folder path %s does not exist", folderPath.c_str());
        return;
    }

    // to order the filenames
    std::set<fs::directory_entry> entries;
    for (const auto& entry : fs::directory_iterator(folderPath))
        entries.insert(entry);
    
    for (const auto& entry : entries) {

        fs::path entryPath = entry.path();
        fs::path entryFilename = entryPath.filename();
        
        GtkTreeIter iter;
        if (entry.is_directory() || entryPath.extension() == ".pdf" || entryPath.extension() == ".xopp") {
            gtk_tree_store_append(store, &iter, parent);
            gtk_tree_store_set(store, &iter, COL_NAME, entryFilename.generic_string().c_str(), COL_FULLPATH, entryPath.c_str(), -1);
        }

        if (entry.is_directory()) {
            fillTreeFromFolderPath(store, &iter, entryPath);
        }

    }
}

void Workspace::createViewAndStore() {

    treeView = gtk_tree_view_new();

    g_signal_connect(treeView, "row-activated", G_CALLBACK(treeViewRowClicked), this);

    GtkCellRenderer* renderer;

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeView), -1, "Name", renderer, "text", 0, nullptr);
    //g_signal_connect(renderer, "cell-data-func", G_CALLBACK(setRootFolderTextBold), nullptr);

    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(treeView), false);

    store = gtk_tree_store_new(NUM_COLS, G_TYPE_STRING, G_TYPE_STRING);
    GtkTreeModel* model = GTK_TREE_MODEL(store);
    gtk_tree_view_set_model(GTK_TREE_VIEW(treeView), model);
    g_object_unref(model);
}

auto Workspace::getControl() -> Control* { return this->control; }

void Workspace::addFolder(fs::path folderPath) {
    
    GtkTreeIter iter;
    gtk_tree_store_append(store, &iter, nullptr);
    gtk_tree_store_set(store, &iter, COL_NAME, fs::path(folderPath).filename().c_str(), -1);
    fillTreeFromFolderPath(store, &iter, folderPath);

    gtk_widget_show_all(GTK_WIDGET(workspaceSidebar));
}

void Workspace::removeFolder(fs::path folderPath) {}

void Workspace::treeViewRowClicked(GtkTreeView* treeView, GtkTreePath* path, GtkTreeViewColumn* column, Workspace* workspace) {
    
    GtkTreeModel* model = gtk_tree_view_get_model(treeView);

    GtkTreeIter iter;
    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gchar* str;
        gtk_tree_model_get(GTK_TREE_MODEL(model), &iter, COL_FULLPATH, &str, -1);

        fs::path fsPath;
        if (str != nullptr) // root folder
            fsPath = fs::path(std::string(str));

        if (str == nullptr || fs::is_directory(fsPath)) {
            bool wasExpanded = gtk_tree_view_row_expanded(treeView, path);
            if (wasExpanded)
                gtk_tree_view_collapse_row(treeView, path);
            else
                gtk_tree_view_expand_row(treeView, path, false);
        }
        else {
            workspace->control->openFile(fsPath);
        }
        
        g_free(str);
    }

}
/*
void Workspace::setRootFolderTextBold(GtkTreeViewColumn *column, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, void* _ptr) {
    gchar* text;
    gtk_tree_model_get(model, iter, COL_FULLPATH, &text, -1);

    if (text != nullptr)
        return;
    
    // Creiamo una tag di stile Pango per il testo in grassetto
    PangoAttrList* attrs = pango_attr_list_new();
    PangoAttribute* attr = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
    pango_attr_list_insert(attrs, attr);

    // Impostiamo il testo nel renderer con il tag di stile Pango
    g_object_set(renderer, "text", text, "attributes", attrs, nullptr);

    g_free(text);
    pango_attr_list_unref(attrs);
}*/

void Workspace::saveSize() {
    if (this->control->getSettings()->isWorkspaceVisible()) {
        GtkAllocation alloc;
        gtk_widget_get_allocation(this->workspaceSidebar, &alloc);

        this->control->getSettings()->setWorkspaceWidth(alloc.width);
    }
}
