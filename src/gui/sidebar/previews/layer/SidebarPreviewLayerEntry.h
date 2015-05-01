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
	SidebarPreviewLayerEntry(SidebarPreviewBase* sidebar, PageRef page);
	virtual ~SidebarPreviewLayerEntry();

private:
	XOJ_TYPE_ATTRIB;

	friend class PreviewJob;
};
