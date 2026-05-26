#include "SidebarBookmarks.h"
#include <glib/gi18n.h> // For the _() macro
#include <gtk/gtk.h>

#include "control/Control.h" // for Control
#include "control/ScrollHandler.h" // for ScrollHandler
#include "undo/BookmarkUndoAction.h" // for BookmarkUndoAction
#include "gui/sidebar/Sidebar.h"
#include "model/XojPage.h"  // for XojPage
#include "model/PageRef.h"  // for PageRef
#include "model/Document.h"  // for Document

static auto makeIconButton(const char* iconName, const char* tooltip) -> GtkWidget* {
    GtkWidget* button = gtk_button_new();
    GtkWidget* image = gtk_image_new_from_icon_name(iconName, GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_visible(image, TRUE);
    gtk_widget_set_can_focus(image, FALSE);
    gtk_container_add(GTK_CONTAINER(button), image);
    gtk_widget_set_tooltip_text(button, tooltip);
    return button;
}

SidebarBookmarks::SidebarBookmarks(Control* control):
    AbstractSidebarPage(control),
    iconNameHelper(control->getSettings())
{
    mainBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    scrolledWindow = gtk_scrolled_window_new(nullptr, nullptr);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledWindow),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);

    // Claim all available vertical space
    gtk_widget_set_vexpand(scrolledWindow, TRUE);

    listStore = gtk_list_store_new(NUM_COLUMNS, G_TYPE_STRING, G_TYPE_INT);
    treeView  = gtk_tree_view_new_with_model(GTK_TREE_MODEL(listStore));
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(treeView), FALSE);
    gtk_tree_view_set_activate_on_single_click(GTK_TREE_VIEW(treeView), TRUE);

    // Create a column to hold the label and the page number
    GtkTreeViewColumn* column = gtk_tree_view_column_new();
    gtk_tree_view_column_set_expand(GTK_TREE_VIEW_COLUMN(column), TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeView), column);

    auto* rendererLabel = static_cast<GtkCellRenderer*>(
            g_object_new(GTK_TYPE_CELL_RENDERER_TEXT, "ellipsize", PANGO_ELLIPSIZE_END, nullptr));
    gtk_tree_view_column_pack_start(GTK_TREE_VIEW_COLUMN(column), rendererLabel, TRUE);
    gtk_tree_view_column_set_attributes(GTK_TREE_VIEW_COLUMN(column), rendererLabel,
                                        "text", COLUMN_LABEL, nullptr);

    GtkCellRenderer* rendererPage = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_end(GTK_TREE_VIEW_COLUMN(column), rendererPage, FALSE);
    gtk_tree_view_column_set_attributes(GTK_TREE_VIEW_COLUMN(column), rendererPage,
                                        "text", COLUMN_PAGE_NUM, nullptr);
    g_object_set(G_OBJECT(rendererPage), "style", PANGO_STYLE_ITALIC, nullptr);

    gtk_container_add(GTK_CONTAINER(scrolledWindow), treeView);
    gtk_box_pack_start(GTK_BOX(mainBox), scrolledWindow, TRUE, TRUE, 0);

    // Bottom buttons
    GtkWidget* buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(buttons), "bottom_buttons");
    gtk_style_context_add_class(gtk_widget_get_style_context(buttons), "linked");

    buttonAdd = makeIconButton("bookmark-new", nullptr);
    buttonEdit = makeIconButton("document-edit-symbolic", nullptr);
    buttonDelete = makeIconButton("edit-delete", nullptr);

    gtk_box_pack_start(GTK_BOX(buttons), buttonAdd, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(buttons), buttonEdit, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(buttons), buttonDelete, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(mainBox), buttons, FALSE, FALSE, 0);

    updateButtonSensitivity();

    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeView));
    g_signal_connect(selection, "changed", G_CALLBACK(onSelectionChanged), this);

    g_signal_connect(buttonAdd, "clicked", G_CALLBACK(onAddClicked), this);
    g_signal_connect(buttonEdit, "clicked", G_CALLBACK(onEditClicked), this);
    g_signal_connect(buttonDelete, "clicked", G_CALLBACK(onDeleteClicked), this);

    g_signal_connect(treeView, "row-activated", G_CALLBACK(onRowActivated), this);

    gtk_widget_show_all(mainBox);

    registerListener(control);
}

SidebarBookmarks::~SidebarBookmarks() {
    if (idleRefreshId != 0) {
        g_source_remove(idleRefreshId);
        idleRefreshId = 0;
    }

    if (listStore) {
        g_object_unref(listStore);
        listStore = nullptr;
    }
}



auto SidebarBookmarks::getWidget() -> GtkWidget* {
    return mainBox;
}

auto SidebarBookmarks::getName() -> std::string {
    return _("Bookmarks");
}

auto SidebarBookmarks::getIconName() -> std::string  {
    return iconNameHelper.iconName("bookmark");
}

void SidebarBookmarks::enableSidebar() {
    refresh();
}

void SidebarBookmarks::disableSidebar() {}

void SidebarBookmarks::layout() {}

auto SidebarBookmarks::hasData() -> bool {
    return hasBookmarks;
}



void SidebarBookmarks::onSelectionChanged(GtkTreeSelection* /*selection*/,
                                          SidebarBookmarks* self) {
    self->updateButtonSensitivity();
}

