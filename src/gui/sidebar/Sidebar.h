/*
 * Xournal++
 *
 * The Sidebar
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __SIDEBAR_H__
#define __SIDEBAR_H__

#include <gtk/gtk.h>
#include "../../util/MemoryCheck.h"
#include "../../model/DocumentChangeType.h"
#include "../../model/DocumentListener.h"

class Control;
class Document;
class SidebarPreview;
class GladeGui;
class PdfCache;
// TODO: AA: type check

class Sidebar: public DocumentListener, MemoryCheckObject {
public:
	Sidebar(GladeGui * gui, Control * control);
	virtual ~Sidebar();

public:
	// DocumentListener interface
	void documentChanged(DocumentChangeType type);
	void pageSizeChanged(int page);
	void pageChanged(int page);
	void pageInserted(int page);
	void pageDeleted(int page);
	void pageSelected(int page);

public:
	bool selectPageNr(int page, int pdfPage, GtkTreeIter * iter = NULL);
	void setBackgroundWhite();
	Document * getDocument();
	Control * getControl();
	double getZoom();
	void setTmpDisabled(bool disabled);

	PdfCache * getCache();

private:
	int expandOpenLinks(GtkTreeModel * model, GtkTreeIter * parent);
	static bool treeClickedCallback(GtkWidget * treeview, GdkEventButton * event, Sidebar * sidebar);
	static gboolean treeSearchFunction(GtkTreeModel * model, gint column, const gchar * key, GtkTreeIter * iter, Sidebar * sidebar);

	static void cbChangedCallback(GtkComboBox * widget, Sidebar * sidebar);
	static bool scrollToPreview(Sidebar * sidebar);

	void askInsertPdfPage(int pdfPage);

	void layout();
	void updatePreviews();

private:
	bool backgroundInitialized;
	Control * control;
	GtkComboBox * comboBox;

	PdfCache * cache;

	GtkWidget * treeViewBookmarks;
	GtkWidget * iconViewPreview;
	GtkWidget * buttonCloseSidebar;

	bool typeSelected;
	int selectedPage;

	double zoom;

	GtkWidget * scrollBookmarks;
	GtkWidget * scrollPreview;

	SidebarPreview ** previews;
	int previewCount;
};

#endif /* __SIDEBAR_H__ */
