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

#include "SidebarPreviewPages.h"

class SidebarPreviewBase;
class SidebarPreviewPages;

class SidebarPreviewPageEntry : public SidebarPreviewBaseEntry
{
public:
	SidebarPreviewPageEntry(SidebarPreviewPages* sidebar, PageRef page);
	virtual ~SidebarPreviewPageEntry();

public:
	/**
	 * @return What should be rendered
	 * @override
	 */
	virtual PreviewRenderType getRenderType();

protected:
	SidebarPreviewPages* sidebar;
	virtual void mouseButtonPressCallback();

private:
	friend class PreviewJob;
};
