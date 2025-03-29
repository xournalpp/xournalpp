#include "MoveSelectionToLayerUndoAction.h"

#include <memory>  // for __shared_ptr_access, __shared_pt...

#include <glib.h>  // for g_warning

#include "control/Control.h"
#include "model/Document.h"
#include "model/Element.h"           // for Element, ELEMENT_IMAGE, ELEMENT_...
#include "model/Layer.h"             // for Layer
#include "model/PageRef.h"           // for PageRef
#include "model/XojPage.h"           // for XojPage
#include "undo/PageLayerPosEntry.h"  // for PageLayerPosEntry, operator<
#include "undo/UndoAction.h"         // for UndoAction
#include "util/i18n.h"               // for _

MoveSelectionToLayerUndoAction::MoveSelectionToLayerUndoAction(const PageRef& page, LayerController* layerController, Layer* oldLayer, size_t oldLayerNo, size_t newLayerNo):
        UndoAction("MoveSelectionToLayerUndoAction"),
        layerController(layerController),
        oldLayer(oldLayer),
        oldLayerNo(oldLayerNo),
        newLayerNo(newLayerNo) {
    this->page = page;
}

auto MoveSelectionToLayerUndoAction::getText() -> std::string {
    return "Move selection to layer";
}

auto MoveSelectionToLayerUndoAction::undo(Control* control) -> bool {
    if (elements.empty()) {
        this->undone = false;
        return false;
    }

    Document* doc = control->getDocument();
    doc->lock();
    for (const auto& elem: elements) {
        this->oldLayer->addElement(elem.layer->removeElement(elem.element).e);
    }

    this->layerController->switchToLay(oldLayerNo + 1);
    doc->unlock();
    this->undone = false;
    return true;
}

auto MoveSelectionToLayerUndoAction::redo(Control* control) -> bool {
    if (elements.empty()) {
        this->undone = true;
        return false;
    }

    Document* doc = control->getDocument();
    doc->lock();
    for (const auto& elem: elements) {
        elem.layer->insertElement(this->oldLayer->removeElement(elem.element).e, elem.pos);
    }

    this->layerController->switchToLay(newLayerNo + 1);
    doc->unlock();
    this->undone = true;
    return true;
}

void MoveSelectionToLayerUndoAction::addElement(Layer* layer, const Element* e, Element::Index pos) {
    elements.emplace(layer, e, pos);
}
