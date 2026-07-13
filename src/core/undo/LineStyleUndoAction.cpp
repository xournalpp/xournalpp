#include "LineStyleUndoAction.h"

#include <algorithm>  // for max, min
#include <memory>     // for __shared_ptr_access, __shared_ptr_acces...

#include "control/Control.h"
#include "model/Document.h"
#include "model/PageRef.h"    // for PageRef
#include "model/Stroke.h"     // for Stroke
#include "model/XojPage.h"    // for XojPage
#include "undo/UndoAction.h"  // for UndoAction
#include "util/Rectangle.h"   // for Rectangle
#include "util/i18n.h"        // for _

using xoj::util::Rectangle;

LineStyleUndoAction::LineStyleUndoAction(const PageRef& page, Layer* layer): UndoAction("LineStyleUndoAction") {
    this->page = page;
    this->layer = layer;
}

void LineStyleUndoAction::addStroke(Stroke* s, LineStyle originalStyle, LineStyle newStyle) {
    this->data.emplace_back(s, originalStyle, newStyle);
}

auto LineStyleUndoAction::undo(Control* control) -> bool {
    if (this->data.empty()) {
        return true;
    }

    Range r;
    Document* doc = control->getDocument();
    doc->lock();

    for (LineStyleUndoActionEntry& e: this->data) {
        e.s->setLineStyle(e.oldStyle);
        r = r.unite(Range(e.s->getBoundingBox()));
    }

    doc->unlock();

    xoj_assert(!r.empty());
    this->page->fireRangeChanged(r);

    return true;
}

auto LineStyleUndoAction::redo(Control* control) -> bool {
    if (this->data.empty()) {
        return true;
    }

    Range r;
    Document* doc = control->getDocument();
    doc->lock();

    for (LineStyleUndoActionEntry& e: this->data) {
        e.s->setLineStyle(e.newStyle);
        r = r.unite(Range(e.s->getBoundingBox()));
    }

    doc->unlock();

    xoj_assert(!r.empty());
    this->page->fireRangeChanged(r);

    return true;
}

auto LineStyleUndoAction::getText() -> std::string { return _("Change line style"); }
