#include "MoveUndoAction.h"

#include "../model/Element.h"
#include "../model/Page.h"
#include "../model/Layer.h"
#include "../gui/Redrawable.h"
#include "../control/tools/EditSelection.h"
#include "../control/tools/VerticalToolHandler.h"
// TODO: AA: type check

class MoveUndoEntry {
public:
	MoveUndoEntry(Element * e, double x, double y) {
		this->e = e;
		this->x = x;
		this->y = y;
	}

	Element * e;
	double x;
	double y;
};

//MoveUndoAction::MoveUndoAction(XojPage * page, EditSelection * selection) {
//	this->page = page;
//	this->newPage = NULL;
//	this->originalPos = NULL;
//	this->newPos = NULL;
//	this->oldLayer = selection->layer;
//	this->newLayer = NULL;
//	this->origView = selection->view;
//	this->newView = NULL;
//	this->text = _("Move");
//
//	double x = selection->x - selection->relativeX;
//	double y = selection->y - selection->relativeY;
//
//	for (GList * l = selection->selected; l != NULL; l = l->next) {
//		Element * e = (Element *) l->data;
//		this->originalPos = g_list_append(this->originalPos, new MoveUndoEntry(e, e->getX() + x, e->getY() + y));
//	}
//}

MoveUndoAction::MoveUndoAction(XojPage * page, VerticalToolHandler * handler) {
	this->page = page;
	this->newPage = NULL;
	this->originalPos = NULL;
	this->newPos = NULL;
	this->oldLayer = handler->layer;
	this->newLayer = NULL;
	this->origView = handler->view;
	this->newView = NULL;
	this->text = _("Vertical Space");

	ListIterator<Element *> it = handler->getElements();
	while (it.hasNext()) {
		Element * e = it.next();
		this->originalPos = g_list_append(this->originalPos, new MoveUndoEntry(e, e->getX(), e->getY()));
	}
}

MoveUndoAction::~MoveUndoAction() {
	for (GList * l = this ->originalPos; l != NULL; l = l->next) {
		MoveUndoEntry * u = (MoveUndoEntry *) l->data;
		delete u;
	}
	for (GList * l = this->newPos; l != NULL; l = l->next) {
		MoveUndoEntry * u = (MoveUndoEntry *) l->data;
		delete u;
	}
	g_list_free(this->originalPos);
}

//void MoveUndoAction::finalize(EditSelection * selection) {
//	for (GList * l = selection->selected; l != NULL; l = l->next) {
//		Element * e = (Element *) l->data;
//		this->newPos = g_list_append(this->newPos, new MoveUndoEntry(e,
//				e->getX() + selection->x - selection->relativeX, e->getY() + selection->y - selection->relativeY));
//	}
//
//	if (this->page != selection->page) {
//		this->newPage = selection->page;
//		this->newLayer = selection->layer;
//		this->newView = selection->view;
//	}
//}

void MoveUndoAction::finalize(VerticalToolHandler * handler) {
	ListIterator<Element *> it = handler->getElements();
	while (it.hasNext()) {
		Element * e = it.next();
		this ->newPos = g_list_append(this->newPos, new MoveUndoEntry(e, e->getX(), e->getY()));
	}
}

bool MoveUndoAction::undo(Control * control) {
	if (this->oldLayer != this->newLayer && this->newLayer != NULL) {
		switchLayer(this->originalPos, this->newLayer, this->oldLayer);
	}

	acceptPositions(this->originalPos);

	repaint();

	return true;
}

bool MoveUndoAction::redo(Control * control) {
	if (this->oldLayer != this->newLayer && this->newLayer != NULL) {
		switchLayer(this->originalPos, this->oldLayer, this->newLayer);
	}

	acceptPositions(this->newPos);

	repaint();

	return true;
}

void MoveUndoAction::acceptPositions(GList * pos) {
	for (GList * l = pos; l != NULL; l = l->next) {
		MoveUndoEntry * u = (MoveUndoEntry *) l->data;
		Element * e = u->e;

		e->move(u->x - e->getX(), u->y - e->getY());
	}
}

void MoveUndoAction::switchLayer(GList * entries, Layer * oldLayer, Layer * newLayer) {
	for (GList * l = this->originalPos; l != NULL; l = l->next) {
		MoveUndoEntry * u = (MoveUndoEntry *) l->data;
		oldLayer->removeElement(u->e, false);
		newLayer->addElement(u->e);
	}
}

void MoveUndoAction::repaint(Redrawable * view, GList * list) {
	MoveUndoEntry * u = (MoveUndoEntry *) list->data;

	Range range(u->x, u->y);

	for (GList * l = list; l != NULL; l = l->next) {
		u = (MoveUndoEntry *) l->data;
		range.addPoint(u->x, u->y);
		range.addPoint(u->x + u->e->getElementWidth(), u->y + u->e->getElementHeight());
	}

	view->rerenderRange(range);
}

void MoveUndoAction::repaint() {
	if (!this->originalPos) {
		return;
	}

	repaint(this->origView, this->originalPos);
	repaint(this->newView != NULL ? this->newView : this->origView, this->newPos);
}

XojPage ** MoveUndoAction::getPages() {
	XojPage ** pages = new XojPage *[3];
	pages[0] = this->page;
	pages[1] = this->newPage;
	pages[2] = NULL;
	return pages;
}

String MoveUndoAction::getText() {
	return text;
}

