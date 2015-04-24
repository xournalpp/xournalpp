#include "MoveUndoAction.h"

#include "model/Element.h"
#include "model/PageRef.h"
#include "model/Layer.h"
#include "gui/Redrawable.h"
#include "control/tools/EditSelection.h"
#include "control/tools/VerticalToolHandler.h"

MoveUndoAction::MoveUndoAction(Layer* sourceLayer, PageRef sourcePage,
							   ElementVector* selected, double mx, double my,
							   Layer* targetLayer,
							   PageRef targetPage) : UndoAction("MoveUndoAction")
{
	XOJ_INIT_TYPE(MoveUndoAction);

	this->page = sourcePage;
	this->sourceLayer = sourceLayer;
	this->text = _("Move");

	this->targetLayer = NULL;
	this->targetPage = NULL;

	this->dx = mx;
	this->dy = my;

	this->elements = *selected;

	if (this->page != targetPage)
	{
		this->targetPage = targetPage;
		this->targetLayer = targetLayer;
	}

	this->undone = false;
}

MoveUndoAction::~MoveUndoAction()
{
	XOJ_CHECK_TYPE(MoveUndoAction);

	XOJ_RELEASE_TYPE(MoveUndoAction);
}

void MoveUndoAction::move()
{
	if (this->undone)
	{
		for (Element* e : this->elements) e->move(dx, dy);
	}
	else
	{
		for (Element* e : this->elements) e->move(-dx, -dy);
	}
}

bool MoveUndoAction::undo(Control* control)
{
	XOJ_CHECK_TYPE(MoveUndoAction);

	if (this->sourceLayer != this->targetLayer && this->targetLayer != NULL)
	{
		switchLayer(&this->elements, this->targetLayer, this->sourceLayer);
	}

	move();
	repaint();
	this->undone = true;

	return true;
}

bool MoveUndoAction::redo(Control* control)
{
	XOJ_CHECK_TYPE(MoveUndoAction);

	if (this->sourceLayer != this->targetLayer && this->targetLayer != NULL)
	{
		switchLayer(&this->elements, this->sourceLayer, this->targetLayer);
	}

	move();
	repaint();
	this->undone = false;

	return true;
}

void MoveUndoAction::switchLayer(ElementVector* entries,
								 Layer* oldLayer,
								 Layer* newLayer)
{
	XOJ_CHECK_TYPE(MoveUndoAction);

	for (Element* e : this->elements)
	{
		oldLayer->removeElement(e, false);
		newLayer->addElement(e);
	}
}

void MoveUndoAction::repaint()
{
	XOJ_CHECK_TYPE(MoveUndoAction);

	if (this->elements.empty()) return;

	this->page->firePageChanged();

	if (this->targetPage.isValid())
	{
		this->targetPage->firePageChanged();
	}
}

XojPage** MoveUndoAction::getPages()
{
	XOJ_CHECK_TYPE(MoveUndoAction);

	XojPage** pages = new XojPage *[3];
	pages[0] = this->page;
	pages[1] = this->targetPage;
	pages[2] = NULL;
	return pages;
}

string MoveUndoAction::getText()
{
	XOJ_CHECK_TYPE(MoveUndoAction);

	return text;
}

