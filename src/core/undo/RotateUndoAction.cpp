#include "RotateUndoAction.h"

#include <memory>  // for allocator, __shared_ptr_access, __share...

#include "control/Control.h"
#include "model/Document.h"
#include "model/Element.h"    // for Element
#include "model/PageRef.h"    // for PageRef
#include "model/XojPage.h"    // for XojPage
#include "undo/UndoAction.h"  // for UndoAction
#include "util/Range.h"       // for Range
#include "util/i18n.h"        // for _

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
    applyRotation(-this->rotation, control->getDocument());
    this->undone = true;
    return true;
}

auto RotateUndoAction::redo(Control* control) -> bool {
    applyRotation(this->rotation, control->getDocument());
    this->undone = false;
    return true;
}

void RotateUndoAction::applyRotation(double rotation, Document* doc) {
    if (this->elements.empty()) {
        return;
    }

    Range r;
    doc->lock();

    for (Element* e: this->elements) {
        r = r.unite(Range(e->getBoundingBox()));
        e->rotate(this->x0, this->y0, rotation);
        r = r.unite(Range(e->getBoundingBox()));
    }

    doc->unlock();

    xoj_assert(!r.empty());
    this->page->fireRangeChanged(r);
}

auto RotateUndoAction::getText() -> std::string { return _("Rotation"); }
