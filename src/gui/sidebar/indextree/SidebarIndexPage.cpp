#include "SidebarIndexPage.h"
#include "../../../model/LinkDestination.h"
#include "../../../model/XojPage.h"
#include "../../../control/Control.h"

#include <config.h>
#include <glib/gi18n-lib.h>

SidebarIndexPage::SidebarIndexPage(Control * control) : AbstractSidebarPage(control) {
	XOJ_INIT_TYPE(SidebarIndexPage);

	this->searchTimeout = 0;
	this->hasContents = false;

	this->treeViewBookmarks = gtk_tree_view_new();
	g_object_ref(this->treeViewBookmarks);

	gtk_tree_view_set_enable_search(GTK_TREE_VIEW(treeViewBookmarks), true);
	gtk_tree_view_set_search_column(GTK_TREE_VIEW(treeViewBookmarks), DOCUMENT_LINKS_COLUMN_NAME);
	gtk_tree_view_set_search_equal_func(GTK_TREE_VIEW(treeViewBookmarks), (GtkTreeViewSearchEqualFunc) treeSearchFunction, this, NULL);

	this->scrollBookmarks = gtk_scrolled_window_new(NULL, NULL);
	g_object_ref(this->scrollBookmarks);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollBookmarks), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrollBookmarks), GTK_SHADOW_IN);

	GtkTreeSelection * selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeViewBookmarks));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(treeViewBookmarks), FALSE);
	gtk_container_add(GTK_CONTAINER(scrollBookmarks), treeViewBookmarks);


	GtkTreeViewColumn * column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_expand(GTK_TREE_VIEW_COLUMN(column), TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeViewBookmarks), column);

	GtkCellRenderer * renderer = (GtkCellRenderer*) g_object_new(GTK_TYPE_CELL_RENDERER_TEXT, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_column_pack_start(GTK_TREE_VIEW_COLUMN(column), renderer, TRUE);
	gtk_tree_view_column_set_attributes(GTK_TREE_VIEW_COLUMN(column), renderer, "markup", DOCUMENT_LINKS_COLUMN_NAME, NULL);

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_end(GTK_TREE_VIEW_COLUMN(column), renderer, FALSE);
	gtk_tree_view_column_set_attributes(GTK_TREE_VIEW_COLUMN(column), renderer, "text", DOCUMENT_LINKS_COLUMN_PAGE_NUMBER, NULL);
	g_object_set(G_OBJECT(renderer), "style", PANGO_STYLE_ITALIC, NULL);

	g_signal_connect(treeViewBookmarks, "cursor-changed", G_CALLBACK(treeBookmarkSelected), this);

	gtk_widget_show(this->treeViewBookmarks);

	registerListener(control);
}

SidebarIndexPage::~SidebarIndexPage() {
	XOJ_RELEASE_TYPE(SidebarIndexPage);

	if (this->searchTimeout) {
		g_source_remove(this->searchTimeout);
		this->searchTimeout = 0;
	}

	gtk_widget_unref(this->treeViewBookmarks);
	gtk_widget_unref(this->scrollBookmarks);
}

void SidebarIndexPage::askInsertPdfPage(int pdfPage) {
	XOJ_CHECK_TYPE(SidebarIndexPage);

	GtkWidget * dialog = gtk_message_dialog_new((GtkWindow*) *control->getWindow(), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
			_("Your current document does not contain PDF Page %i\n"
				"Would you insert this page?\n\nTipp: You can select Journal / Paper Background / PDF Background to insert a PDF page."), pdfPage + 1);

	gtk_dialog_add_button(GTK_DIALOG(dialog), "Cancel", 1);
	gtk_dialog_add_button(GTK_DIALOG(dialog), "Insert after", 2);
	gtk_dialog_add_button(GTK_DIALOG(dialog), "Insert at end", 3);

	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->control->getWindow()->getWindow()));
	int res = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	if (res == 1) {
		return;
	}

	int position = 0;

	Document * doc = control->getDocument();

	if (res == 2) {
		position = control->getCurrentPageNo() + 1;
	} else if (res == 3) {
		position = doc->getPageCount();
	}

	doc->lock();
	XojPopplerPage * pdf = doc->getPdfPage(pdfPage);
	doc->unlock();

	if (pdf) {
		PageRef page = new XojPage(pdf->getWidth(), pdf->getHeight());
		page.setBackgroundPdfPageNr(pdfPage);
		control->insertPage(page, position);
	}
}

