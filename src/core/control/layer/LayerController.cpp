#include "LayerController.h"

#include <memory>   // for __shared_ptr_access, make...
#include <utility>  // for move
#include <vector>   // for vector

#include "control/Control.h"                // for Control
#include "gui/MainWindow.h"                 // for MainWindow
#include "gui/XournalView.h"                // for XournalView
#include "gui/dialog/RenameLayerDialog.h"   // for RenameLayerDialog
#include "model/Document.h"                 // for Document
#include "model/XojPage.h"                  // for XojPage
#include "undo/InsertLayerUndoAction.h"     // for InsertLayerUndoAction
#include "undo/MergeLayerDownUndoAction.h"  // for MergeLayerDownUndoAction
#include "undo/MoveLayerUndoAction.h"       // for MoveLayerUndoAction
#include "undo/RemoveLayerUndoAction.h"     // for RemoveLayerUndoAction
#include "undo/UndoAction.h"                // for UndoActionPtr, UndoAction
#include "undo/UndoRedoHandler.h"           // for UndoRedoHandler
#include "util/Util.h"                      // for npos
#include "util/i18n.h"                      // for FS, _F

#include "LayerCtrlListener.h"  // for LayerCtrlListener

LayerController::LayerController(Control* control): control(control), selectedPage(npos) {}

void LayerController::documentChanged(DocumentChangeType type) {
    if (type == DOCUMENT_CHANGE_CLEARED || type == DOCUMENT_CHANGE_COMPLETE) {
        fireRebuildLayerMenu();
    }
}

void LayerController::pageSelected(size_t page) {
    if (selectedPage == page) {
        return;
    }
    selectedPage = page;

    fireRebuildLayerMenu();
}

void LayerController::insertLayer(PageRef page, Layer* layer, Layer::Index layerPos) {
    page->insertLayer(layer, layerPos);
    fireRebuildLayerMenu();
}

void LayerController::removeLayer(PageRef page, Layer* layer) {
    page->removeLayer(layer);
    fireRebuildLayerMenu();
}

void LayerController::addListener(LayerCtrlListener* listener) { this->listener.push_back(listener); }

void LayerController::removeListener(LayerCtrlListener* listener) { this->listener.remove(listener); }

void LayerController::fireRebuildLayerMenu() {
    for (LayerCtrlListener* l: this->listener) { l->rebuildLayerMenu(); }
}

void LayerController::fireLayerVisibilityChanged() {
    for (LayerCtrlListener* l: this->listener) { l->layerVisibilityChanged(); }
}

auto LayerController::actionPerformed(ActionType type) -> bool {
    switch (type) {
        case ACTION_NEW_LAYER:
            addNewLayer();
            return true;

        case ACTION_DELETE_LAYER:
            deleteCurrentLayer();
            return true;

        case ACTION_MERGE_LAYER_DOWN:
            mergeCurrentLayerDown();
            return true;

        case ACTION_FOOTER_LAYER:
            // This event is not fired anymore
            // This controller is called directly
            return true;

        case ACTION_GOTO_NEXT_LAYER: {
            PageRef p = getCurrentPage();
            auto layer = p->getSelectedLayerId();
            if (layer < p->getLayerCount()) {
                switchToLay(layer + 1, true);
            }
        }
            return true;

        case ACTION_GOTO_PREVIOUS_LAYER: {
            PageRef p = getCurrentPage();
            auto layer = p->getSelectedLayerId();
            if (layer > 0) {
                switchToLay(layer - 1, true);
            }
        }
            return true;

        case ACTION_GOTO_TOP_LAYER: {
            PageRef p = getCurrentPage();
            switchToLay(p->getLayerCount(), true);
        }
            return true;
        case ACTION_RENAME_LAYER: {
            RenameLayerDialog dialog(control->getGladeSearchPath(), control->getUndoRedoHandler(), this,
                                     getCurrentPage()->getSelectedLayer());
            dialog.show(control->getGtkWindow());
        }
            return true;
        default:
            return false;
    }
}

/**
 * Show all layer on the current page
 */
void LayerController::showAllLayer() { hideOrHideAllLayer(true); }

/**
 * Hide all layer on the current page
 */
