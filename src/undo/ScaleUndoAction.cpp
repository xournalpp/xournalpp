#include "ScaleUndoAction.h"

#include "model/Element.h"
#include "model/PageRef.h"

#include "Range.h"
#include "i18n.h"

ScaleUndoAction::ScaleUndoAction(const PageRef& page, vector<Element*>* elements, double x0, double y0, double fx,
                                 double fy):
        UndoAction("ScaleUndoAction") {
    this->page = page;
    this->elements = *elements;
    this->x0 = x0;
    this->y0 = y0;
    this->fx = fx;
    this->fy = fy;
}

ScaleUndoAction::~ScaleUndoAction() { this->page = nullptr; }

auto ScaleUndoAction::undo(Control* control) -> bool {
    applyScale(1 / this->fx, 1 / this->fy);
    this->undone = true;
    return true;
}

auto ScaleUndoAction::redo(Control* control) -> bool {
    applyScale(this->fx, this->fy);
    this->undone = false;
    return true;
}

void ScaleUndoAction::applyScale(double fx, double fy) {
    if (this->elements.empty()) {
        return;
    }

    Range r(elements.front()->getX(), elements.front()->getY());

    for (Element* e: this->elements) {
        r.addPoint(e->getX(), e->getY());
        r.addPoint(e->getX() + e->getElementWidth(), e->getY() + e->getElementHeight());
        e->scale(this->x0, this->y0, fx, fy);
        r.addPoint(e->getX(), e->getY());
        r.addPoint(e->getX() + e->getElementWidth(), e->getY() + e->getElementHeight());
    }

    this->page->fireRangeChanged(r);
}

auto ScaleUndoAction::getText() -> string { return _("Scale"); }
