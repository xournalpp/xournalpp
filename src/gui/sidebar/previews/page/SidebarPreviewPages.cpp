#include "SidebarPreviewPages.h"

#include "control/Control.h"
#include "control/PdfCache.h"

SidebarPreviewPages::SidebarPreviewPages(Control* control) : SidebarPreviewBase(control)
{
	XOJ_INIT_TYPE(SidebarPreviewPages);
}

SidebarPreviewPages::~SidebarPreviewPages()
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);
	XOJ_RELEASE_TYPE(SidebarPreviewPages);
}

const char* SidebarPreviewPages::getName()
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	return _("Page Preview");
}

const char* SidebarPreviewPages::getIconName()
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	return "sidebar-page-preview.svg";
}


