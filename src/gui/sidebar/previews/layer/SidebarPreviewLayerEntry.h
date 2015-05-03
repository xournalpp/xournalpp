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
	SidebarPreviewLayerEntry(SidebarPreviewBase* sidebar, PageRef page, int layer);
	virtual ~SidebarPreviewLayerEntry();

public:
	/**
	 * @return What should be renderered
	 * @override
	 */
	virtual PreviewRenderType getRenderType();

	/**
	 * @return The layer to be rendererd
	 */
	int getLayer();

private:
	XOJ_TYPE_ATTRIB;

	/**
	 * Layer to render
	 */
	int layer;

	friend class PreviewJob;
};
