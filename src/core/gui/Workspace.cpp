#include "Workspace.h"

#include <set>
#include <string>  // for string

#include <gdk/gdk.h>      // for gdk_display_get...
#include <glib-object.h>  // for G_CALLBACK, g_s...
#include <gtk/gtk.h>      // for gtk_toggle_tool_button_new...

#include "control/Control.h"            // for Control
#include "control/settings/Settings.h"  // for Settings
#include "control/workspace/WorkspaceHandler.h"
#include "gui/GladeGui.h"         // for GladeGui
#include "pdf/base/XojPdfPage.h"  // for XojPdfPageSPtr

enum
{
  COL_NAME = 0,
  COL_FULLPATH,
  NUM_COLS
};

Workspace::Workspace(GladeGui* gui, Control* control, WorkspaceHandler* workspaceHandler):
        control(control), gui(gui), workspaceHandler(workspaceHandler) {

    this->workspaceSidebar = gui->get("workspaceSidebar");

    this->createViewAndStore();

    gtk_container_add(GTK_CONTAINER(this->workspaceSidebar), treeView);

    g_signal_connect(gui->get("buttonAddFolderWorkspace"), "clicked", G_CALLBACK(buttonAddFolderWorkspaceClicked), this);
    g_signal_connect(gui->get("buttonCloseWorkspace"), "clicked", G_CALLBACK(buttonCloseWorkspaceClicked), this);

    gtk_widget_show_all(this->workspaceSidebar);

    workspaceHandler->addListener(this);
    for (const auto& path: workspaceHandler->getFoldersPaths())
        addFolder(path);
}

Workspace::~Workspace() {}

void Workspace::buttonAddFolderWorkspaceClicked(GtkButton* button, Workspace* workspace) {    
    workspace->control->openFolder();
}

void Workspace::buttonCloseWorkspaceClicked(GtkButton* button, Workspace* workspace) {
    workspace->control->closeAllFolders(false);
}

void Workspace::fillTreeFromFolderPath(GtkTreeStore* store, GtkTreeIter* parent, std::string folderPath) {

    if (!fs::is_directory(fs::path(folderPath))) {
        g_warning("The workspace folder path %s does not exist", folderPath.c_str());
        return;
    }

    const std::set<std::string> exts = {".xopp", ".xoj", ".pdf", ".xopt", ".moj"};

    // to order the filenames
    std::set<fs::directory_entry> entries;
    for (const auto& entry : fs::directory_iterator(folderPath))
        entries.insert(entry);
    
    for (const auto& entry : entries) {

        fs::path entryPath = entry.path();
        fs::path entryFilename = entryPath.filename();
        
        GtkTreeIter iter;
        if (entry.is_directory() || exts.find(entryPath.extension()) != exts.end()) {
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

    gtk_widget_set_sensitive(gui->get("menuCloseAllFoldersWorkspace"), true);

    gtk_widget_show_all(GTK_WIDGET(workspaceSidebar));
}

void Workspace::closeAllFolders() {

    gtk_tree_store_clear(store);

    //gtk_widget_set_sensitive(gui->get("menuCloseAllFoldersWorkspace"), false);

    gtk_widget_show_all(GTK_WIDGET(workspaceSidebar));
}

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
            workspace->control->close(true, true);
            workspace->control->openFile(fsPath);
        }
        
        g_free(str);
    }

}

void Workspace::saveSize() {
    if (this->control->getSettings()->isWorkspaceVisible()) {
        GtkAllocation alloc;
        gtk_widget_get_allocation(this->workspaceSidebar, &alloc);

        this->control->getSettings()->setWorkspaceWidth(alloc.width);
    }
}
