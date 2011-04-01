#include "MoveUndoAction.h"

#include "../model/Element.h"
#include "../model/PageRef.h"
#include "../model/Layer.h"
#include "../gui/Redrawable.h"
#include "../control/tools/EditSelection.h"
#include "../control/tools/VerticalToolHandler.h"

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

MoveUndoAction::MoveUndoAction(Layer * sourceLayer, PageRef sourcePage, Redrawable * sourceView, GList * selected, double mx, double my, Layer * targetLayer,
		PageRef targetPage, Redrawable * targetView) {
	XOJ_INIT_TYPE(MoveUndoAction);

	this->page = sourcePage;
	this->sourceLayer = sourceLayer;
	this->sourceView = sourceView;
	this->text = _("Move");

	this->targetView = NULL;
	this->targetLayer = NULL;
	this->targetPage = NULL;

	this->targetPos = NULL;
	this->sourcePos = NULL;

	for (GList * l = selected; l != NULL; l = l->next) {
		Element * e = (Element *) l->data;
		this->sourcePos = g_list_append(this->sourcePos, new MoveUndoEntry(e, e->getX() - mx, e->getY() - my));
		this->targetPos = g_list_append(this->targetPos, new MoveUndoEntry(e, e->getX(), e->getY()));
	}

	if (this->page != targetPage) {
		this->targetPage = targetPage;
		this->targetLayer = targetLayer;
		this->targetView = targetView;
	}
}

MoveUndoAction::MoveUndoAction(PageRef sourcePage, VerticalToolHandler * handler) {
	XOJ_INIT_TYPE(MoveUndoAction);

	this->page = sourcePage;
	this->targetPage = NULL;
	this->sourcePos = NULL;
	this->targetPos = NULL;
	this->sourceLayer = handler->layer;
	this->targetLayer = NULL;
	this->sourceView = handler->view;
	this->targetView = NULL;
	this->text = _("Vertical Space");

	ListIterator<Element *> it = handler->getElements();
	while (it.hasNext()) {
		Element * e = it.next();
		this->sourcePos = g_list_append(this->sourcePos, new MoveUndoEntry(e, e->getX(), e->getY()));
	}
}

MoveUndoAction::~MoveUndoAction() {
	XOJ_CHECK_TYPE(MoveUndoAction);

	for (GList * l = this ->sourcePos; l != NULL; l = l->next) {
		MoveUndoEntry * u = (MoveUndoEntry *) l->data;
		delete u;
	}
	g_list_free(this->sourcePos);
	this->sourcePos = NULL;

	for (GList * l = this->targetPos; l != NULL; l = l->next) {
		MoveUndoEntry * u = (MoveUndoEntry *) l->data;
		delete u;
	}
	g_list_free(this->targetPos);
	this->targetPos = NULL;

	XOJ_RELEASE_TYPE(MoveUndoAction);
}

void MoveUndoAction::finalize(VerticalToolHandler * handler) {
	XOJ_CHECK_TYPE(MoveUndoAction);

	ListIterator<Element *> it = handler->getElements();
	while (it.hasNext()) {
		Element * e = it.next();
		this ->targetPos = g_list_append(this->targetPos, new MoveUndoEntry(e, e->getX(), e->getY()));
	}
}

bool MoveUndoAction::undo(Control * control) {
	XOJ_CHECK_TYPE(MoveUndoAction);

	if (this->sourceLayer != this->targetLayer && this->targetLayer != NULL) {
		switchLayer(this->sourcePos, this->targetLayer, this->sourceLayer);
	}

	acceptPositions(this->sourcePos);

	repaint();

	return true;
}

bool MoveUndoAction::redo(Control * control) {
	XOJ_CHECK_TYPE(MoveUndoAction);

	if (this->sourceLayer != this->targetLayer && this->targetLayer != NULL) {
		switchLayer(this->sourcePos, this->sourceLayer, this->targetLayer);
	}

	acceptPositions(this->targetPos);

	repaint();

	return true;
}

void MoveUndoAction::acceptPositions(GList * pos) {
	XOJ_CHECK_TYPE(MoveUndoAction);

	for (GList * l = pos; l != NULL; l = l->next) {
		MoveUndoEntry * u = (MoveUndoEntry *) l->data;
		Element * e = u->e;

		e->move(u->x - e->getX(), u->y - e->getY());
	}
}

void MoveUndoAction::switchLayer(GList * entries, Layer * oldLayer, Layer * newLayer) {
	XOJ_CHECK_TYPE(MoveUndoAction);

	for (GList * l = this->sourcePos; l != NULL; l = l->next) {
		MoveUndoEntry * u = (MoveUndoEntry *) l->data;
		oldLayer->removeElement(u->e, false);
		newLayer->addElement(u->e);
	}
}

void MoveUndoAction::repaint(Redrawable * view, GList * list, GList * list2) {
	XOJ_CHECK_TYPE(MoveUndoAction);

	MoveUndoEntry * u = (MoveUndoEntry *) list->data;

	Range range(u->x, u->y);

	for (GList * l = list; l != NULL; l = l->next) {
		u = (MoveUndoEntry *) l->data;
		range.addPoint(u->x, u->y);
		range.addPoint(u->x + u->e->getElementWidth(), u->y + u->e->getElementHeight());
	}

	for (GList * l = list2; l != NULL; l = l->next) {
		u = (MoveUndoEntry *) l->data;
		range.addPoint(u->x, u->y);
		range.addPoint(u->x + u->e->getElementWidth(), u->y + u->e->getElementHeight());
	}

	view->rerenderRange(range);
}

void MoveUndoAction::repaint() {
	XOJ_CHECK_TYPE(MoveUndoAction);

	if (!this->sourcePos) {
		return;
	}

	if(this->targetView == NULL) {
		repaint(this->sourceView, this->sourcePos, this->targetPos);
	} else {
		repaint(this->sourceView, this->sourcePos);
		repaint(this->targetView, this->targetPos);
	}
}

XojPage ** MoveUndoAction::getPages() {
	XOJ_CHECK_TYPE(MoveUndoAction);

	XojPage ** pages = new XojPage *[3];
	pages[0] = this->page;
	pages[1] = this->targetPage;
	pages[2] = NULL;
	return pages;
}

String MoveUndoAction::getText() {
	XOJ_CHECK_TYPE(MoveUndoAction);

	return text;
}

