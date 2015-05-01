#include "SidebarPreviewLayerEntry.h"

SidebarPreviewLayerEntry::SidebarPreviewLayerEntry(SidebarPreviewBase* sidebar, PageRef page) :
		SidebarPreviewBaseEntry(sidebar, page)
{
	XOJ_INIT_TYPE(SidebarPreviewLayerEntry);
}

SidebarPreviewLayerEntry::~SidebarPreviewLayerEntry()
{
	XOJ_CHECK_TYPE(SidebarPreviewLayerEntry);
	XOJ_RELEASE_TYPE(SidebarPreviewLayerEntry);
}
