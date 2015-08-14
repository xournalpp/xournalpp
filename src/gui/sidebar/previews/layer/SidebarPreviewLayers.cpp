#include "SidebarPreviewLayers.h"

#include "SidebarPreviewLayerEntry.h"

#include "control/Control.h"
#include "control/PdfCache.h"

#include <i18n.h>

SidebarPreviewLayers::SidebarPreviewLayers(Control* control) : SidebarPreviewBase(control)
{
	XOJ_INIT_TYPE(SidebarPreviewLayers);
	displayedPage = 0;
}

SidebarPreviewLayers::~SidebarPreviewLayers()
{
	XOJ_CHECK_TYPE(SidebarPreviewLayers);
	XOJ_RELEASE_TYPE(SidebarPreviewLayers);
}

string SidebarPreviewLayers::getName()
{
	XOJ_CHECK_TYPE(SidebarPreviewLayers);

	return _("Layer Preview");
}

string SidebarPreviewLayers::getIconName()
{
	XOJ_CHECK_TYPE(SidebarPreviewLayers);

	return "layer.svg";
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
	// doc->lock();

	int len = doc->getPageCount();
	if (displayedPage < 0 || displayedPage >= len)
	{
		// doc->unlock();
		return;
	}

	PageRef page = doc->getPage(displayedPage);

	int layerCount = page->getLayerCount();

	for (int i = layerCount - 1; i >= 0; i--)
	{
		SidebarPreviewBaseEntry* p = new SidebarPreviewLayerEntry(this, page, i);
		this->previews.push_back(p);
		gtk_layout_put(GTK_LAYOUT(this->iconViewPreview), p->getWidget(), 0, 0);
	}

	// background
	SidebarPreviewBaseEntry* p = new SidebarPreviewLayerEntry(this, page, -1);
	this->previews.push_back(p);
	gtk_layout_put(GTK_LAYOUT(this->iconViewPreview), p->getWidget(), 0, 0);

	layout();
	// doc->unlock();
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

	printf("->SidebarPreviewLayers::pageChanged(%i)\n", page);

	if (displayedPage == page)
	{
		printf("->current page\n");

//		Document* doc = this->getControl()->getDocument();
//		PageRef page = doc->getPage(displayedPage);
//		int layerCount = page->getLayerCount();
//		if (layerCount + 1 == this->previews.size())
//		{
//			updatePreviews();
//		}

//		for (SidebarPreviewBaseEntry* p : this->previews)
//		{
//			p->repaint();
//		}
	}
}
