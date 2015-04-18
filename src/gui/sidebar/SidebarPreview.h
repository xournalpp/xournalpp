/*
 * Xournal++
 *
 * A Sidebar preview widget
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#pragma once

#include <gtk/gtk.h>
#include <Util.h>
#include "../../model/PageRef.h"
#include <XournalType.h>

#include "../../control/PdfCache.h"

class Sidebar;

class SidebarPreview
{
public:
	SidebarPreview(Sidebar* sidebar, PageRef page);
	virtual ~SidebarPreview();

	GtkWidget* getWidget();
	int getWidth();
	int getHeight();

	void setSelected(bool selected);

	void repaint();
	void updateSize();

private:
	static gboolean exposeEventCallback(GtkWidget* widget, GdkEventExpose* event,
										SidebarPreview* preview);
	static gboolean mouseButtonPressCallback(GtkWidget* widget,
											GdkEventButton* event, SidebarPreview* preview);

	void paint();

private:
	XOJ_TYPE_ATTRIB;

	bool selected;

	bool firstPainted;

	Sidebar* sidebar;
	PageRef page;

	GMutex* drawingMutex;

	GtkWidget* widget;

	cairo_surface_t* crBuffer;

	friend class PreviewJob;
};