void SidebarBookmarks::updateButtonSensitivity() {
    PageRef page = control->getCurrentPage();
    Document* doc = control->getDocument();
    doc->lock_shared();
    bool bookmarkPresent = page && page->getBookmark().has_value();
    doc->unlock_shared();

    gtk_widget_set_sensitive(buttonAdd, !bookmarkPresent);

    // Edit and Delete buttons enabled when a row is selected
    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeView));
    gboolean hasSelection = gtk_tree_selection_count_selected_rows(selection) > 0;
    gtk_widget_set_sensitive(buttonEdit, hasSelection);
    gtk_widget_set_sensitive(buttonDelete, hasSelection);

    control->updatePageActions();
}



void SidebarBookmarks::onAddClicked(GtkButton* /*button*/, SidebarBookmarks* self) {
    Control* control = self->getControl();
    size_t pageIndex = control->getCurrentPageNo();

    control->setBookmark(pageIndex);
}

void SidebarBookmarks::onEditClicked(GtkButton* /*button*/, SidebarBookmarks* self) {
    self->editOrDeleteSelectedBookmark(EDIT);
}

void SidebarBookmarks::onDeleteClicked(GtkButton* button, SidebarBookmarks* self) {
    self->editOrDeleteSelectedBookmark(DELETE);
}

void SidebarBookmarks::editOrDeleteSelectedBookmark(EditOrDelete mode) {
    GtkTreeSelection* select = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeView));
    GtkTreeModel* model = nullptr;
    GtkTreeIter iter;

    if (!gtk_tree_selection_get_selected(select, &model, &iter)) return;

    gchar* label = nullptr;
    gint pageNum = 0;

    // Get the page number of the selected row
    gtk_tree_model_get(model, &iter, COLUMN_LABEL, &label, COLUMN_PAGE_NUM, &pageNum, -1);
    g_free(label);

    size_t pageIndex = static_cast<size_t>(pageNum) - 1;

    if (mode == EDIT) {
        control->setBookmark(pageIndex);
    } else {
        control->deleteBookmark(pageIndex);
    }
}



void SidebarBookmarks::onRowActivated(GtkTreeView* /*treeView*/, GtkTreePath* /*path*/,
                                      GtkTreeViewColumn* /*column*/, SidebarBookmarks* self) {
    self->navigateToSelectedBookmark();
}

void SidebarBookmarks::navigateToSelectedBookmark() {
    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeView));
    GtkTreeModel* model = nullptr;
    GtkTreeIter iter;
    if (!gtk_tree_selection_get_selected(selection, &model, &iter)) return;

    gint pageNum = 0;
    gtk_tree_model_get(model, &iter, COLUMN_PAGE_NUM, &pageNum, -1);

    control->getScrollHandler()->scrollToPage(static_cast<size_t>(pageNum - 1));
}



void SidebarBookmarks::documentChanged(DocumentChangeType type) {
    if (type == DOCUMENT_CHANGE_CLEARED) {
        gtk_list_store_clear(listStore);
        updateButtonSensitivity();

    } else if (type == DOCUMENT_CHANGE_COMPLETE || type == DOCUMENT_CHANGE_BOOKMARKS ||
        type == DOCUMENT_CHANGE_NO_BOOKMARKS) {

        refresh();
    }
}

void SidebarBookmarks::pageInserted(size_t /*pageIndex*/) {
    refresh();
}

void SidebarBookmarks::pageDeleted(size_t /*pageIndex*/) {
    refresh();
}

void SidebarBookmarks::pageSelected(size_t pageIndex) {
    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeView));
    GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));

    GtkTreeIter iter;
    gboolean valid = gtk_tree_model_get_iter_first(model, &iter);

    while (valid) {
        gint pageNum = 0;
        gtk_tree_model_get(model, &iter, COLUMN_PAGE_NUM, &pageNum, -1);

        if (pageNum > 0 && static_cast<size_t>(pageNum - 1) == pageIndex) {
            gtk_tree_selection_unselect_all(selection);
            gtk_tree_selection_select_iter(selection, &iter);
            break;
        }

        valid = gtk_tree_model_iter_next(model, &iter);
    }

    updateButtonSensitivity();
}



void SidebarBookmarks::refresh() {
    if (idleRefreshId == 0) {
        idleRefreshId = g_idle_add(idleRefreshCallback, this);
    }
}

auto SidebarBookmarks::idleRefreshCallback(gpointer data) -> gboolean {
    auto* self = static_cast<SidebarBookmarks*>(data);

    self->idleRefreshId = 0;
    self->doRefresh();

    return G_SOURCE_REMOVE;
}

void SidebarBookmarks::doRefresh() {
    GtkTreeSelection* sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeView));
    gulong handlerId = g_signal_handler_find(
        sel, G_SIGNAL_MATCH_FUNC, 0, 0, nullptr, (gpointer)onSelectionChanged, nullptr
    );

    if (handlerId) g_signal_handler_block(sel, handlerId);

    gtk_list_store_clear(listStore);

    Document* doc = control->getDocument();
    doc->lock_shared();
    const auto bookmarks = doc->listBookmarks();
    doc->unlock_shared();

    hasBookmarks = !bookmarks.empty();
    for (const auto& bookmarkPair : bookmarks) {
        GtkTreeIter iter;
        gtk_list_store_append(listStore, &iter);
        gtk_list_store_set(listStore, &iter,
                           COLUMN_PAGE_NUM, bookmarkPair.second + 1,
                           COLUMN_LABEL, bookmarkPair.first.c_str(), -1);
    }

    if (handlerId) g_signal_handler_unblock(sel, handlerId);

    updateButtonSensitivity();

    if (tabButton) {
        gtk_widget_set_visible(GTK_WIDGET(tabButton), hasBookmarks);
    }
}
