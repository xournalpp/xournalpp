/*
 * Xournal++
 *
 * A Sidebar preview widget
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __SIDEBARPREVIEWPAGE_H__
#define __SIDEBARPREVIEWPAGE_H__

#include <gtk/gtk.h>
#include <Util.h>
#include "../../../model/PageRef.h"
#include <XournalType.h>

class SidebarPreviews;

class SidebarPreviewPage {
public:
	SidebarPreviewPage(SidebarPreviews * sidebar, PageRef page);
	virtual ~SidebarPreviewPage();

	GtkWidget * getWidget();
	int getWidth();
	int getHeight();

	void setSelected(bool selected);

	void repaint();
	void updateSize();

private:
	static gboolean exposeEventCallback(GtkWidget * widget, GdkEventExpose * event, SidebarPreviewPage * preview);
	static gboolean mouseButtonPressCallback(GtkWidget * widget, GdkEventButton * event, SidebarPreviewPage * preview);

	void paint();

private:
	XOJ_TYPE_ATTRIB;

	/**
	 * If this page is currently selected
	 */
	bool selected;

	/**
	 * If this preview is painted once
	 */
	bool firstPainted;

	/**
	 * The sidebar which displays the previews
	 */
	SidebarPreviews * sidebar;

	/**
	 * The page which is representated
	 */
	PageRef page;

	/**
	 * Mutex
	 */
	GMutex * drawingMutex;

	/**
	 * The Widget wich is used for drawing
	 */
	GtkWidget * widget;

	/**
	 * Buffer because of performance reasons
	 */
	cairo_surface_t * crBuffer;

	friend class PreviewJob;
};

#endif /* __SIDEBARPREVIEWPAGE_H__ */

