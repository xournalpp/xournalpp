/*
 * Xournal++
 *
 * A Sidebar preview widget
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "../base/SidebarPreviewBaseEntry.h"
#include "model/PageRef.h"

class SidebarPreviewBase;

class SidebarPreviewLayerEntry : public SidebarPreviewBaseEntry
{
public:
	SidebarPreviewLayerEntry(SidebarPreviewBase* sidebar, PageRef page, int layer, size_t index);
	virtual ~SidebarPreviewLayerEntry();

public:
	virtual int getHeight();

	/**
	 * @return What should be rendered
	 * @override
	 */
	virtual PreviewRenderType getRenderType();

	/**
	 * @return The layer to be rendered
	 */
	int getLayer();

	virtual GtkWidget* getWidget();

	/**
	 * Set the value of the visible checkbox
	 */
	void setVisibleCheckbox(bool enabled);

protected:
	virtual void mouseButtonPressCallback();
	void checkboxToggled();

private:
	/**
	 * Layer preview index
	 */
	size_t index;

	/**
	 * Layer to render
	 */
	int layer;

	/**
	 * Toolbar with controls
	 */
	int toolbarHeight = 0;

	/**
	 * Container box for the preview and the button
	 */
	GtkWidget* box;

	/**
	 * Visible checkbox
	 */
	GtkWidget* cbVisible = nullptr;

	/**
	 * Ignore events
	 */
	bool inUpdate = false;

	friend class PreviewJob;
};
