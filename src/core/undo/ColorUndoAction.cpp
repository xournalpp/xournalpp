#include "ColorUndoAction.h"

#include <algorithm>  // for max, min
#include <memory>     // for __shared_ptr_access, __shared_ptr_acces...

#include "control/Control.h"
#include "model/Document.h"
#include "model/Element.h"    // for Element
#include "model/XojPage.h"    // for XojPage
#include "undo/UndoAction.h"  // for UndoAction
#include "util/Rectangle.h"   // for Rectangle
#include "util/i18n.h"        // for _

using xoj::util::Rectangle;

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
    for (ColorUndoActionEntry* e: this->data) { delete e; }
}

void ColorUndoAction::addStroke(Element* e, Color originalColor, Color newColor) {
    this->data.push_back(new ColorUndoActionEntry(e, originalColor, newColor));
}

auto ColorUndoAction::undo(Control* control) -> bool {
    if (this->data.empty()) {
        return true;
    }

    Range range;
    Document* doc = control->getDocument();
    doc->lock();

    for (ColorUndoActionEntry* e: this->data) {
        e->e->setColor(e->oldColor);
        range = range.unite(Range(e->e->getBoundingBox()));
    }

    doc->unlock();

    xoj_assert(!range.empty());
    this->page->fireRangeChanged(range);

    return true;
}

auto ColorUndoAction::redo(Control* control) -> bool {
    if (this->data.empty()) {
        return true;
    }

    Range range;
    Document* doc = control->getDocument();
    doc->lock();

    for (ColorUndoActionEntry* e: this->data) {
        e->e->setColor(e->newColor);
        range = range.unite(Range(e->e->getBoundingBox()));
    }

    doc->unlock();

    xoj_assert(!range.empty());
    this->page->fireRangeChanged(range);

    return true;
}

auto ColorUndoAction::getText() -> std::string { return _("Change color"); }
