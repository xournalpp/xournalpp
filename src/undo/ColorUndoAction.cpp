#include "ColorUndoAction.h"

#include "gui/Redrawable.h"
#include "model/Element.h"
#include "model/PageRef.h"

#include "Rectangle.h"
#include "i18n.h"

class ColorUndoActionEntry {
public:
    ColorUndoActionEntry(Element* e, Color oldColor, Color newColor): e(e), oldColor(oldColor), newColor(newColor) {}
    Element* e;
    Color oldColor;
    Color newColor;
};

ColorUndoAction::ColorUndoAction(const PageRef& page, Layer* layer): UndoAction("ColorUndoAction") {
    this->page = page;
    this->layer = layer;
}

ColorUndoAction::~ColorUndoAction() {
    for (ColorUndoActionEntry* e: this->data) {
        delete e;
    }
}

void ColorUndoAction::addStroke(Element* e, Color originalColor, Color newColor) {
    this->data.push_back(new ColorUndoActionEntry(e, originalColor, newColor));
}

auto ColorUndoAction::undo(Control* control) -> bool {
    if (this->data.empty()) {
        return true;
    }

    ColorUndoActionEntry* e = this->data.front();
    double x1 = e->e->getX();
    double x2 = e->e->getX() + e->e->getElementWidth();
    double y1 = e->e->getY();
    double y2 = e->e->getY() + e->e->getElementHeight();

    for (ColorUndoActionEntry* e: this->data) {
        e->e->setColor(e->oldColor);

        x1 = std::min(x1, e->e->getX());
        x2 = std::max(x2, e->e->getX() + e->e->getElementWidth());
        y1 = std::min(y1, e->e->getY());
        y2 = std::max(y2, e->e->getY() + e->e->getElementHeight());
    }

    Rectangle rect(x1, y1, x2 - x1, y2 - y1);
    this->page->fireRectChanged(rect);

    return true;
}

auto ColorUndoAction::redo(Control* control) -> bool {
    if (this->data.empty()) {
        return true;
    }

    ColorUndoActionEntry* e = this->data.front();
    double x1 = e->e->getX();
    double x2 = e->e->getX() + e->e->getElementWidth();
    double y1 = e->e->getY();
    double y2 = e->e->getY() + e->e->getElementHeight();

    for (ColorUndoActionEntry* e: this->data) {
        e->e->setColor(e->newColor);

        x1 = std::min(x1, e->e->getX());
        x2 = std::max(x2, e->e->getX() + e->e->getElementWidth());
        y1 = std::min(y1, e->e->getY());
        y2 = std::max(y2, e->e->getY() + e->e->getElementHeight());
    }

    Rectangle rect(x1, y1, x2 - x1, y2 - y1);
    this->page->fireRectChanged(rect);

    return true;
}

auto ColorUndoAction::getText() -> string { return _("Change color"); }
