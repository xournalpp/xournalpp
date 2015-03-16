#include "InsertUndoAction.h"
#include "../model/PageRef.h"
#include "../model/Layer.h"
#include "../model/Element.h"
#include "../gui/Redrawable.h"

InsertUndoAction::InsertUndoAction(PageRef page, Layer* layer,
                                   Element* element) : UndoAction("InsertUndoAction")
{
	XOJ_INIT_TYPE(InsertUndoAction);

	this->page = page;
	this->layer = layer;
	this->element = element;
}

InsertUndoAction::~InsertUndoAction()
{
	XOJ_CHECK_TYPE(InsertUndoAction);

	if (this->undone)
	{
		// Insert was undone, so this is not needed anymore
		delete this->element;
	}
	this->element = NULL;

	XOJ_RELEASE_TYPE(InsertUndoAction);
}

String InsertUndoAction::getText()
{
	XOJ_CHECK_TYPE(InsertUndoAction);

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
	XOJ_CHECK_TYPE(InsertUndoAction);

	this->layer->removeElement(this->element, false);

	this->page->fireElementChanged(this->element);

	this->undone = true;

	return true;
}

bool InsertUndoAction::redo(Control* control)
{
	XOJ_CHECK_TYPE(InsertUndoAction);

	this->layer->addElement(this->element);

	this->page->fireElementChanged(this->element);

	this->undone = false;

	return true;
}


InsertsUndoAction::InsertsUndoAction(PageRef page,
                                     Layer* layer,
                                     GList* elements) : UndoAction("InsertsUndoAction")
{
	XOJ_INIT_TYPE(InsertsUndoAction);

	this->page = page;
	this->layer = layer;
	this->elements = elements;
}

InsertsUndoAction::~InsertsUndoAction()
{
	XOJ_CHECK_TYPE(InsertsUndoAction);

	if (this->undone)
	{
		// Insert was undone, so this is not needed anymore
		for(GList* elem = this->elements;
		    elem != NULL; elem = elem->next)
		{
			Element* e = (Element*) elem->data;
			delete e;
			elem->data = NULL;
		}
	}
	g_list_free(this->elements);
	this->elements = NULL;

	XOJ_RELEASE_TYPE(InsertsUndoAction);
}

String InsertsUndoAction::getText()
{
	XOJ_CHECK_TYPE(InsertsUndoAction);

	return _("Insert elements");
}

bool InsertsUndoAction::undo(Control* control)
{
	XOJ_CHECK_TYPE(InsertsUndoAction);

	for(GList* l = this->elements;
	    l != NULL; l = l->next)
	{
		Element* elem = (Element*) l->data;

		this->layer->removeElement(elem, false);
		this->page->fireElementChanged(elem);
	}

	this->undone = true;

	return true;
}

bool InsertsUndoAction::redo(Control* control)
{
	XOJ_CHECK_TYPE(InsertsUndoAction);

	for(GList* l = this->elements;
	    l != NULL; l = l->next)
	{
		Element* elem = (Element*) l->data;

		this->layer->addElement(elem);
		this->page->fireElementChanged(elem);
	}

	this->undone = false;

	return true;
}
