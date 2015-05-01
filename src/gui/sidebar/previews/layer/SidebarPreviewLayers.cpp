#include "SidebarPreviewLayers.h"

#include "control/Control.h"
#include "control/PdfCache.h"

SidebarPreviewLayers::SidebarPreviewLayers(Control* control) : SidebarPreviewBase(control)
{
	XOJ_INIT_TYPE(SidebarPreviewLayers);
}

SidebarPreviewLayers::~SidebarPreviewLayers()
{
	XOJ_CHECK_TYPE(SidebarPreviewLayers);
	XOJ_RELEASE_TYPE(SidebarPreviewLayers);
}

const char* SidebarPreviewLayers::getName()
{
	XOJ_CHECK_TYPE(SidebarPreviewLayers);

	return _("Layer Preview");
}

const char* SidebarPreviewLayers::getIconName()
{
	XOJ_CHECK_TYPE(SidebarPreviewLayers);

	return "sidebar-layer-preview.svg";
}


