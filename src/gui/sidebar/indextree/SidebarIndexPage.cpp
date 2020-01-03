#include "SidebarIndexPage.h"

#include <config.h>

#include "control/Control.h"
#include "model/LinkDestination.h"
#include "model/XojPage.h"

#include "Util.h"
#include "i18n.h"

SidebarIndexPage::SidebarIndexPage(Control* control, SidebarToolbar* toolbar): AbstractSidebarPage(control, toolbar) {
    this->treeViewBookmarks = gtk_tree_view_new();
    g_object_ref(this->treeViewBookmarks);

    gtk_tree_view_set_enable_search(GTK_TREE_VIEW(treeViewBookmarks), true);
    gtk_tree_view_set_search_column(GTK_TREE_VIEW(treeViewBookmarks), DOCUMENT_LINKS_COLUMN_NAME);
    gtk_tree_view_set_search_equal_func(GTK_TREE_VIEW(treeViewBookmarks),
                                        reinterpret_cast<GtkTreeViewSearchEqualFunc>(treeSearchFunction), this,
                                        nullptr);

    this->scrollBookmarks = gtk_scrolled_window_new(nullptr, nullptr);
    g_object_ref(this->scrollBookmarks);

    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollBookmarks), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrollBookmarks), GTK_SHADOW_IN);

    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeViewBookmarks));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(treeViewBookmarks), false);
    gtk_container_add(GTK_CONTAINER(scrollBookmarks), treeViewBookmarks);


    GtkTreeViewColumn* column = gtk_tree_view_column_new();
    gtk_tree_view_column_set_expand(GTK_TREE_VIEW_COLUMN(column), true);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeViewBookmarks), column);

    auto* renderer = static_cast<GtkCellRenderer*>(
            g_object_new(GTK_TYPE_CELL_RENDERER_TEXT, "ellipsize", PANGO_ELLIPSIZE_END, nullptr));
    gtk_tree_view_column_pack_start(GTK_TREE_VIEW_COLUMN(column), renderer, true);
    gtk_tree_view_column_set_attributes(GTK_TREE_VIEW_COLUMN(column), renderer, "markup", DOCUMENT_LINKS_COLUMN_NAME,
                                        nullptr);

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_end(GTK_TREE_VIEW_COLUMN(column), renderer, false);
    gtk_tree_view_column_set_attributes(GTK_TREE_VIEW_COLUMN(column), renderer, "text",
                                        DOCUMENT_LINKS_COLUMN_PAGE_NUMBER, nullptr);
    g_object_set(G_OBJECT(renderer), "style", PANGO_STYLE_ITALIC, nullptr);

    g_signal_connect(treeViewBookmarks, "cursor-changed", G_CALLBACK(treeBookmarkSelected), this);

    gtk_widget_show(this->treeViewBookmarks);

    registerListener(control);
}

SidebarIndexPage::~SidebarIndexPage() {
    if (this->searchTimeout) {
        g_source_remove(this->searchTimeout);
        this->searchTimeout = 0;
    }

    g_object_unref(this->treeViewBookmarks);
    g_object_unref(this->scrollBookmarks);
}

void SidebarIndexPage::enableSidebar() { toolbar->setHidden(true); }

void SidebarIndexPage::disableSidebar() {
    // Nothing to do at the moment
}

void SidebarIndexPage::askInsertPdfPage(size_t pdfPage) {
    GtkWidget* dialog = gtk_message_dialog_new(control->getGtkWindow(), GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION,
                                               GTK_BUTTONS_NONE, "%s",
                                               FC(_F("Your current document does not contain PDF Page no {1}\n"
                                                     "Would you like to insert this page?\n\n"
                                                     "Tip: You can select Journal → Paper Background → PDF Background "
                                                     "to insert a PDF page.") %
                                                  (pdfPage + 1)));

    gtk_dialog_add_button(GTK_DIALOG(dialog), "Cancel", 1);
    gtk_dialog_add_button(GTK_DIALOG(dialog), "Insert after", 2);
    gtk_dialog_add_button(GTK_DIALOG(dialog), "Insert at end", 3);

    gtk_window_set_transient_for(GTK_WINDOW(dialog), control->getGtkWindow());
    int res = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    if (res == 1) {
        return;
    }

    int position = 0;

    Document* doc = control->getDocument();

    if (res == 2) {
        position = control->getCurrentPageNo() + 1;
    } else if (res == 3) {
        position = doc->getPageCount();
    }

    doc->lock();
    XojPdfPageSPtr pdf = doc->getPdfPage(pdfPage);
    doc->unlock();

    if (pdf) {
        PageRef page = new XojPage(pdf->getWidth(), pdf->getHeight());
        page->setBackgroundPdfPageNr(pdfPage);
        control->insertPage(page, position);
    }
}

