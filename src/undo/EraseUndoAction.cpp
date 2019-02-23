#include "EraseUndoAction.h"

#include "PageLayerPosEntry.h"

#include "gui/Redrawable.h"
#include "model/eraser/EraseableStroke.h"
#include "model/Layer.h"
#include "model/Stroke.h"

#include <i18n.h>

EraseUndoAction::EraseUndoAction(PageRef page)
 : UndoAction("EraseUndoAction"),
   page(page)
{
	XOJ_INIT_TYPE(EraseUndoAction);
}

EraseUndoAction::~EraseUndoAction()
{
	XOJ_CHECK_TYPE(EraseUndoAction);

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

	XOJ_RELEASE_TYPE(EraseUndoAction);
}

void EraseUndoAction::addOriginal(Layer* layer, Stroke* element, int pos)
{
	XOJ_CHECK_TYPE(EraseUndoAction);

	this->original = g_list_insert_sorted(this->original, new PageLayerPosEntry<Stroke> (layer, element, pos),
										  (GCompareFunc) PageLayerPosEntry<Stroke>::cmp);
}

void EraseUndoAction::addEdited(Layer* layer, Stroke* element, int pos)
{
	XOJ_CHECK_TYPE(EraseUndoAction);

	this->edited = g_list_insert_sorted(this->edited, new PageLayerPosEntry<Stroke> (layer, element, pos),
										(GCompareFunc) PageLayerPosEntry<Stroke>::cmp);
}

void EraseUndoAction::removeEdited(Stroke* element)
{
	XOJ_CHECK_TYPE(EraseUndoAction);

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
	XOJ_CHECK_TYPE(EraseUndoAction);

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
	XOJ_CHECK_TYPE(EraseUndoAction);

	return _("Erase stroke");
}

bool EraseUndoAction::undo(Control* control)
{
	XOJ_CHECK_TYPE(EraseUndoAction);

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
	XOJ_CHECK_TYPE(EraseUndoAction);

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