void LayerController::hideAllLayer() { hideOrHideAllLayer(false); }

/**
 * Show / Hide all layer on the current page
 */
void LayerController::hideOrHideAllLayer(bool show) {
    PageRef page = getCurrentPage();
    for (Layer::Index i = 1; i <= page->getLayerCount(); i++) { page->setLayerVisible(i, show); }

    fireLayerVisibilityChanged();
    control->getWindow()->getXournal()->layerChanged(selectedPage);
}

void LayerController::addNewLayer() {
    control->clearSelectionEndText();
    PageRef p = getCurrentPage();
    if (!p) {
        return;
    }

    auto* l = new Layer();
    auto layerPos = p->getSelectedLayerId();
    p->insertLayer(l, layerPos);

    control->getUndoRedoHandler()->addUndoAction(std::make_unique<InsertLayerUndoAction>(this, p, l, layerPos));

    fireRebuildLayerMenu();
    // Repaint is not needed here - the new layer is empty
}

void LayerController::deleteCurrentLayer() {
    control->clearSelectionEndText();

    PageRef p = getCurrentPage();
    auto pId = selectedPage;
    if (!p) {
        return;
    }

    auto lId = p->getSelectedLayerId();
    if (lId == 0) {
        return;
    }
    Layer* l = p->getSelectedLayer();

    p->removeLayer(l);

    MainWindow* win = control->getWindow();
    if (win) {
        win->getXournal()->layerChanged(pId);
    }

    control->getUndoRedoHandler()->addUndoAction(std::make_unique<RemoveLayerUndoAction>(this, p, l, lId - 1));

    fireRebuildLayerMenu();
}

void LayerController::moveCurrentLayer(bool up) {
    control->clearSelectionEndText();

    PageRef p = getCurrentPage();
    auto pId = selectedPage;
    if (!p) {
        return;
    }

    auto lId = p->getSelectedLayerId();
    Layer* currentLayer = p->getSelectedLayer();
    if (lId == 0) {
        // Background cannot be moved
        return;
    }

    if (lId == 1 && !up) {
        // bottom layer cannot be moved down
        return;
    }

    if (lId == p->getLayerCount() && up) {
        // top layer cannot be moved up
        return;
    }

    p->removeLayer(currentLayer);

    // Layer IDs are a bit strange, because background is 0
    // so the first layer is 1, technical the first layer is still
    // index 0 in the vector... confusing...
    auto newIndex = up ? lId : lId - 2;
    p->insertLayer(currentLayer, newIndex);

    MainWindow* win = control->getWindow();
    if (win) {
        win->getXournal()->layerChanged(pId);
    }

    control->getUndoRedoHandler()->addUndoAction(
            std::make_unique<MoveLayerUndoAction>(this, p, currentLayer, lId - 1, newIndex));

    fireRebuildLayerMenu();
}

void LayerController::mergeCurrentLayerDown() {
    control->clearSelectionEndText();

    PageRef page = getCurrentPage();
    auto pageID = selectedPage;
    if (page == nullptr) {
        return;
    }

    /*
     * layerID value:
     *    ...
     *    2: layer 2
     *    1: layer 1
     *    0: background
     */
    const auto layerID = page->getSelectedLayerId();
    Layer* currentLayer = page->getSelectedLayer();
    if (layerID < 2) {
        /*
         * lowest (non-background) layer cannot be merged into background
         * and the background itself obviously also cannot be merged down
         */
        return;
    }

    /*
     * We know this cannot be the background (or even an underflow) because
     * we checked for !(layerID < 2) before.
     */
    const auto layerBelowID = layerID - 1;
    /*
     * Layer indices in the vector are off by one from the layer IDs because
     * the background is not in the vector, so layer 1 has index 0 and so on.
     */
    const Layer::Index layerBelowIndex = layerBelowID - 1;
    Layer* layerBelow = (*page->getLayers())[layerBelowIndex];

    UndoActionPtr undo_redo_action =
            std::make_unique<MergeLayerDownUndoAction>(this, page, currentLayer, layerBelow, layerID - 1, pageID);
    undo_redo_action->redo(this->control);

    control->getUndoRedoHandler()->addUndoAction(std::move(undo_redo_action));
}

