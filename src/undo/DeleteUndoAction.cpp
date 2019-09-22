#include "DeleteUndoAction.h"

#include "gui/Redrawable.h"
#include "model/Element.h"
#include "model/Layer.h"
#include "model/PageRef.h"
#include "PageLayerPosEntry.h"

#include <i18n.h>

DeleteUndoAction::DeleteUndoAction(const PageRef& page, bool eraser)
 : UndoAction("DeleteUndoAction")
{
	this->page = page;
	this->eraser = eraser;
}

DeleteUndoAction::~DeleteUndoAction()
{
	for (GList* l = this->elements; l != nullptr; l = l->next)
	{
		auto e = (PageLayerPosEntry<Element>*) l->data;
		if (!undone)
		{
			delete e->element;
		}
		delete e;
	}
	g_list_free(this->elements);
}

void DeleteUndoAction::addElement(Layer* layer, Element* e, int pos)
{
	this->elements = g_list_insert_sorted(this->elements, new PageLayerPosEntry<Element> (layer, e, pos),
										  (GCompareFunc) PageLayerPosEntry<Element>::cmp);
}

bool DeleteUndoAction::undo(Control*)
{
	if (this->elements == nullptr)
	{
		g_warning("Could not undo DeleteUndoAction, there is nothing to undo");

		this->undone = true;
		return false;
	}

	for (GList* l = this->elements; l != nullptr; l = l->next)
	{
		auto e = (PageLayerPosEntry<Element>*) l->data;
		e->layer->insertElement(e->element, e->pos);
		this->page->fireElementChanged(e->element);
	}

	this->undone = true;
	return true;
}

bool DeleteUndoAction::redo(Control*)
{
	if (this->elements == nullptr)
	{
		g_warning("Could not redo DeleteUndoAction, there is nothing to redo");

		this->undone = false;
		return false;
	}

	for (GList* l = this->elements; l != nullptr; l = l->next)
	{
		auto e = (PageLayerPosEntry<Element>*) l->data;
		e->layer->removeElement(e->element, false);
		this->page->fireElementChanged(e->element);
	}

	this->undone = false;

	return true;
}

string DeleteUndoAction::getText()
{
	if (eraser)
	{
		return _("Erase stroke");
	}

	string text = _("Delete");

	if (this->elements != nullptr)
	{
		ElementType type = ((PageLayerPosEntry<Element>*) this->elements->data)->element->getType();

		for (GList* l = this->elements->next; l != nullptr; l = l->next)
		{
			auto e = (PageLayerPosEntry<Element>*) l->data;
			if (type != e->element->getType())
			{
				text += " ";
				text += _("elements");
				return text;
			}
		}

		text += " ";
		switch (type)
		{
			case ELEMENT_STROKE:
				text += _("stroke");
				break;
			case ELEMENT_IMAGE:
				text += _("image");
				break;
			case ELEMENT_TEXIMAGE:
				text += _("latex");
				break;
			case ELEMENT_TEXT:
				text += _("text");
				break;
		}
	}

	return text;
}
