/*
 * Xournal++
 *
 * The Workspace
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>   // for string
#include <vector>   // for vector

#include <gtk/gtk.h>  // for GtkWidget, Gtk...
#include "control/workspace/WorkspaceHandlerListener.h"

class Control;
class GladeGui;
class WorkspaceHandler;

class Workspace:
    public WorkspaceHandlerListener {
public:
    Workspace(GladeGui* gui, Control* control, WorkspaceHandler* workspaceHandler);
    ~Workspace();

    Control* getControl();

public:
    void addFolder(fs::path folderPath);
    void closeAllFolders();

    void saveSize();

private:
    void createViewAndStore();
    void fillTreeFromFolderPath(GtkTreeStore* store, GtkTreeIter* parent, std::string folderPath);

    static void buttonAddFolderWorkspaceClicked(GtkButton* button, Workspace* workspace);
    static void buttonCloseWorkspaceClicked(GtkButton* button, Workspace* workspace);
    
    static void treeViewRowClicked(GtkTreeView* treeView, GtkTreePath* path, GtkTreeViewColumn* column, Workspace* workspace);
    
    Control* control = nullptr;

    GladeGui* gui = nullptr;

    /**
     * The workspaceSidebar widget
     */
    GtkWidget* workspaceSidebar = nullptr;
    
    /**
     * The GTK tree view widget
     */
    GtkWidget* treeView = nullptr;

    WorkspaceHandler* workspaceHandler = nullptr;

    GtkTreeStore* store;
};

