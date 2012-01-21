/*
 * Xournal++
 *
 * Index Sidebar Page
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __SIDEBARINDEXPAGE_H__
#define __SIDEBARINDEXPAGE_H__

#include "../AbstractSidebarPage.h"

#include <gtk/gtk.h>
#include <XournalType.h>

class Control;

class SidebarIndexPage : public AbstractSidebarPage {
public:
	SidebarIndexPage(Control * control);
	virtual ~SidebarIndexPage();

public:

	/**
	 * @overwrite
	 */
	virtual const char * getName();

	/**
	 * @overwrite
	 */
	virtual const char * getIconName();

	/**
	 * @overwrite
	 */
	virtual bool hasData();

	/**
	 * @overwrite
	 */
	virtual GtkWidget * getWidget();

	/**
	 * @overwrite
	 */
	virtual bool selectPageNr(int page, int pdfPage);

	/**
	 * Select page in the tree
	 */
	bool selectPageNr(int page, int pdfPage, GtkTreeIter * parent);

	/**
	 * @overwrite
	 */
	virtual void documentChanged(DocumentChangeType type);

private:
	/**
	 * Tree search function if you type chars within the tree. Source: Pidgin
	 */
	static gboolean treeSearchFunction(GtkTreeModel * model, gint column, const gchar * key, GtkTreeIter * iter, SidebarIndexPage * sidebar);

	/**
	 * A bookmark was selected
	 */
	static bool treeBookmarkSelected(GtkWidget * treeview, SidebarIndexPage * sidebar);

	/**
	 * If you select a Bookmark wich is currently not in the Xournal document, only in the PDF (page deleted or so)
	 */
	void askInsertPdfPage(int pdfPage);

	/**
	 * The function which is called after a search timeout
	 */
	static bool searchTimeoutFunc(SidebarIndexPage * sidebar);

private:
	XOJ_TYPE_ATTRIB;

private:

	/**
	 * Expand links
	 */
	int expandOpenLinks(GtkTreeModel * model, GtkTreeIter * parent);

	/**
	 * The Tree with the Bookmarks
	 */
	GtkWidget * treeViewBookmarks;

	/**
	 * The scrollbars for the Tree
	 */
	GtkWidget * scrollBookmarks;

	/**
	 * If currently searching, scroll to the page is disable, else search is not really working
	 *
	 * After a timeout we scroll to the selected page
	 */
	int searchTimeout;

	/**
	 * If there is something to display in the tree
	 */
	bool hasContents;

};

#endif /* __SIDEBARINDEXPAGE_H__ */
