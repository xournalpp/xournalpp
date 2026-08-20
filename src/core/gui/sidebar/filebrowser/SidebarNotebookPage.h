/*
 * Xournal++
 *
 * Sidebar Notebook Page
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>

#include <gtk/gtk.h>

#include "gui/IconNameHelper.h"
#include "gui/sidebar/AbstractSidebarPage.h"
#include "util/raii/GtkWindowUPtr.h"

class Control;

class SidebarNotebookPage: public AbstractSidebarPage {
public:
    SidebarNotebookPage(Control* control);
    ~SidebarNotebookPage() override;

public:
    void enableSidebar() override;
    void disableSidebar() override;

    void layout() override;

    std::string getName() override;
    std::string getIconName() override;
    bool hasData() override;
    GtkWidget* getWidget() override;

    void updateList();

private:
    static void onRowActivated(GtkTreeView* treeView, GtkTreePath* path, GtkTreeViewColumn* column, SidebarNotebookPage* sidebar);
    static void onNewNotebook(GtkButton* button, SidebarNotebookPage* sidebar);

    IconNameHelper iconNameHelper;
    GtkWidget* mainBox;
    GtkWidget* scrolledWindow;
    GtkWidget* treeView;
    GtkTreeStore* treeStore;

    enum Columns {
        COLUMN_NAME,
        COLUMN_FILEPATH,
        NUM_COLUMNS
    };
};
