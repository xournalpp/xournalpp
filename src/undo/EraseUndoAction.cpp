#include "EraseUndoAction.h"

#include "PageLayerPosEntry.h"

#include "gui/Redrawable.h"
#include "model/eraser/EraseableStroke.h"
#include "model/Layer.h"
#include "model/Stroke.h"

#include <i18n.h>

EraseUndoAction::EraseUndoAction(PageRef page)
 : UndoAction("EraseUndoAction")
{
	this->page = page;
}

EraseUndoAction::~EraseUndoAction()
{
	for (GList* l = this->original; l != NULL; l = l->next)
	{
		PageLayerPosEntry<Stroke>* e = (PageLayerPosEntry<Stroke>*) l->data;
		if (!undone)
		{
			delete e->element;
		}
		delete e;
	}
	g_list_free(this->original);
	this->original = NULL;

	for (GList* l = this->edited; l != NULL; l = l->next)
	{
		PageLayerPosEntry<Stroke>* e = (PageLayerPosEntry<Stroke>*) l->data;
		if (undone)
		{
			delete e->element;
		}
		delete e;
	}
	g_list_free(this->edited);
	this->edited = NULL;
}

void EraseUndoAction::addOriginal(Layer* layer, Stroke* element, int pos)
{
	this->original = g_list_insert_sorted(this->original, new PageLayerPosEntry<Stroke> (layer, element, pos),
										  (GCompareFunc) PageLayerPosEntry<Stroke>::cmp);
}

void EraseUndoAction::addEdited(Layer* layer, Stroke* element, int pos)
{
	this->edited = g_list_insert_sorted(this->edited, new PageLayerPosEntry<Stroke> (layer, element, pos),
										(GCompareFunc) PageLayerPosEntry<Stroke>::cmp);
}

void EraseUndoAction::removeEdited(Stroke* element)
{
	for (GList* l = this->edited; l != NULL; l = l->next)
	{
		PageLayerPosEntry<Stroke>* p = (PageLayerPosEntry<Stroke>*) l->data;
		if (p->element == element)
		{
			this->edited = g_list_delete_link(this->edited, l);
			delete p;
			p = NULL;
			return;
		}
	}
}

void EraseUndoAction::finalize()
{
	for (GList* l = this->original; l != NULL;)
	{
		PageLayerPosEntry<Stroke>* p = (PageLayerPosEntry<Stroke>*) l->data;
		GList* del = l;
		l = l->next;

		if (p->element->getPointCount() == 0)
		{
			this->edited = g_list_delete_link(this->edited, del);
			delete p;
			p = NULL;
		}
		else
		{

			// Remove the original and add the copy
			int pos = p->layer->removeElement(p->element, false);

			EraseableStroke* e = p->element->getEraseable();
			GList* stroke = e->getStroke(p->element);
			for (GList* ls = stroke; ls != NULL; ls = ls->next)
			{
				Stroke* copy = (Stroke*) ls->data;
				p->layer->insertElement(copy, pos);
				this->addEdited(p->layer, copy, pos);
				pos++;
			}

			delete e;
			e = NULL;
			p->element->setEraseable(NULL);
		}
	}

	this->page->firePageChanged();
}

string EraseUndoAction::getText()
{
	return _("Erase stroke");
}

bool EraseUndoAction::undo(Control* control)
{
	for (GList* l = this->edited; l != NULL; l = l->next)
	{
		PageLayerPosEntry<Stroke>* e = (PageLayerPosEntry<Stroke>*) l->data;

		e->layer->removeElement(e->element, false);
		this->page->fireElementChanged(e->element);
	}

	for (GList* l = this->original; l != NULL; l = l->next)
	{
		PageLayerPosEntry<Stroke>* e = (PageLayerPosEntry<Stroke>*) l->data;

		e->layer->insertElement(e->element, e->pos);
		this->page->fireElementChanged(e->element);
	}

	this->undone = true;
	return true;
}

bool EraseUndoAction::redo(Control* control)
{
	for (GList* l = this->original; l != NULL; l = l->next)
	{
		PageLayerPosEntry<Stroke>* e = (PageLayerPosEntry<Stroke>*) l->data;

		e->layer->removeElement(e->element, false);
		this->page->fireElementChanged(e->element);
	}

	for (GList* l = this->edited; l != NULL; l = l->next)
	{
		PageLayerPosEntry<Stroke>* e = (PageLayerPosEntry<Stroke>*) l->data;

		e->layer->insertElement(e->element, e->pos);
		this->page->fireElementChanged(e->element);
	}

	this->undone = false;
	return true;
}