void LayerController::copyCurrentLayer() {
    control->clearSelectionEndText();

    PageRef p = getCurrentPage();
    auto pId = selectedPage;
    if (!p) {
        return;
    }

    auto lId = p->getSelectedLayerId();
    if (lId == 0) {
        return;
    }
    Layer* l = p->getSelectedLayer();
    Layer* cloned = l->clone();

    p->insertLayer(cloned, lId);

    MainWindow* win = control->getWindow();
    if (win) {
        win->getXournal()->layerChanged(pId);
    }

    control->getUndoRedoHandler()->addUndoAction(std::make_unique<InsertLayerUndoAction>(this, p, cloned, lId));

    fireRebuildLayerMenu();
}

auto LayerController::getCurrentPage() const -> PageRef { return control->getDocument()->getPage(selectedPage); }

auto LayerController::getCurrentPageId() const -> size_t { return selectedPage; }

void LayerController::setLayerVisible(Layer::Index layerId, bool visible) {
    getCurrentPage()->setLayerVisible(layerId, visible);
    fireLayerVisibilityChanged();

    control->getWindow()->getXournal()->layerChanged(selectedPage);
}

/**
 * Switch to a layer
 *
 * @param hideShow	Auto hide / show other layers,
 * 					as it was before the advance layer menu
 * @param clearSelection Clear / keep current selection before switching layers.
 */
void LayerController::switchToLay(Layer::Index layerId, bool hideShow, bool clearSelection) {
    if (clearSelection) {
        control->clearSelectionEndText();
    }

    PageRef p = getCurrentPage();
    if (!p) {
        return;
    }

    p->setSelectedLayerId(layerId);

    if (hideShow) {
        for (Layer::Index i = 1; i <= p->getLayerCount(); i++) { p->setLayerVisible(i, i <= layerId); }
    }

    // Repaint page
    control->getWindow()->getXournal()->layerChanged(selectedPage);
    fireLayerVisibilityChanged();
}

/**
 * @return Layer count of the current page
 */
auto LayerController::getLayerCount() const -> Layer::Index {
    PageRef page = getCurrentPage();
    if (!page) {
        return 0;
    }

    return page->getLayerCount();
}

/**
 * @return Current layer ID
 */
auto LayerController::getCurrentLayerId() const -> Layer::Index {
    PageRef page = getCurrentPage();
    if (!page) {
        return 0;
    }
    return page->getSelectedLayerId();
}

auto LayerController::getCurrentLayerName() const -> std::string {
    PageRef page = getCurrentPage();

    if (page == nullptr) {
        return "Unknown layer name";
    }

    auto currentID = getCurrentLayerId();

    if (currentID == 0) {  // If is background
        return page->getBackgroundName();
    } else if (auto layer = page->getSelectedLayer(); layer->hasName()) {
        return layer->getName();
    } else {
        return FS(_F("Layer {1}") % static_cast<long>(currentID));
    }
}

void LayerController::setCurrentLayerName(const std::string& newName) {
    PageRef page = getCurrentPage();

    if (page == nullptr) {
        return;
    }

    if (getCurrentLayerId() == 0) {  // Background
        page->setBackgroundName(newName);
    } else {  // Any other layer
        page->getSelectedLayer()->setName(newName);
    }

    fireRebuildLayerMenu();
}

std::string LayerController::getLayerNameById(Layer::Index id) const {
    PageRef page = getCurrentPage();

    if (page == nullptr) {
        return "Unknown layer name";
    }

    if (id == 0) {
        return page->getBackgroundName();
    }

    auto previousId = page->getSelectedLayerId();
    if (previousId == id) {
        return getCurrentLayerName();
    }
    page->setSelectedLayerId(id);
    std::string name = getCurrentLayerName();
    page->setSelectedLayerId(previousId);

    return name;
}

/**
 * Make sure there is at least one layer on the page
 */
void LayerController::ensureLayerExists(PageRef page) {
    if (page->getSelectedLayerId() > 0) {
        return;
    }

    // This creates a layer if none exists
    page->getSelectedLayer();
    page->setSelectedLayerId(1);

    fireRebuildLayerMenu();
}
