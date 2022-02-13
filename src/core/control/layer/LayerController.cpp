#include "LayerController.h"

#include <memory>

#include "control/Control.h"
#include "gui/XournalView.h"
#include "undo/InsertLayerUndoAction.h"
#include "undo/MergeLayerDownUndoAction.h"
#include "undo/MoveLayerUndoAction.h"
#include "undo/RemoveLayerUndoAction.h"
#include "util/Util.h"
#include "util/i18n.h"

#include "LayerCtrlListener.h"

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

void LayerController::insertLayer(PageRef page, Layer* layer, int layerPos) {
    page->insertLayer(layer, layerPos);
    fireRebuildLayerMenu();
}

void LayerController::removeLayer(PageRef page, Layer* layer) {
    page->removeLayer(layer);
    fireRebuildLayerMenu();
}

void LayerController::addLayer(PageRef page, Layer* layer) {
    page->addLayer(layer);
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
            int layer = p->getSelectedLayerId();
            if (layer < static_cast<int>(p->getLayerCount())) {
                switchToLay(layer + 1, true);
            }
        }
            return true;

        case ACTION_GOTO_PREVIOUS_LAYER: {
            PageRef p = getCurrentPage();
            int layer = p->getSelectedLayerId();
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
    for (size_t i = 1; i <= page->getLayerCount(); i++) { page->setLayerVisible(i, show); }

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
    int layerPos = p->getSelectedLayerId();
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

    int lId = p->getSelectedLayerId();
    if (lId < 1) {
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

    int lId = p->getSelectedLayerId();
    Layer* currentLayer = p->getSelectedLayer();
    if (lId < 1) {
        // Background cannot be moved
        return;
    }

    if (lId < 2 && !up) {
        // bottom layer cannot be moved down
        return;
    }

    if (lId == static_cast<int>(p->getLayerCount()) && up) {
        // top layer cannot be moved up
        return;
    }

    p->removeLayer(currentLayer);

    // Layer IDs are a bit strange, because background is 0
    // so the first layer is 1, technical the first layer is still
    // index 0 in the vector... confusing...
    int newIndex = up ? lId : lId - 2;
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
    const size_t layerBelowIndex = ((size_t)layerBelowID) - 1;
    Layer* layerBelow = page->getLayers()->at(layerBelowIndex);

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

    int lId = p->getSelectedLayerId();
    if (lId < 1) {
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

auto LayerController::getCurrentPage() -> PageRef { return control->getDocument()->getPage(selectedPage); }

auto LayerController::getCurrentPageId() const -> size_t { return selectedPage; }

void LayerController::setLayerVisible(int layerId, bool visible) {
    getCurrentPage()->setLayerVisible(layerId, visible);
    fireLayerVisibilityChanged();

    control->getWindow()->getXournal()->layerChanged(selectedPage);
}

/**
 * Switch to a layer
 *
 * @param hideShow	Auto hide / show other layers,
 * 					as it was before the advance layer menu
 */
void LayerController::switchToLay(int layer, bool hideShow) {
    control->clearSelectionEndText();

    PageRef p = getCurrentPage();
    if (!p) {
        return;
    }

    p->setSelectedLayerId(layer);

    if (hideShow) {
        for (size_t i = 1; i <= p->getLayerCount(); i++) { p->setLayerVisible(i, static_cast<int>(i) <= layer); }
    }

    // Repaint page
    control->getWindow()->getXournal()->layerChanged(selectedPage);
    fireLayerVisibilityChanged();
}

/**
 * @return Layer count of the current page
 */
auto LayerController::getLayerCount() -> size_t {
    PageRef page = getCurrentPage();
    if (!page) {
        return 0;
    }

    return page->getLayerCount();
}

/**
 * @return Current layer ID
 */
auto LayerController::getCurrentLayerId() -> size_t {
    PageRef page = getCurrentPage();
    if (!page) {
        return 0;
    }
    return page->getSelectedLayerId();
}

auto LayerController::getCurrentLayerName() -> std::string {
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
        return FS(_F("Layer {1}") % currentID);
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

std::string LayerController::getLayerNameById(int id) {
    PageRef page = getCurrentPage();

    if (page == nullptr) {
        return "Unknown layer name";
    }

    if (id == 0) {
        return page->getBackgroundName();
    }

    int previousId = page->getSelectedLayerId();
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