bool SidebarIndexPage::treeBookmarkSelected(GtkWidget * treeview, SidebarIndexPage * sidebar) {
	XOJ_CHECK_TYPE_OBJ(sidebar, SidebarIndexPage);

	if (sidebar->searchTimeout) {
		return false;
	}

	gtk_widget_grab_focus(GTK_WIDGET(treeview));

	GtkTreeSelection * selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

	if (selection) {
		GtkTreeModel *model = NULL;
		GtkTreeIter iter = { 0 };

		if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
			XojLinkDest * link = NULL;

			gtk_tree_model_get(model, &iter, DOCUMENT_LINKS_COLUMN_LINK, &link, -1);
			if (link && link->dest) {
				LinkDestination * dest = link->dest;

				int pdfPage = dest->getPdfPage();

				if (pdfPage >= 0) {
					Document * doc = sidebar->control->getDocument();
					doc->lock();
					int page = doc->findPdfPage(pdfPage);
					doc->unlock();

					if (page == -1) {
						sidebar->askInsertPdfPage(pdfPage);
					} else {
						if (dest->shouldChangeTop()) {
							sidebar->control->getScrollHandler()->scrollToPage(page, dest->getTop());
						} else {
							if (sidebar->control->getCurrentPageNo() != page) {
								sidebar->control->getScrollHandler()->scrollToPage(page);
							}
						}
					}

					// TODO LOW PRIO / OPTIONAL?: Manage scroll to left coordinate and change zoom if dest->shouldChangeZoom()
				}
			}
			g_object_unref(link);

			return true;
		}
	}
	return false;
}


bool SidebarIndexPage::searchTimeoutFunc(SidebarIndexPage * sidebar) {
	XOJ_CHECK_TYPE_OBJ(sidebar, SidebarIndexPage);
	sidebar->searchTimeout = 0;

	treeBookmarkSelected(sidebar->treeViewBookmarks, sidebar);


	return false;
}

gboolean SidebarIndexPage::treeSearchFunction(GtkTreeModel * model, gint column, const gchar * key, GtkTreeIter * iter, SidebarIndexPage * sidebar) {
	XOJ_CHECK_TYPE_OBJ(sidebar, SidebarIndexPage);


	if (sidebar->searchTimeout) {
		g_source_remove(sidebar->searchTimeout);
		sidebar->searchTimeout = 0;
	}
	sidebar->searchTimeout = g_timeout_add_seconds_full(G_PRIORITY_DEFAULT_IDLE, 2, (GSourceFunc) searchTimeoutFunc, sidebar, NULL);


	// Source: Pidgin
	gchar *enteredstring;
	gchar *tmp;
	gchar *text;
	gchar *normalized;
	gboolean result;
	size_t i;
	size_t len;
	PangoLogAttr *log_attrs;
	gchar *word;

	gtk_tree_model_get(model, iter, DOCUMENT_LINKS_COLUMN_NAME, &text, -1);
	if (text == NULL)
		return TRUE;

	tmp = g_utf8_normalize(key, -1, G_NORMALIZE_DEFAULT);
	enteredstring = g_utf8_casefold(tmp, -1);
	g_free(tmp);

	tmp = g_utf8_normalize(text, -1, G_NORMALIZE_DEFAULT);
	normalized = g_utf8_casefold(tmp, -1);
	g_free(tmp);

	if (g_str_has_prefix(normalized, enteredstring)) {
		g_free(enteredstring);
		g_free(normalized);
		return FALSE;
	}

	/* Use Pango to separate by words. */
	len = g_utf8_strlen(normalized, -1);
	log_attrs = g_new(PangoLogAttr, len + 1);

	pango_get_log_attrs(normalized, strlen(normalized), -1, NULL, log_attrs, len + 1);

	word = normalized;
	result = TRUE;
	for (i = 0; i < (len - 1); i++) {
		if (log_attrs[i].is_word_start && g_str_has_prefix(word, enteredstring)) {
			result = FALSE;
			break;
		}
		word = g_utf8_next_char(word);
	}
	g_free(log_attrs);

	g_free(enteredstring);
	g_free(normalized);

	return result;
}

const char * SidebarIndexPage::getName() {
	XOJ_CHECK_TYPE(SidebarIndexPage);

	return _("Contents");
}

