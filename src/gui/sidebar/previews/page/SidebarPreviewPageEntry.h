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

class SidebarPreviewPageEntry : public SidebarPreviewBaseEntry
{
public:
	SidebarPreviewPageEntry(SidebarPreviewBase* sidebar, PageRef page);
	virtual ~SidebarPreviewPageEntry();

public:
	/**
	 * @return What should be renderered
	 * @override
	 */
	virtual PreviewRenderType getRenderType();

private:
	XOJ_TYPE_ATTRIB;

	friend class PreviewJob;
};
