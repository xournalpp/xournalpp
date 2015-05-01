/*
 * Xournal++
 *
 * A preview entry in a sidebar
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/PageRef.h"

#include <Util.h>
#include <XournalType.h>

#include <gtk/gtk.h>

class SidebarPreviewBase;

typedef enum {
	/**
	 * Render the whole page
	 */
	RENDER_TYPE_PAGE_PREVIEW = 1,

	/**
	 * Render only a layer
	 */
	RENDER_TYPE_PAGE_LAYER
} PreviewRenderType;


class SidebarPreviewBaseEntry
{
public:
	SidebarPreviewBaseEntry(SidebarPreviewBase* sidebar, PageRef page);
	virtual ~SidebarPreviewBaseEntry();

	GtkWidget* getWidget();
	int getWidth();
	int getHeight();

	void setSelected(bool selected);

	void repaint();
	void updateSize();

	/**
	 * @return What should be renderered
	 */
	virtual PreviewRenderType getRenderType() = 0;

private:
	static gboolean exposeEventCallback(GtkWidget* widget, GdkEventExpose* event, SidebarPreviewBaseEntry* preview);
	static gboolean mouseButtonPressCallback(GtkWidget* widget, GdkEventButton* event, SidebarPreviewBaseEntry* preview);

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
	SidebarPreviewBase* sidebar;

	/**
	 * The page which is representated
	 */
	PageRef page;

	/**
	 * Mutex
	 */
	GMutex drawingMutex;

	/**
	 * The Widget wich is used for drawing
	 */
	GtkWidget* widget;

	/**
	 * Buffer because of performance reasons
	 */
	cairo_surface_t* crBuffer;

	friend class PreviewJob;
};
