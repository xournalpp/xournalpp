#include "AddUndoAction.h"

#include "PageLayerPosEntry.h"

#include "gui/Redrawable.h"
#include "model/Layer.h"
#include "model/Element.h"
#include "model/PageRef.h"

#include <i18n.h>

AddUndoAction::AddUndoAction(const PageRef& page, bool eraser)
 : UndoAction("AddUndoAction")
{
	this->page = page;
	this->eraser = eraser;
}

AddUndoAction::~AddUndoAction()
{
	for (GList* l = this->elements; l != nullptr; l = l->next)
	{
		auto e = (PageLayerPosEntry<Element>*) l->data;
		if (!undone)
		{
			//The element will be deleted when the layer is removed.
			//delete e->element;
		}
		delete e;
	}
	g_list_free(this->elements);
			;
}

void AddUndoAction::addElement(Layer* layer, Element* e, int pos)
{
	this->elements = g_list_insert_sorted(this->elements, new PageLayerPosEntry<Element> (layer, e, pos),
										  (GCompareFunc) PageLayerPosEntry<Element>::cmp);
}

bool AddUndoAction::redo(Control*)
{
	if (this->elements == nullptr)
	{
		g_warning("Could not undo AddUndoAction, there is nothing to undo");

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

bool AddUndoAction::undo(Control*)
{
	if (this->elements == nullptr)
	{
		g_warning("Could not redo AddUndoAction, there is nothing to redo");

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

string AddUndoAction::getText()
{
	string text;

	if (eraser)
	{
		text = _("Erase stroke");
	}
	else
	{
		text = _("Paste");

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
	}
	return text;
}
