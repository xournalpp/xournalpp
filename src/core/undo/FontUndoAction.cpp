#include "FontUndoAction.h"

#include <algorithm>  // for max, min
#include <memory>     // for __shared_ptr_access, __shared_ptr_acces...

#include "control/Control.h"
#include "model/Document.h"
#include "model/Font.h"       // for XojFont
#include "model/Text.h"       // for Text
#include "model/XojPage.h"    // for XojPage
#include "undo/UndoAction.h"  // for UndoAction
#include "util/Rectangle.h"   // for Rectangle
#include "util/i18n.h"        // for _

using xoj::util::Rectangle;

class FontUndoActionEntry {
public:
    FontUndoActionEntry(Text* e, const XojFont& oldFont, const XojFont& newFont) {
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
    for (FontUndoActionEntry* e: this->data) { delete e; }
    this->data.clear();
}

void FontUndoAction::addStroke(Text* e, const XojFont& oldFont, const XojFont& newFont) {
    this->data.push_back(new FontUndoActionEntry(e, oldFont, newFont));
}

auto FontUndoAction::undo(Control* control) -> bool {
    if (this->data.empty()) {
        return true;
    }

    Range r;
    Document* doc = control->getDocument();
    doc->lock();

    for (FontUndoActionEntry* e: this->data) {
        r = r.unite(Range(e->e->getBoundingBox()));
        e->e->setFont(e->oldFont);
        r = r.unite(Range(e->e->getBoundingBox()));
    }

    doc->unlock();

    xoj_assert(!r.empty());
    this->page->fireRangeChanged(r);

    return true;
}

auto FontUndoAction::redo(Control* control) -> bool {
    if (this->data.empty()) {
        return true;
    }

    Range r;
    Document* doc = control->getDocument();
    doc->lock();

    for (FontUndoActionEntry* e: this->data) {
        r = r.unite(Range(e->e->getBoundingBox()));
        e->e->setFont(e->newFont);
        r = r.unite(Range(e->e->getBoundingBox()));
    }

    doc->unlock();

    xoj_assert(!r.empty());
    this->page->fireRangeChanged(r);

    return true;
}

auto FontUndoAction::getText() -> std::string { return _("Change font"); }
