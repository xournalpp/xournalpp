#include "LayerController.h"
#include "LayerCtrlListener.h"
#include "control/Control.h"
#include "gui/XournalView.h"
#include "undo/RemoveLayerUndoAction.h"
#include "undo/InsertLayerUndoAction.h"
#include "undo/MoveLayerUndoAction.h"

#include "Util.h"
#include "util/cpp14memory.h"

LayerController::LayerController(Control* control)
 : control(control)
 , selectedPage(npos)
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
		// This controller is called directly
		return true;

	case ACTION_GOTO_NEXT_LAYER:
		{
			PageRef p = getCurrentPage();
			int layer = p->getSelectedLayerId();
			if (layer < (int)p->getLayerCount())
			{
				switchToLay(layer + 1, true);
			}
		}
		return true;

	case ACTION_GOTO_PREVIOUS_LAYER:
		{
			PageRef p = getCurrentPage();
			int layer = p->getSelectedLayerId();
			if (layer > 0)
			{
				switchToLay(layer - 1, true);
			}
		}
		return true;

	case ACTION_GOTO_TOP_LAYER:
		{
			PageRef p = getCurrentPage();
			switchToLay(p->getLayerCount(), true);
		}
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
	hideOrHideAllLayer(true);
}

/**
 * Hide all layer on the current page
 */
void LayerController::hideAllLayer()
{
	XOJ_CHECK_TYPE(LayerController);
	hideOrHideAllLayer(false);
}

/**
 * Show / Hide all layer on the current page
 */
void LayerController::hideOrHideAllLayer(bool show)
{
	PageRef page = getCurrentPage();
	for (size_t i = 1; i <= page->getLayerCount(); i++)
	{
		page->setLayerVisible(i, show);
	}

	fireLayerVisibilityChanged();
	control->getWindow()->getXournal()->layerChanged(selectedPage);
}

void LayerController::addNewLayer()
{
	XOJ_CHECK_TYPE(LayerController);

	control->clearSelectionEndText();
	PageRef p = getCurrentPage();
	if (!p.isValid())
	{
		return;
	}

	Layer* l = new Layer();
	int layerPos = p->getSelectedLayerId();
	p->insertLayer(l, layerPos);

	control->getUndoRedoHandler()->addUndoAction(mem::make_unique<InsertLayerUndoAction>(this, p, l, layerPos));

	fireRebuildLayerMenu();
	// Repaint is not needed here - the new layer is empty
}

void LayerController::deleteCurrentLayer()
{
	XOJ_CHECK_TYPE(LayerController);

	control->clearSelectionEndText();

	PageRef p = getCurrentPage();
	int pId = selectedPage;
	if (!p.isValid())
	{
		return;
	}

	int lId = p->getSelectedLayerId();
	if (lId < 1)
	{
		return;
	}
	Layer* l = p->getSelectedLayer();

	p->removeLayer(l);

	MainWindow* win = control->getWindow();
	if (win)
	{
		win->getXournal()->layerChanged(pId);
	}

	control->getUndoRedoHandler()->addUndoAction(mem::make_unique<RemoveLayerUndoAction>(this, p, l, lId - 1));
	control->resetShapeRecognizer();

	fireRebuildLayerMenu();
}

void LayerController::moveCurrentLayer(bool up)
{
	XOJ_CHECK_TYPE(LayerController);

	control->clearSelectionEndText();

	PageRef p = getCurrentPage();
	int pId = selectedPage;
	if (!p.isValid())
	{
		return;
	}

	int lId = p->getSelectedLayerId();
	Layer* currentLayer = p->getSelectedLayer();
	if (lId < 1)
	{
		// Background cannot be moved
		return;
	}

	if (lId < 2 && !up)
	{
		// bottom layer cannot be moved down
		return;
	}

	if (lId == (int)p->getLayerCount() && up)
	{
		// top layer cannot be moved up
		return;
	}

	p->removeLayer(currentLayer);

	// Layer IDs are a bit strange, because background is 0
	// so the first layer is 1, technical the first layer is still
	// index 0 in the vector... confusing...
	int newIndex = up ? lId : lId - 2;
	p->insertLayer(currentLayer, newIndex);

	MainWindow* win = control->getWindow();
	if (win)
	{
		win->getXournal()->layerChanged(pId);
	}

	control->getUndoRedoHandler()->addUndoAction(
	        mem::make_unique<MoveLayerUndoAction>(this, p, currentLayer, lId - 1, newIndex));

	fireRebuildLayerMenu();
}

void LayerController::copyCurrentLayer()
{
	XOJ_CHECK_TYPE(LayerController);

	control->clearSelectionEndText();

	PageRef p = getCurrentPage();
	int pId = selectedPage;
	if (!p.isValid())
	{
		return;
	}

	int lId = p->getSelectedLayerId();
	if (lId < 1)
	{
		return;
	}
	Layer* l = p->getSelectedLayer();
	Layer* cloned = l->clone();

	p->insertLayer(cloned, lId);

	MainWindow* win = control->getWindow();
	if (win)
	{
		win->getXournal()->layerChanged(pId);
	}

	control->getUndoRedoHandler()->addUndoAction(mem::make_unique<InsertLayerUndoAction>(this, p, cloned, lId));
	control->resetShapeRecognizer();

	fireRebuildLayerMenu();
}

PageRef LayerController::getCurrentPage()
{
	XOJ_CHECK_TYPE(LayerController);

	return control->getDocument()->getPage(selectedPage);
}

size_t LayerController::getCurrentPageId()
{
	XOJ_CHECK_TYPE(LayerController);

	return selectedPage;
}

void LayerController::setLayerVisible(int layerId, bool visible)
{
	XOJ_CHECK_TYPE(LayerController);

	getCurrentPage()->setLayerVisible(layerId, visible);
	fireLayerVisibilityChanged();

	control->getWindow()->getXournal()->layerChanged(selectedPage);
}

/**
 * Switch to a layer
 *
 * @param hideShow	Auto hide / show other layers,
 * 					as it was before the advance layer menu
 */
void LayerController::switchToLay(int layer, bool hideShow)
{
	XOJ_CHECK_TYPE(LayerController);

	control->clearSelectionEndText();

	PageRef p = getCurrentPage();
	if (!p.isValid())
	{
		return;
	}

	p->setSelectedLayerId(layer);

	if (hideShow)
	{
		for (size_t i = 1; i <= p->getLayerCount(); i++)
		{
			p->setLayerVisible(i, (int)i <= layer);
		}
	}

	// Repaint page
	control->getWindow()->getXournal()->layerChanged(selectedPage);
	fireLayerVisibilityChanged();
}

/**
 * @return Layer count of the current page
 */
size_t LayerController::getLayerCount()
{
	XOJ_CHECK_TYPE(LayerController);

	PageRef page = getCurrentPage();
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

	PageRef page = getCurrentPage();
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
