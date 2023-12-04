#include "RecognizerUndoAction.h"

#include <memory>  // for __shared_ptr_access, __shared_ptr_acces...
#include <utility>

#include <glib.h>  // for g_warning

#include "model/Element.h"
#include "model/Layer.h"      // for Layer
#include "model/Stroke.h"     // for Stroke
#include "model/XojPage.h"    // for XojPage
#include "undo/UndoAction.h"  // for UndoAction
#include "util/i18n.h"        // for _

class Control;

RecognizerUndoAction::RecognizerUndoAction(const PageRef& page, Layer* layer, ElementPtr original, Element* recognized):
        UndoAction("RecognizerUndoAction"),
        layer(layer),
        original(original.get()),
        originalOwned(std::move(original)),
        recognized(recognized) {
    this->page = page;
}

RecognizerUndoAction::~RecognizerUndoAction() = default;

auto RecognizerUndoAction::undo(Control* control) -> bool {
    auto [owned, pos] = this->layer->removeElement(this->recognized);
    this->recognizedOwned = std::move(owned);

    this->layer->insertElement(std::move(this->originalOwned), pos);

    this->page->fireElementChanged(this->recognized);
    this->page->fireElementChanged(original);

    this->undone = true;
    return true;
}

auto RecognizerUndoAction::redo(Control* control) -> bool {
    Element::Index pos = 0;

    auto [owned, posi] = this->layer->removeElement(original);
    this->originalOwned = std::move(owned);
    this->layer->insertElement(std::move(this->recognizedOwned), pos);

    this->page->fireElementChanged(original);
    this->page->fireElementChanged(this->recognized);

    this->undone = false;
    return true;
}

auto RecognizerUndoAction::getText() -> std::string { return _("Stroke recognizer"); }
