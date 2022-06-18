#include "ScaleUndoAction.h"

#include <cmath>   // for isfinite
#include <memory>  // for allocator, __shared_ptr_access, __share...

#include "model/Element.h"    // for Element
#include "model/PageRef.h"    // for PageRef
#include "model/XojPage.h"    // for XojPage
#include "undo/UndoAction.h"  // for UndoAction
#include "util/Range.h"       // for Range
#include "util/i18n.h"        // for _

class Control;

ScaleUndoAction::ScaleUndoAction(const PageRef& page, std::vector<Element*>* elements, double x0, double y0, double fx,
                                 double fy, double rotation, bool restoreLineWidth):
        UndoAction("ScaleUndoAction") {
    this->page = page;
    this->elements = *elements;
    this->x0 = x0;
    this->y0 = y0;
    this->fx = std::isfinite(fx) ? fx : 1.0;
    this->fy = std::isfinite(fy) ? fy : 1.0;
    this->rotation = rotation;
    this->restoreLineWidth = restoreLineWidth;
}

ScaleUndoAction::~ScaleUndoAction() { this->page = nullptr; }

auto ScaleUndoAction::undo(Control* control) -> bool {
    applyScale(1 / this->fx, 1 / this->fy, restoreLineWidth);
    this->undone = true;
    return true;
}

auto ScaleUndoAction::redo(Control* control) -> bool {
    applyScale(this->fx, this->fy, restoreLineWidth);
    this->undone = false;
    return true;
}

void ScaleUndoAction::applyScale(double fx, double fy, bool restoreLineWidth) {
    if (this->elements.empty()) {
        return;
    }

    Range r(elements.front()->getX(), elements.front()->getY());

    for (Element* e: this->elements) {
        r.addPoint(e->getX(), e->getY());
        r.addPoint(e->getX() + e->getElementWidth(), e->getY() + e->getElementHeight());
        e->scale(this->x0, this->y0, fx, fy, this->rotation, restoreLineWidth);
        r.addPoint(e->getX(), e->getY());
        r.addPoint(e->getX() + e->getElementWidth(), e->getY() + e->getElementHeight());
    }

    this->page->fireRangeChanged(r);
}

auto ScaleUndoAction::getText() -> std::string { return _("Scale"); }
