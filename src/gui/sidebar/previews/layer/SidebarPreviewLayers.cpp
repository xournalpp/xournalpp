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

void SidebarPreviewLayers::enableSidebar()
{
	XOJ_CHECK_TYPE(SidebarPreviewLayers);
	SidebarPreviewBase::enableSidebar();

	rebuildLayerMenu();
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

	if (!enabled)
	{
		return;
	}

	// clear old previews
	for (SidebarPreviewBaseEntry* p : this->previews)
	{
		delete p;
	}
	this->previews.clear();
	this->selectedEntry = size_t_npos;

	PageRef page = lc->getCurrentPage();
	if (!page.isValid())
	{
		return;
	}

	int layerCount = page->getLayerCount();

	size_t index = 0;
	for (int i = layerCount; i >= 0; i--)
	{
		SidebarPreviewBaseEntry* p = new SidebarPreviewLayerEntry(this, page, i - 1, index++);
		this->previews.push_back(p);
		gtk_layout_put(GTK_LAYOUT(this->iconViewPreview), p->getWidget(), 0, 0);
	}

	layout();
	updateSelectedLayer();
	layerVisibilityChanged();
}

void SidebarPreviewLayers::rebuildLayerMenu()
{
	XOJ_CHECK_TYPE(SidebarPreviewLayers);

	if (!enabled)
	{
		return;
	}

	updatePreviews();
}

void SidebarPreviewLayers::layerVisibilityChanged()
{
	XOJ_CHECK_TYPE(SidebarPreviewLayers);

	PageRef p = lc->getCurrentPage();
	if (!p.isValid())
	{
		return;
	}

	for (int i = 0; i < this->previews.size(); i++)
	{
		SidebarPreviewLayerEntry* sp = (SidebarPreviewLayerEntry*)this->previews[this->previews.size() - i - 1];
		sp->setVisibleCheckbox(p->isLayerVisible(i));
	}
}

void SidebarPreviewLayers::updateSelectedLayer()
{
	// Layers are in reverse order (top index: 0, but bottom preview is 0)
	size_t layerIndex = this->previews.size() - lc->getCurrentLayerId() - 1;

	if (this->selectedEntry == layerIndex)
	{
		return;
	}

	if (this->selectedEntry != size_t_npos && this->selectedEntry < this->previews.size())
	{
		this->previews[this->selectedEntry]->setSelected(false);
	}

	this->selectedEntry = layerIndex;
	if (this->selectedEntry != size_t_npos && this->selectedEntry < this->previews.size())
	{
		SidebarPreviewBaseEntry* p = this->previews[this->selectedEntry];
		p->setSelected(true);
		scrollToPreview(this);
	}
}

void SidebarPreviewLayers::layerSelected(size_t layerIndex)
{
	XOJ_CHECK_TYPE(SidebarPreviewLayers);

	// Layers are in reverse order (top index: 0, but bottom preview is 0)
	lc->switchToLay(this->previews.size() - layerIndex - 1);
	updateSelectedLayer();
}

/**
 * A layer was hidden / showed
 */
void SidebarPreviewLayers::layerVisibilityChanged(int layerIndex, bool enabled)
{
	lc->setLayerVisible(layerIndex, enabled);
}


