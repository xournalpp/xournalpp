#include "FontUndoAction.h"

#include "gui/Redrawable.h"
#include "model/Font.h"
#include "model/Text.h"

#include "Rectangle.h"
#include "i18n.h"

class FontUndoActionEntry {
public:
    FontUndoActionEntry(Text* e, XojFont& oldFont, XojFont& newFont) {
        this->e = e;
        this->oldFont = oldFont;
        this->newFont = newFont;
    }

    Text* e;
    XojFont oldFont;
    XojFont newFont;
};

FontUndoAction::FontUndoAction(const PageRef& page, Layer* layer): UndoAction("FontUndoAction") {
    this->page = page;
    this->layer = layer;
}

FontUndoAction::~FontUndoAction() {
    for (FontUndoActionEntry* e: this->data) {
        delete e;
    }
    this->data.clear();
}

void FontUndoAction::addStroke(Text* e, XojFont& oldFont, XojFont& newFont) {
    this->data.push_back(new FontUndoActionEntry(e, oldFont, newFont));
}

auto FontUndoAction::undo(Control* control) -> bool {
    if (this->data.empty()) {
        return true;
    }

    FontUndoActionEntry* e = this->data.front();
    double x1 = e->e->getX();
    double x2 = e->e->getX() + e->e->getElementWidth();
    double y1 = e->e->getY();
    double y2 = e->e->getY() + e->e->getElementHeight();

    for (FontUndoActionEntry* e: this->data) {
        // size with old font
        x1 = std::min(x1, e->e->getX());
        x2 = std::max(x2, e->e->getX() + e->e->getElementWidth());
        y1 = std::min(y1, e->e->getY());
        y2 = std::max(y2, e->e->getY() + e->e->getElementHeight());

        e->e->setFont(e->oldFont);

        // size with new font
        x1 = std::min(x1, e->e->getX());
        x2 = std::max(x2, e->e->getX() + e->e->getElementWidth());
        y1 = std::min(y1, e->e->getY());
        y2 = std::max(y2, e->e->getY() + e->e->getElementHeight());
    }

    Rectangle<double> rect(x1, y1, x2 - x1, y2 - y1);
    this->page->fireRectChanged(rect);

    return true;
}

auto FontUndoAction::redo(Control* control) -> bool {
    if (this->data.empty()) {
        return true;
    }

    FontUndoActionEntry* e = this->data.front();
    double x1 = e->e->getX();
    double x2 = e->e->getX() + e->e->getElementWidth();
    double y1 = e->e->getY();
    double y2 = e->e->getY() + e->e->getElementHeight();

    for (FontUndoActionEntry* e: this->data) {
        // size with old font
        x1 = std::min(x1, e->e->getX());
        x2 = std::max(x2, e->e->getX() + e->e->getElementWidth());
        y1 = std::min(y1, e->e->getY());
        y2 = std::max(y2, e->e->getY() + e->e->getElementHeight());

        e->e->setFont(e->newFont);

        // size with new font
        x1 = std::min(x1, e->e->getX());
        x2 = std::max(x2, e->e->getX() + e->e->getElementWidth());
        y1 = std::min(y1, e->e->getY());
        y2 = std::max(y2, e->e->getY() + e->e->getElementHeight());
    }

    Rectangle<double> rect(x1, y1, x2 - x1, y2 - y1);
    this->page->fireRectChanged(rect);

    return true;
}

auto FontUndoAction::getText() -> string { return _("Change font"); }
