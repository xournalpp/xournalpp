#include "SidebarPreviewLayers.h"

#include "control/Control.h"
#include "control/PdfCache.h"
#include "SidebarPreviewLayerEntry.h"

SidebarPreviewLayers::SidebarPreviewLayers(Control* control) : SidebarPreviewBase(control)
{
	XOJ_INIT_TYPE(SidebarPreviewLayers);
	displayedPage = -1;
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

void SidebarPreviewLayers::updatePreviews()
{
	XOJ_CHECK_TYPE(SidebarPreviewLayers);

	// clear old previews
	for (SidebarPreviewBaseEntry* p : this->previews)
	{
		delete p;
	}
	this->previews.clear();

	Document* doc = this->getControl()->getDocument();
	doc->lock();

	int len = doc->getPageCount();
	if (displayedPage < 0 || displayedPage >= len)
	{
		doc->unlock();
		return;
	}

	PageRef page = doc->getPage(displayedPage);

	// background
	SidebarPreviewBaseEntry* p = new SidebarPreviewLayerEntry(this, page, -1);
	this->previews.push_back(p);
	gtk_layout_put(GTK_LAYOUT(this->iconViewPreview), p->getWidget(), 0, 0);

	for (int i = page->getLayerCount() - 1; i >= 0; i--)
	{
		SidebarPreviewBaseEntry* p = new SidebarPreviewLayerEntry(this, page, i);
		this->previews.push_back(p);
		gtk_layout_put(GTK_LAYOUT(this->iconViewPreview), p->getWidget(), 0, 0);
	}

	layout();
	doc->unlock();
}

void SidebarPreviewLayers::pageSelected(int page)
{
	XOJ_CHECK_TYPE(SidebarPreviewLayers);

	displayedPage = page;
	updatePreviews();
}

void SidebarPreviewLayers::pageSizeChanged(int page)
{
	XOJ_CHECK_TYPE(SidebarPreviewLayers);

	if (displayedPage == page)
	{
		updatePreviews();
	}
}

void SidebarPreviewLayers::pageChanged(int page)
{
	XOJ_CHECK_TYPE(SidebarPreviewLayers);


	if (displayedPage == page)
	{
		// TODO DO IN JOB
		//updatePreviews();
	}
}
