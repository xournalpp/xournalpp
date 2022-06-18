#include "RotateUndoAction.h"

#include <memory>  // for allocator, __shared_ptr_access, __share...

#include "model/Element.h"    // for Element
#include "model/PageRef.h"    // for PageRef
#include "model/XojPage.h"    // for XojPage
#include "undo/UndoAction.h"  // for UndoAction
#include "util/Range.h"       // for Range
#include "util/i18n.h"        // for _

class Control;

RotateUndoAction::RotateUndoAction(const PageRef& page, std::vector<Element*>* elements, double x0, double y0,
                                   double rotation):
        UndoAction("RotateUndoAction") {
    this->page = page;
    this->elements = *elements;
    this->x0 = x0;
    this->y0 = y0;
    this->rotation = rotation;
}

RotateUndoAction::~RotateUndoAction() { this->page = nullptr; }

auto RotateUndoAction::undo(Control* control) -> bool {
    applyRotation(-this->rotation);
    this->undone = true;
    return true;
}

auto RotateUndoAction::redo(Control* control) -> bool {
    applyRotation(this->rotation);
    this->undone = false;
    return true;
}

void RotateUndoAction::applyRotation(double rotation) {
    if (this->elements.empty()) {
        return;
    }

    Range r(elements.front()->getX(), elements.front()->getY());

    for (Element* e: this->elements) {
        r.addPoint(e->getX(), e->getY());
        r.addPoint(e->getX() + e->getElementWidth(), e->getY() + e->getElementHeight());
        e->rotate(this->x0, this->y0, rotation);
        r.addPoint(e->getX(), e->getY());
        r.addPoint(e->getX() + e->getElementWidth(), e->getY() + e->getElementHeight());
    }

    this->page->fireRangeChanged(r);
}

auto RotateUndoAction::getText() -> std::string { return _("Rotation"); }
