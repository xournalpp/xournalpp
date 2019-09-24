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

public:
	virtual GtkWidget* getWidget();
	virtual int getWidth();
	virtual int getHeight();

	virtual void setSelected(bool selected);

	virtual void repaint();
	virtual void updateSize();

	/**
	 * @return What should be renderered
	 */
	virtual PreviewRenderType getRenderType() = 0;

private:
	static gboolean drawCallback(GtkWidget* widget, cairo_t* cr, SidebarPreviewBaseEntry* preview);

protected:
	virtual void mouseButtonPressCallback() = 0;

	virtual int getWidgetWidth();
	virtual int getWidgetHeight();

	virtual void drawLoadingPage();
	virtual void paint(cairo_t* cr);

private:
	protected:
	/**
	 * If this page is currently selected
	 */
	bool selected = false;

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
	 * The Widget which is used for drawing
	 */
	GtkWidget* widget;

	/**
	 * Buffer because of performance reasons
	 */
	cairo_surface_t* crBuffer = nullptr;

	friend class PreviewJob;
};
