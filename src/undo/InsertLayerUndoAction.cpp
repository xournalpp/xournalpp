#include "InsertLayerUndoAction.h"

#include "control/Control.h"
#include "gui/XournalView.h"
#include "model/Document.h"
#include "model/Layer.h"
#include "model/PageRef.h"

InsertLayerUndoAction::InsertLayerUndoAction(PageRef page, Layer* layer) : UndoAction("InsertLayerUndoAction")
{
	XOJ_INIT_TYPE(InsertLayerUndoAction);

	this->page = page;
	this->layer = layer;
}

InsertLayerUndoAction::~InsertLayerUndoAction()
{
	XOJ_CHECK_TYPE(InsertLayerUndoAction);

	if (this->undone)
	{
		// The layer was undone, also deleted
		delete this->layer;
	}

	XOJ_RELEASE_TYPE(InsertLayerUndoAction);
}

string InsertLayerUndoAction::getText()
{
	XOJ_CHECK_TYPE(InsertLayerUndoAction);

	return _("Insert layer");
}

bool InsertLayerUndoAction::undo(Control* control)
{
	XOJ_CHECK_TYPE(InsertLayerUndoAction);

	Document* doc = control->getDocument();

	//perform the same thing we did to InsertDeletePage
	//to prevent a double lock (we're already locked here)
	//doc->lock();

	this->page->removeLayer(this->layer);
	int id = doc->indexOf(this->page);
	control->getWindow()->getXournal()->layerChanged(id);

	//the combobox is also threadsafe
	doc->unlock();
	control->getWindow()->updateLayerCombobox();

	this->undone = true;

	//doc->unlock();
	doc->lock();
	return true;
}

bool InsertLayerUndoAction::redo(Control* control)
{
	XOJ_CHECK_TYPE(InsertLayerUndoAction);

	Document* doc = control->getDocument();

	//doc->lock();

	this->page->addLayer(this->layer);
	int id = doc->indexOf(this->page);
	control->getWindow()->getXournal()->layerChanged(id);

	//the combobox is also threadsafe
	doc->unlock();
	control->getWindow()->updateLayerCombobox();

	this->undone = false;

	//doc->unlock();
	doc->lock();

	return true;
}
