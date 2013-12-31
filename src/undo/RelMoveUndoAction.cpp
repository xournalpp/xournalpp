#include "RelMoveUndoAction.h"

#include "../model/Element.h"
#include "../model/PageRef.h"
#include "../model/Layer.h"
#include "../gui/Redrawable.h"
#include "../control/tools/EditSelection.h"
#include "../control/tools/VerticalToolHandler.h"


RelMoveUndoAction::RelMoveUndoAction(Layer* sourceLayer, PageRef sourcePage,
                               Redrawable* sourceView, GList* selected, double mx, double my,
                               Layer* targetLayer,
                               PageRef targetPage, Redrawable* targetView) : UndoAction("RelMoveUndoAction")
{
	XOJ_INIT_TYPE(RelMoveUndoAction);

	this->page = sourcePage;
	this->sourceLayer = sourceLayer;
	this->sourceView = sourceView;
	this->text = _("Move");

	this->targetView = NULL;
	this->targetLayer = NULL;
	this->targetPage = NULL;

	this->dx = mx;
	this->dy = my;

	this->elements = g_list_copy(selected);

	if (this->page != targetPage)
	{
		this->targetPage = targetPage;
		this->targetLayer = targetLayer;
		this->targetView = targetView;
	}

	this->undone = false;
}

RelMoveUndoAction::~RelMoveUndoAction()
{
	XOJ_CHECK_TYPE(RelMoveUndoAction);

	g_list_free(this->elements);
	this->elements = NULL;

	XOJ_RELEASE_TYPE(RelMoveUndoAction);
}

void RelMoveUndoAction::move()
{
	if(this->undone)
	{
		for (GList* l = this->elements; l != NULL; l = l->next)
		{
			Element* e = (Element*) l->data;

			e->move(dx, dy);
		}
	}
	else
	{
		for (GList* l = this->elements; l != NULL; l = l->next)
		{
			Element* e = (Element*) l->data;

			e->move(-dx, -dy);
		}
	}
}

bool RelMoveUndoAction::undo(Control* control)
{
	XOJ_CHECK_TYPE(RelMoveUndoAction);

	if (this->sourceLayer != this->targetLayer && this->targetLayer != NULL)
	{
		switchLayer(this->elements, this->targetLayer, this->sourceLayer);
	}

	move();
	repaint();
	this->undone = true;

	return true;
}

bool RelMoveUndoAction::redo(Control* control)
{
	XOJ_CHECK_TYPE(RelMoveUndoAction);

	if (this->sourceLayer != this->targetLayer && this->targetLayer != NULL)
	{
		switchLayer(this->elements, this->sourceLayer, this->targetLayer);
	}

	move();
	repaint();
	this->undone = false;

	return true;
}

void RelMoveUndoAction::switchLayer(GList* entries, Layer* oldLayer,
                                    Layer* newLayer)
{
	XOJ_CHECK_TYPE(RelMoveUndoAction);

	for (GList* l = this->elements; l != NULL; l = l->next)
	{
		Element* e = (Element*) l->data;
		oldLayer->removeElement(e, false);
		newLayer->addElement(e);
	}
}

void RelMoveUndoAction::repaint(Redrawable *view,
                                bool target)
{
	Element *e = (Element*) this->elements->data;
	
	Range range(e->getX(), e->getY());

	for (GList* l = this->elements; l != NULL; l = l->next)
	{
		e = (Element*) l->data;

		double ex = e->getX(), ey = e->getY();

		if(this->undone && target)
		{
			ex += dx;
			ey += dy;
		}
		else if(!this->undone && !target)
		{
			ex -= dx;
			ey -= dy;
		}

		range.addPoint(ex, ey);
		range.addPoint(ex + e->getElementWidth(),
		               ey + e->getElementHeight());
	}

	view->rerenderRange(range);
}

void RelMoveUndoAction::repaint()
{
	XOJ_CHECK_TYPE(RelMoveUndoAction);

	if(!this->elements)
	{
		return;
	}

	sourceView->rerenderPage();
	if(targetView)
		targetView->rerenderPage();
}

XojPage** RelMoveUndoAction::getPages()
{
	XOJ_CHECK_TYPE(RelMoveUndoAction);

	XojPage** pages = new XojPage *[3];
	pages[0] = this->page;
	pages[1] = this->targetPage;
	pages[2] = NULL;
	return pages;
}

String RelMoveUndoAction::getText()
{
	XOJ_CHECK_TYPE(RelMoveUndoAction);

	return text;
}

