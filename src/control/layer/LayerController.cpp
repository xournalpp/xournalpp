#include "LayerController.h"
#include "LayerCtrlListener.h"
#include "control/Control.h"

#include <Util.h>

LayerController::LayerController(Control* control)
 : control(control),
   selectedPage(size_t_npos)
{
	XOJ_INIT_TYPE(LayerController);
}

LayerController::~LayerController()
{
	XOJ_CHECK_TYPE(LayerController);

	// Do not delete the listeners!

	XOJ_RELEASE_TYPE(LayerController);
}

void LayerController::documentChanged(DocumentChangeType type)
{
	XOJ_CHECK_TYPE(LayerController);

	if (type == DOCUMENT_CHANGE_CLEARED || type == DOCUMENT_CHANGE_COMPLETE)
	{
		fireRebuildLayerMenu();
	}
}

void LayerController::pageSelected(size_t page)
{
	XOJ_CHECK_TYPE(LayerController);

	if (selectedPage == page)
	{
		return;
	}
	selectedPage = page;

	fireRebuildLayerMenu();
}

void LayerController::insertLayer(PageRef page, Layer* layer, int layerPos)
{
	XOJ_CHECK_TYPE(LayerController);

	page->insertLayer(layer, layerPos);
	fireRebuildLayerMenu();
}

void LayerController::removeLayer(PageRef page, Layer* layer)
{
	XOJ_CHECK_TYPE(LayerController);

	page->removeLayer(layer);
	fireRebuildLayerMenu();
}

void LayerController::addLayer(PageRef page, Layer* layer)
{
	XOJ_CHECK_TYPE(LayerController);

	page->addLayer(layer);
	fireRebuildLayerMenu();
}

void LayerController::addListener(LayerCtrlListener* listener)
{
	XOJ_CHECK_TYPE(LayerController);

	this->listener.push_back(listener);
}

void LayerController::removeListener(LayerCtrlListener* listener)
{
	XOJ_CHECK_TYPE(LayerController);

	this->listener.remove(listener);
}

void LayerController::fireRebuildLayerMenu()
{
	XOJ_CHECK_TYPE(LayerController);

	for (LayerCtrlListener* l : this->listener)
	{
		l->rebuildLayerMenu();
	}
}

void LayerController::fireLayerVisibilityChanged()
{
	XOJ_CHECK_TYPE(LayerController);

	for (LayerCtrlListener* l : this->listener)
	{
		l->layerVisibilityChanged();
	}
}

bool LayerController::actionPerformed(ActionType type)
{
	XOJ_CHECK_TYPE(LayerController);

	switch(type)
	{
	case ACTION_NEW_LAYER:
		addNewLayer();
		return true;

	case ACTION_DELETE_LAYER:
		deleteCurrentLayer();
		return true;

	case ACTION_FOOTER_LAYER:
		// This event is not fired anymore
		// Tis controller is called directly
		return true;

	case ACTION_GOTO_NEXT_LAYER:
//		{
//			int layer = this->win->getCurrentLayer();
//			PageRef p = getCurrentPage();
//			if (layer < (int)p->getLayerCount())
//			{
//				switchToLay(layer + 1);
//			}
//		}
		return true;

	case ACTION_GOTO_PREVIOUS_LAYER:
//		{
//			int layer = this->win->getCurrentLayer();
//			if (layer > 0)
//			{
//				switchToLay(layer - 1);
//			}
//		}
		return true;

	case ACTION_GOTO_TOP_LAYER:
//		{
//			PageRef p = getCurrentPage();
//			switchToLay(p->getLayerCount());
//		}
		return true;
	default:
		return false;
	}
}

/**
 * Show all layer on the current page
 */
void LayerController::showAllLayer()
{
	XOJ_CHECK_TYPE(LayerController);
	printf("LayerController::showAllLayer\n");
}

/**
 * Hide all layer on the current page
 */
void LayerController::hideAllLayer()
{
	XOJ_CHECK_TYPE(LayerController);
	printf("LayerController::hideAllLayer\n");
}

void LayerController::addNewLayer()
{
	XOJ_CHECK_TYPE(LayerController);

//	clearSelectionEndText();
//	PageRef p = getCurrentPage();
//	if (!p.isValid())
//	{
//		return;
//	}
//
//	Layer* l = new Layer();
//	p->insertLayer(l, p->getSelectedLayerId());
//	if (win)
//	{
//		win->updateLayerCombobox();
//	}
//
//	undoRedo->addUndoAction(new InsertLayerUndoAction(p, l));
}

void LayerController::deleteCurrentLayer()
{
	XOJ_CHECK_TYPE(LayerController);

//	clearSelectionEndText();
//	PageRef p = getCurrentPage();
//	int pId = getCurrentPageNo();
//	if (!p.isValid())
//	{
//		return;
//	}
//
//	int lId = p->getSelectedLayerId();
//	if (lId < 1)
//	{
//		return;
//	}
//	Layer* l = p->getSelectedLayer();
//
//	p->removeLayer(l);
//	if (win)
//	{
//		win->getXournal()->layerChanged(pId);
//		win->updateLayerCombobox();
//	}
//
//	undoRedo->addUndoAction(new RemoveLayerUndoAction(p, l, lId - 1));
//	this->resetShapeRecognizer();
}

void LayerController::switchToLay(int layer)
{
	XOJ_CHECK_TYPE(LayerController);

//	clearSelectionEndText();
//	PageRef p = getCurrentPage();
//	if (p.isValid())
//	{
//		p->setSelectedLayerId(layer);
//		this->win->getXournal()->layerChanged(getCurrentPageNo());
//		this->win->updateLayerCombobox();
//	}
}

/**
 * @return Layer count of the current page
 */
size_t LayerController::getLayerCount()
{
	XOJ_CHECK_TYPE(LayerController);

	PageRef page = control->getDocument()->getPage(selectedPage);
	if (!page.isValid())
	{
		return 0;
	}

	return page->getLayerCount();
}

/**
 * @return Current layer ID
 */
size_t LayerController::getCurrentLayerId()
{
	XOJ_CHECK_TYPE(LayerController);

	PageRef page = control->getDocument()->getPage(selectedPage);
	if (!page.isValid())
	{
		return 0;
	}

	return page->getSelectedLayerId();
}

/**
 * Make sure there is at least one layer on the page
 */
void LayerController::ensureLayerExists(PageRef page)
{
	XOJ_CHECK_TYPE(LayerController);

	if (page->getSelectedLayerId() > 0)
	{
		return;
	}

	// This creates a layer if none exists
	page->getSelectedLayer();
	page->setSelectedLayerId(1);

	fireRebuildLayerMenu();
}
