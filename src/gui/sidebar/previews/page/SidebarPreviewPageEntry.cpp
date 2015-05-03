#include "SidebarPreviewPageEntry.h"

SidebarPreviewPageEntry::SidebarPreviewPageEntry(SidebarPreviewBase* sidebar, PageRef page) :
		SidebarPreviewBaseEntry(sidebar, page)
{
	XOJ_INIT_TYPE(SidebarPreviewPageEntry);
}

SidebarPreviewPageEntry::~SidebarPreviewPageEntry()
{
	XOJ_CHECK_TYPE(SidebarPreviewPageEntry);
	XOJ_RELEASE_TYPE(SidebarPreviewPageEntry);
}

PreviewRenderType SidebarPreviewPageEntry::getRenderType()
{
	XOJ_CHECK_TYPE(SidebarPreviewPageEntry);

	return RENDER_TYPE_PAGE_PREVIEW;
}
