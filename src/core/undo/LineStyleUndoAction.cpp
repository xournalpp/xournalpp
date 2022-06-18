#include "LineStyleUndoAction.h"

#include <algorithm>  // for max, min
#include <memory>     // for __shared_ptr_access, __shared_ptr_acces...

#include "model/PageRef.h"    // for PageRef
#include "model/Stroke.h"     // for Stroke
#include "model/XojPage.h"    // for XojPage
#include "undo/UndoAction.h"  // for UndoAction
#include "util/Rectangle.h"   // for Rectangle
#include "util/i18n.h"        // for _

class Control;

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

    LineStyleUndoActionEntry e = this->data.front();
    double x1 = e.s->getX();
    double x2 = e.s->getX() + e.s->getElementWidth();
    double y1 = e.s->getY();
    double y2 = e.s->getY() + e.s->getElementHeight();

    for (LineStyleUndoActionEntry& e: this->data) {
        e.s->setLineStyle(e.oldStyle);

        x1 = std::min(x1, e.s->getX());
        x2 = std::max(x2, e.s->getX() + e.s->getElementWidth());
        y1 = std::min(y1, e.s->getY());
        y2 = std::max(y2, e.s->getY() + e.s->getElementHeight());
    }

    Rectangle rect(x1, y1, x2 - x1, y2 - y1);
    this->page->fireRectChanged(rect);

    return true;
}

auto LineStyleUndoAction::redo(Control* control) -> bool {
    if (this->data.empty()) {
        return true;
    }

    LineStyleUndoActionEntry e = this->data.front();
    double x1 = e.s->getX();
    double x2 = e.s->getX() + e.s->getElementWidth();
    double y1 = e.s->getY();
    double y2 = e.s->getY() + e.s->getElementHeight();

    for (LineStyleUndoActionEntry& e: this->data) {
        e.s->setLineStyle(e.newStyle);

        x1 = std::min(x1, e.s->getX());
        x2 = std::max(x2, e.s->getX() + e.s->getElementWidth());
        y1 = std::min(y1, e.s->getY());
        y2 = std::max(y2, e.s->getY() + e.s->getElementHeight());
    }

    Rectangle rect(x1, y1, x2 - x1, y2 - y1);
    this->page->fireRectChanged(rect);

    return true;
}

auto LineStyleUndoAction::getText() -> std::string { return _("Change line style"); }
