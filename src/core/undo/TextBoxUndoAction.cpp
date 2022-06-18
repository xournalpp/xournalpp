#include "TextBoxUndoAction.h"

#include <algorithm>  // for max, min
#include <memory>     // for allocator, __shared_ptr_access, __share...

#include "model/Element.h"    // for Element
#include "model/Layer.h"      // for Layer
#include "model/PageRef.h"    // for PageRef
#include "model/XojPage.h"    // for XojPage
#include "undo/UndoAction.h"  // for UndoAction
#include "util/Rectangle.h"   // for Rectangle
#include "util/i18n.h"        // for _

class Control;

using xoj::util::Rectangle;

TextBoxUndoAction::TextBoxUndoAction(const PageRef& page, Layer* layer, Element* element, Element* oldelement):
        UndoAction("TextBoxUndoAction") {
    this->page = page;
    this->layer = layer;
    this->element = element;
    this->oldelement = oldelement;
}

TextBoxUndoAction::~TextBoxUndoAction() {
    if (this->undone) {
        // Insert was undone, so this is not needed anymore
        if (this->layer->indexOf(element) == -1) {
            delete this->element;
        }
        //	we won't be able to delete the old element, as it will
        //	get cleaned up in the next TextBoxUndoAction cleanup.
    } else if (this->layer->indexOf(oldelement) == -1) {
        delete this->oldelement;
        // if it hasn't been undone we clear out the old element,
        // since that won't be used in the future and isn't drawn.
    }
    this->element = nullptr;
    this->oldelement = nullptr;
}

auto TextBoxUndoAction::getText() -> std::string { return _("Edit text"); }

auto TextBoxUndoAction::undo(Control* control) -> bool {
    this->layer->removeElement(this->element, false);
    this->layer->addElement(this->oldelement);

    double x1 = element->getX();
    double y1 = element->getY();
    double x2 = element->getX() + element->getElementWidth();
    double y2 = element->getY() + element->getElementHeight();

    x1 = std::min(x1, oldelement->getX());
    y1 = std::min(y1, oldelement->getY());
    x2 = std::max(x2, oldelement->getX() + oldelement->getElementWidth());
    y2 = std::max(y2, oldelement->getY() + oldelement->getElementHeight());

    Rectangle<double> rect(x1, y1, x2 - x1, y2 - y1);
    this->page->fireRectChanged(rect);

    this->undone = true;

    return true;
}

auto TextBoxUndoAction::redo(Control* control) -> bool {
    this->layer->removeElement(this->oldelement, false);
    this->layer->addElement(this->element);

    double x1 = oldelement->getX();
    double y1 = oldelement->getY();
    double x2 = oldelement->getX() + oldelement->getElementWidth();
    double y2 = oldelement->getY() + oldelement->getElementHeight();

    x1 = std::min(x1, element->getX());
    y1 = std::min(y1, element->getY());
    x2 = std::max(x2, element->getX() + element->getElementWidth());
    y2 = std::max(y2, element->getY() + element->getElementHeight());

    Rectangle rect(x1, y1, x2 - x1, y2 - y1);
    this->page->fireRectChanged(rect);

    this->undone = false;

    return true;
}
