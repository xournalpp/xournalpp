/*
 * Xournal++
 *
 * The previes within the sidebar, with a Toolbar
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __SIDEBARPREVIEWS_H__
#define __SIDEBARPREVIEWS_H__

#include "../AbstractSidebarPage.h"

#include <gtk/gtk.h>
#include <XournalType.h>

class SidebarPreviewPage;
class PdfCache;
class SidebarLayout;

class SidebarPreviews : public AbstractSidebarPage {
public:
	SidebarPreviews(Control * control);
	virtual ~SidebarPreviews();

public:
	/**
	 * Layout the pages to the current size of the sidebar
	 */
	void layout();

	/**
	 * Update the preview images
	 */
	void updatePreviews();

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
	 * Setts the Background of the panel white, before the first draw
	 */
	void setBackgroundWhite();

	/**
	 * Gets the zoom factor for the previews
	 */
	double getZoom();

	/**
	 * Gets the PDF cache for preview rendering
	 */
	PdfCache * getCache();

public:
	// DocumentListener interface
	virtual void documentChanged(DocumentChangeType type);
	virtual void pageSizeChanged(int page);
	virtual void pageChanged(int page);
	virtual void pageInserted(int page);
	virtual void pageDeleted(int page);
	virtual void pageSelected(int page);

private:
	/**
	 * Timeout callback to scroll to a page
	 */
	static bool scrollToPreview(SidebarPreviews * sidebar);

	/**
	 * The size of the sidebar has chnaged
	 */
	static void sizeChanged(GtkWidget * widget, GtkAllocation * allocation, SidebarPreviews * sidebar);
private:
	XOJ_TYPE_ATTRIB;

	/**
	 * The widget within the scrollarea with the page icons
	 */
	GtkWidget * iconViewPreview;

	/**
	 * The scrollbar with the icons
	 */
	GtkWidget * scrollPreview;

	/**
	 * The previews
	 */
	SidebarPreviewPage ** previews;
	int previewCount;

	/**
	 * The currently selected page
	 */
	int selectedPage;

	/**
	 * The Zoom of the previews
	 */
	double zoom;

	/**
	 * Only once, see setBackgroundWhite()
	 */
	bool backgroundInitialized;

	/**
	 * For preview rendering
	 */
	PdfCache * cache;

	/**
	 * The layouting class for the prviews
	 */
	SidebarLayout * layoutmanager;


	friend class SidebarLayout;
};

#endif /* __SIDEBARPREVIEWS_H__ */
