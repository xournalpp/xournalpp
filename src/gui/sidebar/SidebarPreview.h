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
#ifndef __SIDEBARPREVIEW_H__
#define __SIDEBARPREVIEW_H__

#include <gtk/gtk.h>
#include "../../model/Page.h"
#include "../../view/DocumentView.h"

class Sidebar;

class SidebarPreview {
public:
	SidebarPreview(Sidebar * sidebar, XojPage * page);
	virtual ~SidebarPreview();

	GtkWidget * getWidget();
	int getWidth();
	int getHeight();

	void setSelected(bool selected);

	void repaint();
	void updateSize();
private:
	static gboolean exposeEventCallback(GtkWidget *widget, GdkEventExpose *event, SidebarPreview * preview);
	static gboolean mouseButtonPressCallback(GtkWidget *widget, GdkEventButton *event, SidebarPreview * preview);

	void paint();
private:
	/**
	 * The View to draw the page
	 */
	DocumentView * view;

	bool selected;

	bool firstPainted;

	Sidebar * sidebar;
	XojPage * page;

	GtkWidget * widget;

	cairo_surface_t * crBuffer;
};

#endif /* __SIDEBARPREVIEW_H__ */
