#include "ReflectUndoAction.h"

#include "model/Element.h"
#include "model/PageRef.h"
#include "util/Range.h"
#include "util/i18n.h"

ReflectUndoAction::ReflectUndoAction(const PageRef& page, std::vector<Element*>* elements, double x0, double y0,
                                   bool x_axis):
        UndoAction("ReflectUndoAction") {
    this->page = page;
    this->elements = *elements;
    this->x0 = x0;
    this->y0 = y0;
    this->x_axis = x_axis;
}

ReflectUndoAction::~ReflectUndoAction() { this->page = nullptr; }

auto ReflectUndoAction::undo(Control* control) -> bool {
    applyReflection(this->x0, this->y0, this->x_axis);
    this->undone = true;
    return true;
}

auto ReflectUndoAction::redo(Control* control) -> bool {
    applyReflection(this->x0, this->y0, this->x_axis);
    this->undone = false;
    return true;
}

void ReflectUndoAction::applyReflection(double x0, double y0, bool x_axis) {
    if (this->elements.empty()) {
        return;
    }

    Range r(elements.front()->getX(), elements.front()->getY());

    for (Element* e: this->elements) {
        r.addPoint(e->getX(), e->getY());
        r.addPoint(e->getX() + e->getElementWidth(), e->getY() + e->getElementHeight());
        e->axisReflect(x0, y0, x_axis);
        r.addPoint(e->getX(), e->getY());
        r.addPoint(e->getX() + e->getElementWidth(), e->getY() + e->getElementHeight());
    }

    this->page->fireRangeChanged(r);
}

auto ReflectUndoAction::getText() -> std::string {  
	if(x_axis ) return _("Reflection About X axis");
	else return _("Reflection About Y axis");
}