auto SidebarIndexPage::treeBookmarkSelected(GtkWidget* treeview, SidebarIndexPage* sidebar) -> bool {
    if (sidebar->searchTimeout) {
        return false;
    }

    gtk_widget_grab_focus(GTK_WIDGET(treeview));

    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

    if (selection) {
        GtkTreeModel* model = nullptr;
        GtkTreeIter iter = {0};

        if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
            XojLinkDest* link = nullptr;

            gtk_tree_model_get(model, &iter, DOCUMENT_LINKS_COLUMN_LINK, &link, -1);
            if (link && link->dest) {
                LinkDestination* dest = link->dest;

                size_t pdfPage = dest->getPdfPage();

                if (pdfPage != npos) {
                    Document* doc = sidebar->control->getDocument();
                    doc->lock();
                    size_t page = doc->findPdfPage(pdfPage);
                    doc->unlock();

                    if (page == npos) {
                        sidebar->askInsertPdfPage(pdfPage);
                    } else {
                        if (dest->shouldChangeTop()) {
                            sidebar->control->getScrollHandler()->scrollToPage(
                                    page, dest->getTop() * sidebar->control->getZoomControl()->getZoom());
                        } else {
                            if (sidebar->control->getCurrentPageNo() != page) {
                                sidebar->control->getScrollHandler()->scrollToPage(page);
                            }
                        }
                    }
                }
            }
            g_object_unref(link);

            return true;
        }
    }
    return false;
}

auto SidebarIndexPage::searchTimeoutFunc(SidebarIndexPage* sidebar) -> bool {
    sidebar->searchTimeout = 0;

    treeBookmarkSelected(sidebar->treeViewBookmarks, sidebar);

    return false;
}

auto SidebarIndexPage::treeSearchFunction(GtkTreeModel* model, gint column, const gchar* key, GtkTreeIter* iter,
                                          SidebarIndexPage* sidebar) -> gboolean {
    if (sidebar->searchTimeout) {
        g_source_remove(sidebar->searchTimeout);
        sidebar->searchTimeout = 0;
    }
    sidebar->searchTimeout = g_timeout_add_seconds_full(
            G_PRIORITY_DEFAULT_IDLE, 2, reinterpret_cast<GSourceFunc>(searchTimeoutFunc), sidebar, nullptr);

    // Source: Pidgin
    gchar* text = nullptr;
    gtk_tree_model_get(model, iter, DOCUMENT_LINKS_COLUMN_NAME, &text, -1);
    if (text == nullptr) {
        return true;
    }

    gchar* tmp = g_utf8_normalize(key, -1, G_NORMALIZE_DEFAULT);
    gchar* enteredstring = g_utf8_casefold(tmp, -1);
    g_free(tmp);

    tmp = g_utf8_normalize(text, -1, G_NORMALIZE_DEFAULT);
    gchar* normalized = g_utf8_casefold(tmp, -1);
    g_free(tmp);

    if (g_str_has_prefix(normalized, enteredstring)) {
        g_free(enteredstring);
        g_free(normalized);
        return false;
    }

    /* Use Pango to separate by words. */
    size_t len = g_utf8_strlen(normalized, -1);
    PangoLogAttr* log_attrs = g_new(PangoLogAttr, len + 1);

    pango_get_log_attrs(normalized, strlen(normalized), -1, nullptr, log_attrs, len + 1);

    gchar* word = normalized;
    gboolean result = true;
    for (size_t i = 0; i < (len - 1); i++) {
        if (log_attrs[i].is_word_start && g_str_has_prefix(word, enteredstring)) {
            result = false;
            break;
        }
        word = g_utf8_next_char(word);
    }
    g_free(log_attrs);

    g_free(enteredstring);
    g_free(normalized);

    return result;
}

