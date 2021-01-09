#include "LineStyleUndoAction.h"

#include "gui/Redrawable.h"
#include "model/PageRef.h"
#include "model/Stroke.h"

#include "Rectangle.h"
#include "i18n.h"

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

auto LineStyleUndoAction::getText() -> string { return _("Change line style"); }
