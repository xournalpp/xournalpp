#include "SidebarPreviewLayers.h"
#include "SidebarPreviewLayerEntry.h"

#include "control/Control.h"
#include "control/PdfCache.h"
#include "control/layer/LayerController.h"

#include <i18n.h>

SidebarPreviewLayers::SidebarPreviewLayers(Control* control, GladeGui* gui, SidebarToolbar* toolbar)
 : SidebarPreviewBase(control, gui, toolbar),
   lc(control->getLayerController())
{
	XOJ_INIT_TYPE(SidebarPreviewLayers);

	LayerCtrlListener::registerListener(lc);

	this->toolbar->setButtonEnabled(false, false, false, false, PageRef());
}

SidebarPreviewLayers::~SidebarPreviewLayers()
{
	XOJ_CHECK_TYPE(SidebarPreviewLayers);

	// clear old previews
	for (SidebarPreviewBaseEntry* p : this->previews)
	{
		delete p;
	}
	this->previews.clear();

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

	PageRef page = lc->getCurrentPage();
	if (!page.isValid())
	{
		return;
	}

	int layerCount = page->getLayerCount();

	size_t index = 0;
	for (int i = layerCount - 1; i >= 0; i--)
	{
		SidebarPreviewBaseEntry* p = new SidebarPreviewLayerEntry(this, page, i, index++);
		this->previews.push_back(p);
		gtk_layout_put(GTK_LAYOUT(this->iconViewPreview), p->getWidget(), 0, 0);
	}

	// background
	SidebarPreviewBaseEntry* p = new SidebarPreviewLayerEntry(this, page, -1, index);
	this->previews.push_back(p);
	gtk_layout_put(GTK_LAYOUT(this->iconViewPreview), p->getWidget(), 0, 0);

	layout();
	updateSelectedLayer();
}

void SidebarPreviewLayers::rebuildLayerMenu()
{
	XOJ_CHECK_TYPE(SidebarPreviewLayers);

	updatePreviews();
}

void SidebarPreviewLayers::layerVisibilityChanged()
{
	XOJ_CHECK_TYPE(SidebarPreviewLayers);
	updateSelectedLayer();
}

void SidebarPreviewLayers::updateSelectedLayer()
{
	size_t layerIndex = lc->getCurrentLayerId();

	if (this->selectedEntry == layerIndex)
	{
		return;
	}
	this->selectedEntry = layerIndex;


	if (this->selectedEntry != size_t_npos && this->selectedEntry < this->previews.size())
	{
		this->previews[this->selectedEntry]->setSelected(false);
	}

//	if (this->selectedEntry != size_t_npos && this->selectedEntry < this->previews.size())
//	{
//		SidebarPreviewBaseEntry* p = this->previews[this->selectedEntry];
//		p->setSelected(true);
//		scrollToPreview(this);
//
//		// TODO This needs also be implemented seperate for layer
//		this->toolbar->setButtonEnabled(layerIndex != 0 && this->previews.size() != 0,
//				layerIndex != this->previews.size() - 1 && this->previews.size() != 0,
//										true, this->previews.size() > 1, PageRef());
//	}
}

void SidebarPreviewLayers::layerSelected(size_t layerIndex)
{
	XOJ_CHECK_TYPE(SidebarPreviewLayers);

	lc->switchToLay(layerIndex);
	updateSelectedLayer();
}

