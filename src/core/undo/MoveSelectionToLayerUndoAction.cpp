#include "MoveSelectionToLayerUndoAction.h"

#include <memory>  // for __shared_ptr_access, __shared_pt...

#include <glib.h>  // for g_warning

#include "model/Element.h"           // for Element, ELEMENT_IMAGE, ELEMENT_...
#include "model/Layer.h"             // for Layer
#include "model/PageRef.h"           // for PageRef
#include "model/XojPage.h"           // for XojPage
#include "undo/PageLayerPosEntry.h"  // for PageLayerPosEntry, operator<
#include "undo/UndoAction.h"         // for UndoAction
#include "util/i18n.h"               // for _

class Control;


MoveSelectionToLayerUndoAction::MoveSelectionToLayerUndoAction(const PageRef& page, LayerController* layerController, Layer* oldLayer, size_t oldLayerNo, size_t newLayerNo): UndoAction("MoveSelectionToLayerUndoAction") {
    this->page = page;
    this->layerController = layerController;
    this->oldLayer = oldLayer;
    this->oldLayerNo = oldLayerNo;
    this->newLayerNo = newLayerNo;
}

auto MoveSelectionToLayerUndoAction::getText() -> std::string {
    return "Move selection to layer";
}

auto MoveSelectionToLayerUndoAction::undo(Control* control) -> bool {
    if (elements.empty()) {
        this->undone = false;
        return false;
    }

    for (const auto& elem: elements) {
        elem.layer->removeElement(elem.element, false);
        this->oldLayer->addElement(elem.element);
    }

    this->layerController->switchToLay(oldLayerNo + 1, false, false);
    this->undone = false;
    return true;
}

auto MoveSelectionToLayerUndoAction::redo(Control* control) -> bool {
    if (elements.empty()) {
        this->undone = true;
        return false;
    }

    for (const auto& elem: elements) {
        this->oldLayer->removeElement(elem.element, false);
        elem.layer->insertElement(elem.element, elem.pos);
    }

    this->layerController->switchToLay(newLayerNo + 1);
    this->undone = true;
    return true;
}

void MoveSelectionToLayerUndoAction::addElement(Layer* layer, Element* e, int pos) {
    elements.emplace(layer, e, pos);
}
