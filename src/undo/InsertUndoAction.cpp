#include "InsertUndoAction.h"

#include "gui/Redrawable.h"
#include "model/Element.h"
#include "model/Layer.h"
#include "model/PageRef.h"

#include <i18n.h>

InsertUndoAction::InsertUndoAction(PageRef page, Layer* layer, Element* element)
 : UndoAction("InsertUndoAction")
{
	this->page = page;
	this->layer = layer;
	this->element = element;
}

InsertUndoAction::~InsertUndoAction()
{
	if (this->undone)
	{
		// Insert was undone, so this is not needed anymore
		delete this->element;
	}
	this->element = nullptr;
}

string InsertUndoAction::getText()
{
	if (element->getType() == ELEMENT_STROKE)
	{
		return _("Draw stroke");
	}
	else if (element->getType() == ELEMENT_TEXT)
	{
		return _("Write text");
	}
	else if (element->getType() == ELEMENT_IMAGE)
	{
		return _("Insert image");
	}
	else if (element->getType() == ELEMENT_TEXIMAGE)
	{
		return _("Insert latex");
	}
	else
	{
		return "";
	}
}

bool InsertUndoAction::undo(Control* control)
{
	this->layer->removeElement(this->element, false);

	this->page->fireElementChanged(this->element);

	this->undone = true;

	return true;
}

bool InsertUndoAction::redo(Control* control)
{
	this->layer->addElement(this->element);

	this->page->fireElementChanged(this->element);

	this->undone = false;

	return true;
}

InsertsUndoAction::InsertsUndoAction(PageRef page, Layer* layer, vector<Element*> elements) : UndoAction("InsertsUndoAction")
{
	this->page = page;
	this->layer = layer;
	this->elements = elements;
}

InsertsUndoAction::~InsertsUndoAction()
{
	if (this->undone)
	{
		// Insert was undone, so this is not needed anymore
		for (Element* e : this->elements)
		{
			delete e;
			e = nullptr;
		}
	}
}

string InsertsUndoAction::getText()
{
	return _("Insert elements");
}

bool InsertsUndoAction::undo(Control* control)
{
	for (Element* elem : this->elements)
	{
		this->layer->removeElement(elem, false);
		this->page->fireElementChanged(elem);
	}

	this->undone = true;

	return true;
}

bool InsertsUndoAction::redo(Control* control)
{
	for (Element* elem : this->elements)
	{
		this->layer->addElement(elem);
		this->page->fireElementChanged(elem);
	}

	this->undone = false;

	return true;
}
