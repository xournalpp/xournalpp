#include "MoveUndoAction.h"

#include "control/tools/EditSelection.h"
#include "control/tools/VerticalToolHandler.h"
#include "gui/Redrawable.h"
#include "model/Element.h"
#include "model/Layer.h"
#include "model/PageRef.h"

#include <i18n.h>

MoveUndoAction::MoveUndoAction(Layer* sourceLayer, PageRef sourcePage, vector<Element*>* selected, double mx, double my,
							   Layer* targetLayer, PageRef targetPage)
 : UndoAction("MoveUndoAction")
{
	this->page = sourcePage;
	this->sourceLayer = sourceLayer;
	this->text = _("Move");

	this->dx = mx;
	this->dy = my;

	this->elements = *selected;

	if (this->page != targetPage)
	{
		this->targetPage = targetPage;
		this->targetLayer = targetLayer;
	}
}

MoveUndoAction::~MoveUndoAction()
{
}

void MoveUndoAction::move()
{
	if (this->undone)
	{
		for (Element* e : this->elements)
		{
			e->move(dx, dy);
		}
	}
	else
	{
		for (Element* e : this->elements)
		{
			e->move(-dx, -dy);
		}
	}
}

bool MoveUndoAction::undo(Control* control)
{
	if (this->sourceLayer != this->targetLayer && this->targetLayer != nullptr)
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
	if (this->sourceLayer != this->targetLayer && this->targetLayer != nullptr)
	{
		switchLayer(&this->elements, this->sourceLayer, this->targetLayer);
	}

	move();
	repaint();
	this->undone = false;

	return true;
}

void MoveUndoAction::switchLayer(vector<Element*>* entries, Layer* oldLayer, Layer* newLayer)
{
	for (Element* e : this->elements)
	{
		oldLayer->removeElement(e, false);
		newLayer->addElement(e);
	}
}

void MoveUndoAction::repaint()
{
	if (this->elements.empty()) return;

	this->page->firePageChanged();

	if (this->targetPage.isValid())
	{
		this->targetPage->firePageChanged();
	}
}

vector<PageRef> MoveUndoAction::getPages()
{
	vector<PageRef> pages;
	pages.push_back(this->page);
	pages.push_back(this->targetPage);
	return pages;
}

string MoveUndoAction::getText()
{
	return text;
}
