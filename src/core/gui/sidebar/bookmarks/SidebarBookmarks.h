/*
 * Xournal++
 *
 * Bookmarks Sidebar Page
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>
#include <cstdint>
#include <string>

#include "gui/IconNameHelper.h"
#include "gui/sidebar/AbstractSidebarPage.h"

class Control;

class SidebarBookmarks : public AbstractSidebarPage {
public:
    SidebarBookmarks(Control* control);
    ~SidebarBookmarks() override;

    // AbstractSidebarPage
    auto getWidget() -> GtkWidget* override;
    auto getName() -> std::string override;
    auto getIconName() -> std::string override;
    void enableSidebar() override;
    void disableSidebar() override;
    void layout() override;
    auto hasData() -> bool override;

    // DocumentListener
    void documentChanged(DocumentChangeType type) override;
    void pageInserted(size_t page) override;
    void pageDeleted(size_t page) override;
    void pageSelected(size_t page) override;

    void refresh();

private:
    static void onSelectionChanged(GtkTreeSelection* selection, SidebarBookmarks* self);
    static void onAddClicked(GtkButton* button, SidebarBookmarks* self);
    static void onEditClicked(GtkButton* button, SidebarBookmarks* self);
    static void onDeleteClicked(GtkButton* button, SidebarBookmarks* self);
    static void onRowActivated(GtkTreeView* treeView, GtkTreePath* path,
                               GtkTreeViewColumn* column, SidebarBookmarks* self);

    void updateButtonSensitivity();
    void navigateToSelectedBookmark();

    enum EditOrDelete : std::uint8_t {
        EDIT, DELETE
    };

    void editOrDeleteSelectedBookmark(EditOrDelete mode);

    bool hasBookmarks = false;

    static constexpr int COLUMN_LABEL = 0;
    static constexpr int COLUMN_PAGE_NUM = 1;
    static constexpr int NUM_COLUMNS = 2;

    GtkWidget* mainBox = nullptr;
    GtkWidget* scrolledWindow = nullptr;
    GtkListStore* listStore = nullptr;
    GtkWidget* treeView = nullptr;

    GtkWidget* buttonAdd = nullptr;
    GtkWidget* buttonEdit = nullptr;
    GtkWidget* buttonDelete = nullptr;

    IconNameHelper iconNameHelper;

    guint idleRefreshId = 0;
    static auto idleRefreshCallback(gpointer data) -> gboolean;
    void doRefresh();
};