auto SidebarIndexPage::getName() -> string { return _("Contents"); }

auto SidebarIndexPage::getIconName() -> string { return "sidebar_index"; }

auto SidebarIndexPage::hasData() -> bool { return this->hasContents; }

auto SidebarIndexPage::getWidget() -> GtkWidget* { return this->scrollBookmarks; }

auto SidebarIndexPage::expandOpenLinks(GtkTreeModel* model, GtkTreeIter* parent) -> int {
    GtkTreeIter iter = {0};
    XojLinkDest* link = nullptr;
    if (model == nullptr) {
        return 0;
    }

    int count = 0;

    if (gtk_tree_model_iter_children(model, &iter, parent)) {
        do {
            gtk_tree_model_get(model, &iter, DOCUMENT_LINKS_COLUMN_LINK, &link, -1);

            if (link->dest->getExpand()) {
                GtkTreePath* path = gtk_tree_model_get_path(model, &iter);
                gtk_tree_view_expand_row(GTK_TREE_VIEW(treeViewBookmarks), path, false);
                gtk_tree_path_free(path);
            }

            g_object_unref(link);

            count++;

            count += expandOpenLinks(model, &iter);
        } while (gtk_tree_model_iter_next(model, &iter));
    }

    return count;
}

void SidebarIndexPage::selectPageNr(size_t page, size_t pdfPage) { selectPageNr(page, pdfPage, nullptr); }

auto SidebarIndexPage::selectPageNr(size_t page, size_t pdfPage, GtkTreeIter* parent) -> bool {
    GtkTreeIter iter;

    Document* doc = control->getDocument();
    doc->lock();
    GtkTreeModel* model = doc->getContentsModel();
    if (model == nullptr) {
        doc->unlock();
        return false;
    }

    g_object_ref(model);
    doc->unlock();

    if (parent == nullptr) {

        // check if there is already the current page selected
        GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeViewBookmarks));

        if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
            XojLinkDest* link = nullptr;

            gtk_tree_model_get(model, &iter, DOCUMENT_LINKS_COLUMN_LINK, &link, -1);

            if (link && link->dest) {
                LinkDestination* dest = link->dest;

                if (dest->getPdfPage() == pdfPage) {

                    g_object_unref(model);
                    g_object_unref(link);

                    // already a bookmark from this page selected
                    return true;
                }
            }

            g_object_unref(link);
        }
    }

    gboolean valid = gtk_tree_model_iter_children(model, &iter, parent);

    while (valid) {
        XojLinkDest* link = nullptr;

        gtk_tree_model_get(model, &iter, DOCUMENT_LINKS_COLUMN_LINK, &link, -1);

        if (link->dest->getPdfPage() == pdfPage) {
            GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeViewBookmarks));
            gtk_tree_selection_select_iter(selection, &iter);

            g_object_unref(link);
            g_object_unref(model);
            return true;
        }


        g_object_unref(link);

        if (selectPageNr(page, pdfPage, &iter)) {
            g_object_unref(model);
            return true;
        }


        valid = gtk_tree_model_iter_next(model, &iter);
    }

    g_object_unref(model);
    return false;
}

void SidebarIndexPage::documentChanged(DocumentChangeType type) {
    if (type == DOCUMENT_CHANGE_CLEARED) {
        gtk_tree_view_set_model(GTK_TREE_VIEW(this->treeViewBookmarks), nullptr);
    } else if (type == DOCUMENT_CHANGE_PDF_BOOKMARKS || type == DOCUMENT_CHANGE_COMPLETE) {

        Document* doc = this->control->getDocument();

        doc->lock();
        GtkTreeModel* model = doc->getContentsModel();
        gtk_tree_view_set_model(GTK_TREE_VIEW(this->treeViewBookmarks), model);
        int count = expandOpenLinks(model, nullptr);
        doc->unlock();

        hasContents = (count != 0);
    }
}
