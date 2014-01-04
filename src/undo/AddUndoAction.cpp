#include "AddUndoAction.h"
#include "../model/Layer.h"
#include "../model/Element.h"
#include "../model/PageRef.h"
#include "../gui/Redrawable.h"
#include "PageLayerPosEntry.h"

AddUndoAction::AddUndoAction(PageRef page,
                             bool eraser) : UndoAction("AddUndoAction")
{
	XOJ_INIT_TYPE(AddUndoAction);

	this->page = page;
	this->eraser = eraser;
	this->elements = NULL;
}

AddUndoAction::~AddUndoAction()
{
	XOJ_CHECK_TYPE(AddUndoAction);

	for (GList* l = this->elements; l != NULL; l = l->next)
	{
		PageLayerPosEntry<Element>* e = (PageLayerPosEntry<Element>*) l->data;
		if (!undone)
		{
			delete e->element;
		}
		delete e;
	}
	g_list_free(this->elements);

	XOJ_RELEASE_TYPE(AddUndoAction)
	;
}

void AddUndoAction::addElement(Layer* layer, Element* e, int pos)
{
	XOJ_CHECK_TYPE(AddUndoAction);

	this->elements = g_list_insert_sorted(this->elements,
	                                      new PageLayerPosEntry<Element> (layer, e, pos),
	                                      (GCompareFunc) PageLayerPosEntry<Element>::cmp);
}

bool AddUndoAction::redo(Control* control)
{
	XOJ_CHECK_TYPE(AddUndoAction);

	if (this->elements == NULL)
	{
		g_warning("Could not undo AddUndoAction, there is nothing to undo");

		this->undone = true;
		return false;
	}

	for (GList* l = this->elements; l != NULL; l = l->next)
	{
		PageLayerPosEntry<Element>* e = (PageLayerPosEntry<Element>*) l->data;
		e->layer->insertElement(e->element, e->pos);
		this->page->fireElementChanged(e->element);
	}

	this->undone = true;
	return true;
}

bool AddUndoAction::undo(Control* control)
{
	XOJ_CHECK_TYPE(AddUndoAction);

	if (this->elements == NULL)
	{
		g_warning("Could not redo AddUndoAction, there is nothing to redo");

		this->undone = false;
		return false;
	}

	for (GList* l = this->elements; l != NULL; l = l->next)
	{
		PageLayerPosEntry<Element>* e = (PageLayerPosEntry<Element>*) l->data;
		e->layer->removeElement(e->element, false);
		this->page->fireElementChanged(e->element);
	}

	this->undone = false;

	return true;
}

String AddUndoAction::getText()
{
	XOJ_CHECK_TYPE(AddUndoAction);

	String text;

	if (eraser)
	{
		text = _("Erase stroke");
	}
	else
	{
		text = _("Delete");

		if (this->elements != NULL)
		{
			ElementType type = ((PageLayerPosEntry<Element>*)
			                    this->elements->data)->element->getType();

			for (GList* l = this->elements->next; l != NULL; l = l->next)
			{
				PageLayerPosEntry<Element>* e = (PageLayerPosEntry<Element>*) l->data;
				if (type != e->element->getType())
				{
					text += _(" elements");
					return text;
				}
			}

			if (type == ELEMENT_STROKE)
			{
				text += _(" stroke");
			}
			else if (type == ELEMENT_TEXT)
			{
				text += _(" text");
			}
			else if (type == ELEMENT_IMAGE)
			{
				text += _(" image");
			}
			else if (type == ELEMENT_TEXIMAGE)
			{
				text += _(" latex");
			}
		}
	}
	return text;
}
