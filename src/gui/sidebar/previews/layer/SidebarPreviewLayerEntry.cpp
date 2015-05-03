#include "SidebarPreviewLayerEntry.h"

SidebarPreviewLayerEntry::SidebarPreviewLayerEntry(SidebarPreviewBase* sidebar, PageRef page, int layer) :
		SidebarPreviewBaseEntry(sidebar, page)
{
	XOJ_INIT_TYPE(SidebarPreviewLayerEntry);
	this->layer = layer;
}

SidebarPreviewLayerEntry::~SidebarPreviewLayerEntry()
{
	XOJ_CHECK_TYPE(SidebarPreviewLayerEntry);
	XOJ_RELEASE_TYPE(SidebarPreviewLayerEntry);
}

PreviewRenderType SidebarPreviewLayerEntry::getRenderType()
{
	XOJ_CHECK_TYPE(SidebarPreviewLayerEntry);

	return RENDER_TYPE_PAGE_LAYER;
}

int SidebarPreviewLayerEntry::getLayer()
{
	XOJ_CHECK_TYPE(SidebarPreviewLayerEntry);

	return layer;
}
