#include "LayerController.h"

LayerController::LayerController()
{
	XOJ_INIT_TYPE(LayerController);
}

LayerController::~LayerController()
{
	XOJ_CHECK_TYPE(LayerController);

	XOJ_RELEASE_TYPE(LayerController);
}

bool LayerController::actionPerformed(ActionType type)
{
	XOJ_CHECK_TYPE(LayerController);

	switch(type)
	{
//	case ACTION_NEW_LAYER:
//		addNewLayer();
//		return true;
//
//	case ACTION_DELETE_LAYER:
//		deleteCurrentLayer();
//		return true;
//
//	case ACTION_FOOTER_LAYER:
//		switchToLay(this->win->getCurrentLayer());
//		return true;
//
//	case ACTION_GOTO_NEXT_LAYER:
//		{
//			int layer = this->win->getCurrentLayer();
//			PageRef p = getCurrentPage();
//			if (layer < (int)p->getLayerCount())
//			{
//				switchToLay(layer + 1);
//			}
//		}
//		return true;
//
//	case ACTION_GOTO_PREVIOUS_LAYER:
//		{
//			int layer = this->win->getCurrentLayer();
//			if (layer > 0)
//			{
//				switchToLay(layer - 1);
//			}
//		}
//		return true;
//
//	case ACTION_GOTO_TOP_LAYER:
//		{
//			PageRef p = getCurrentPage();
//			switchToLay(p->getLayerCount());
//		}
//		return true;
	default:
		return false;
	}
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