const char * SidebarIndexPage::getIconName() {
	XOJ_CHECK_TYPE(SidebarIndexPage);

	return "sidebar_index.png";
}

bool SidebarIndexPage::hasData() {
	XOJ_CHECK_TYPE(SidebarIndexPage);

	return this->hasContents;
}

GtkWidget * SidebarIndexPage::getWidget() {
	XOJ_CHECK_TYPE(SidebarIndexPage);

	return this->scrollBookmarks;
}

int SidebarIndexPage::expandOpenLinks(GtkTreeModel * model, GtkTreeIter * parent) {
	XOJ_CHECK_TYPE(SidebarIndexPage);

	GtkTreeIter iter = { 0 };
	XojLinkDest * link = NULL;
	if (model == NULL) {
		return 0;
	}

	int count = 0;

	if (gtk_tree_model_iter_children(model, &iter, parent)) {
		do {
			gtk_tree_model_get(model, &iter, DOCUMENT_LINKS_COLUMN_LINK, &link, -1);

			if (link->dest->getExpand()) {
				GtkTreePath * path = gtk_tree_model_get_path(model, &iter);
				gtk_tree_view_expand_row(GTK_TREE_VIEW(treeViewBookmarks), path, FALSE);
				gtk_tree_path_free(path);
			}

			g_object_unref(link);

			count++;

			count += expandOpenLinks(model, &iter);
		} while (gtk_tree_model_iter_next(model, &iter));
	}

	return count;
}

bool SidebarIndexPage::selectPageNr(int page, int pdfPage) {
	XOJ_CHECK_TYPE(SidebarIndexPage);

	return selectPageNr(page, pdfPage, NULL);
}

bool SidebarIndexPage::selectPageNr(int page, int pdfPage, GtkTreeIter * parent) {
	XOJ_CHECK_TYPE(SidebarIndexPage);

	GtkTreeIter iter;

	Document * doc = control->getDocument();
	doc->lock();
	GtkTreeModel * model = doc->getContentsModel();
	if (model == NULL) {
		doc->unlock();
		return false;
	}
	g_object_ref(model);
	doc->unlock();

	if (parent == NULL) {

		// check if there is already the current page selected
		GtkTreeSelection * selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeViewBookmarks));

		if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
			XojLinkDest * link = NULL;

			gtk_tree_model_get(model, &iter, DOCUMENT_LINKS_COLUMN_LINK, &link, -1);

			if (link && link->dest) {
				LinkDestination *dest = link->dest;

				if (dest->getPdfPage() == pdfPage) {

					g_object_unref(model);
					g_object_unref(link);

					// already bookmak from this page selected
					return true;
				}
			}

			g_object_unref(link);
		}
	}

	gboolean valid = gtk_tree_model_iter_children(model, &iter, parent);

	while (valid) {
		XojLinkDest * link = NULL;

		gtk_tree_model_get(model, &iter, DOCUMENT_LINKS_COLUMN_LINK, &link, -1);

		if (link->dest->getPdfPage() == pdfPage) {
			GtkTreeSelection * selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeViewBookmarks));
			gtk_tree_selection_select_iter(selection, &iter);

			g_object_unref(link);
			g_object_unref(model);
			return true;
		} else {
			g_object_unref(link);

			if (selectPageNr(page, pdfPage, &iter)) {
				g_object_unref(model);
				return true;
			} else {
				valid = gtk_tree_model_iter_next(model, &iter);
			}
		}
	}

	g_object_unref(model);
	return false;
}

void SidebarIndexPage::documentChanged(DocumentChangeType type) {
	XOJ_CHECK_TYPE(SidebarIndexPage);

	if (type == DOCUMENT_CHANGE_CLEARED) {
		gtk_tree_view_set_model(GTK_TREE_VIEW(this->treeViewBookmarks), NULL);
	} else if (type == DOCUMENT_CHANGE_PDF_BOOKMARKS || type == DOCUMENT_CHANGE_COMPLETE) {

		Document * doc = this->control->getDocument();

		doc->lock();
		GtkTreeModel * model = doc->getContentsModel();
		gtk_tree_view_set_model(GTK_TREE_VIEW(this->treeViewBookmarks), model);
		int count = expandOpenLinks(model, NULL);
		doc->unlock();

		hasContents = (count != 0);
	}
}




