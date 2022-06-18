/*
 * Xournal++
 *
 * Index Sidebar Page
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>  // for size_t
#include <string>   // for string

#include <glib.h>     // for gboolean, gchar, gint
#include <gtk/gtk.h>  // for GtkWidget, GtkTreeIter

#include "gui/IconNameHelper.h"               // for IconNameHelper
#include "gui/sidebar/AbstractSidebarPage.h"  // for AbstractSidebarPage
#include "model/DocumentChangeType.h"         // for DocumentChangeType

class Control;
class SidebarToolbar;

class SidebarIndexPage: public AbstractSidebarPage {
public:
    SidebarIndexPage(Control* control, SidebarToolbar* toolbar);
    ~SidebarIndexPage() override;

public:
    void enableSidebar() override;
    void disableSidebar() override;

    /**
     * @overwrite
     */
    std::string getName() override;

    /**
     * @overwrite
     */
    std::string getIconName() override;

    /**
     * @overwrite
     */
    bool hasData() override;

    /**
     * @overwrite
     */
    GtkWidget* getWidget() override;

    /**
     * @overwrite
     */
    void selectPageNr(size_t page, size_t pdfPage) override;

    /**
     * Select page in the tree
     */
    bool selectPageNr(size_t page, size_t pdfPage, GtkTreeIter* parent);

    /**
     * @overwrite
     */
    void documentChanged(DocumentChangeType type) override;

private:
    /**
     * Tree search function if you type chars within the tree. Source: Pidgin
     */
    static gboolean treeSearchFunction(GtkTreeModel* model, gint column, const gchar* key, GtkTreeIter* iter,
                                       SidebarIndexPage* sidebar);

    /**
     * A bookmark was selected
     */
    static bool treeBookmarkSelected(GtkWidget* treeview, SidebarIndexPage* sidebar);

    /**
     * If you select a Bookmark which is currently not in the Xournal document, only in the PDF (page deleted or so)
     */
    void askInsertPdfPage(size_t pdfPage);

    /**
     * The function which is called after a search timeout
     */
    static bool searchTimeoutFunc(SidebarIndexPage* sidebar);

private:
    /**
     * Expand links
     */
    int expandOpenLinks(GtkTreeModel* model, GtkTreeIter* parent);

private:
    /**
     * The Tree with the Bookmarks
     */
    GtkWidget* treeViewBookmarks = nullptr;

    /**
     * The scrollbars for the Tree
     */
    GtkWidget* scrollBookmarks = nullptr;

    /**
     * Keep track of the tree bookmark selection handler; see documentChanged
     * method for why this is necessary.
     */
    unsigned long selectHandler = 0;  // g_signal_connect uses 0 as error value

    /**
     * If currently searching, scroll to the page is disable, else search is not really working
     *
     * After a timeout we scroll to the selected page
     */
    int searchTimeout = 0;

    /**
     * If there is something to display in the tree
     */
    bool hasContents = false;

    IconNameHelper iconNameHelper;
};
