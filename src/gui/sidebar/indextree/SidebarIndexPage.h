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

#include "gui/sidebar/AbstractSidebarPage.h"

#include <gtk/gtk.h>
#include <XournalType.h>

class Control;

class SidebarIndexPage : public AbstractSidebarPage
{
public:
	SidebarIndexPage(Control* control, SidebarToolbar* toolbar);
	virtual ~SidebarIndexPage();

public:
	virtual void enableSidebar();
	virtual void disableSidebar();

	/**
	 * @overwrite
	 */
	virtual string getName();

	/**
	 * @overwrite
	 */
	virtual string getIconName();

	/**
	 * @overwrite
	 */
	virtual bool hasData();

	/**
	 * @overwrite
	 */
	virtual GtkWidget* getWidget();

	/**
	 * @overwrite
	 */
	virtual void selectPageNr(size_t page, size_t pdfPage);

	/**
	 * Select page in the tree
	 */
	bool selectPageNr(size_t page, size_t pdfPage, GtkTreeIter* parent);

	/**
	 * @overwrite
	 */
	virtual void documentChanged(DocumentChangeType type);

private:
	/**
	 * Tree search function if you type chars within the tree. Source: Pidgin
	 */
	static gboolean treeSearchFunction(GtkTreeModel* model, gint column, const gchar* key,
									   GtkTreeIter* iter, SidebarIndexPage* sidebar);

	/**
	 * A bookmark was selected
	 */
	static bool treeBookmarkSelected(GtkWidget* treeview, SidebarIndexPage* sidebar);

	/**
	 * If you select a Bookmark wich is currently not in the Xournal document, only in the PDF (page deleted or so)
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
	 * If currently searching, scroll to the page is disable, else search is not really working
	 *
	 * After a timeout we scroll to the selected page
	 */
	int searchTimeout = 0;

	/**
	 * If there is something to display in the tree
	 */
	bool hasContents = false;

};
