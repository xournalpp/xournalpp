/*
 * Xournal++
 *
 * Toolbar edit dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <functional>

#include <glib.h>     // for gchar
#include <gtk/gtk.h>  // for GtkButton, GtkCellRendererText, GtkListStore

#include "util/raii/GObjectSPtr.h"
#include "util/raii/GtkWindowUPtr.h"

class ToolbarData;
class ToolbarModel;
class GladeSearchpath;

class ToolbarManageDialog {
public:
    ToolbarManageDialog(GladeSearchpath* gladeSearchPath, ToolbarModel* model, std::function<void()> callback);

public:
    inline GtkWindow* getWindow() const { return window.get(); }

private:
    static void treeSelectionChangedCallback(GtkTreeSelection* selection, ToolbarManageDialog* dlg);
    static void treeCellEditedCallback(GtkCellRendererText* renderer, gchar* pathString, gchar* newText,
                                       ToolbarManageDialog* dlg);

    static void buttonNewCallback(GtkButton* button, ToolbarManageDialog* dlg);
    static void buttonDeleteCallback(GtkButton* button, ToolbarManageDialog* dlg);
    static void buttonCopyCallback(GtkButton* button, ToolbarManageDialog* dlg);

    void addToolbarData(ToolbarData* data);
    void entrySelected(ToolbarData* data);

    void updateSelectionData();

    ToolbarData* getSelectedEntry();

private:
    ToolbarModel* tbModel;
    xoj::util::GObjectSPtr<GtkListStore> model;

    GtkTreeView* tree;
    GtkWidget* copyButton;
    GtkWidget* deleteButton;

    xoj::util::GtkWindowUPtr window;
    std::function<void()> callback;
};
