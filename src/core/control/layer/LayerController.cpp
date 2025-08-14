#include "LayerController.h"

#include <memory>   // for __shared_ptr_access, make...
#include <utility>  // for move
#include <vector>   // for vector

#include "control/Control.h"                 // for Control
#include "control/actions/ActionDatabase.h"  // for ActionDatabase
#include "gui/MainWindow.h"                  // for MainWindow
#include "gui/XournalView.h"                 // for XournalView
#include "model/Document.h"                  // for Document
#include "model/XojPage.h"                   // for XojPage
#include "undo/InsertLayerUndoAction.h"      // for InsertLayerUndoAction
#include "undo/MergeLayerDownUndoAction.h"   // for MergeLayerDownUndoAction
#include "undo/MoveLayerUndoAction.h"        // for MoveLayerUndoAction
#include "undo/RemoveLayerUndoAction.h"      // for RemoveLayerUndoAction
#include "undo/UndoAction.h"                 // for UndoActionPtr, UndoAction
#include "undo/UndoRedoHandler.h"            // for UndoRedoHandler
#include "util/Util.h"                       // for npos
#include "util/i18n.h"                       // for FS, _F

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

void LayerController::updateActions() {
    auto layer = getCurrentLayerId();
    auto maxLayer = getLayerCount();

    auto* actionDB = control->getActionDatabase();

    PageRef currentPage = getCurrentPage();
    if (currentPage != nullptr) {
        bool can_delete;
        bool can_merge_down;

        Layer* currentLayer = currentPage->getSelectedLayer();
        if (currentLayer != nullptr) {
            if (selectedPage == currentLayer->getFirstPage() or selectedPage == currentLayer->getLastPage()) {
                can_delete = layer > 0;
            } else {
                can_delete = false;
            }

            if (currentLayer->getFirstPage() != currentLayer->getLastPage()) {
                can_merge_down = false;
            } else {
                can_merge_down = true;
            }
        } else {
            can_delete = layer > 0;
            can_merge_down = layer > 1;
        }

        actionDB->enableAction(Action::LAYER_DELETE, can_delete);
        actionDB->enableAction(Action::LAYER_MERGE_DOWN, can_merge_down);
    }

    actionDB->enableAction(Action::LAYER_MOVE_UP, layer < maxLayer);
    actionDB->enableAction(Action::LAYER_MOVE_DOWN, layer > 1);
    actionDB->enableAction(Action::MOVE_SELECTION_LAYER_UP, layer < maxLayer);
    actionDB->enableAction(Action::MOVE_SELECTION_LAYER_DOWN, layer > 1);
    actionDB->enableAction(Action::LAYER_GOTO_NEXT, layer < maxLayer);
    actionDB->enableAction(Action::LAYER_GOTO_PREVIOUS, layer > 0);
    actionDB->enableAction(Action::LAYER_GOTO_TOP, layer < maxLayer);

    actionDB->enableAction(Action::LAYER_SPAN_UP, selectedPage > 0);
    actionDB->enableAction(Action::LAYER_SPAN_DOWN, selectedPage + 1 < control->getDocument()->getPageCount());

    actionDB->setActionState(Action::LAYER_ACTIVE, layer);
}

void LayerController::fireRebuildLayerMenu() {
    for (LayerCtrlListener* l: this->listener) {
        l->rebuildLayerMenu();
    }
    updateActions();
}

void LayerController::fireLayerVisibilityChanged() {
    for (LayerCtrlListener* l: this->listener) {
        l->layerVisibilityChanged();
    }

    // Rerenders all pages this layer is present in - Todo: make this another listener
    Layer* l = getCurrentPage()->getSelectedLayer();
    for (std::size_t i = l->getFirstPage(); i <= l->getLastPage(); ++i) {
        control->getWindow()->getXournal()->layerChanged(i);
    }
}

void LayerController::fireSelectedLayerChanged() {
    for (LayerCtrlListener* l: this->listener) {
        l->updateSelectedLayer();
    }
    updateActions();
}

/**
 * Show all layer on the current page
 */
void LayerController::showAllLayer() { showOrHideAllLayer(true); }

/**
 * Hide all layer on the current page
 */
void LayerController::hideAllLayer() { showOrHideAllLayer(false); }

/**
 * Show / Hide all layer on the current page
 */
void LayerController::showOrHideAllLayer(bool show) {
    PageRef page = getCurrentPage();
    for (Layer::Index i = 1; i <= page->getLayerCount(); i++) {
        page->setLayerVisible(i, show);
    }

    fireLayerVisibilityChanged();
}

void LayerController::addNewLayer(bool belowCurrentLayer) {
    control->clearSelectionEndText();
    PageRef p = getCurrentPage();
    if (!p) {
        return;
    }

    auto* l = new Layer();
    xoj_assert(p->getSelectedLayerId() > 0);

    l->setFirstPage(selectedPage);
    l->setLastPage(selectedPage);

    auto layerPos = belowCurrentLayer ? p->getSelectedLayerId() - 1 : p->getSelectedLayerId();
    p->insertLayer(l, layerPos);

    control->getUndoRedoHandler()->addUndoAction(std::make_unique<InsertLayerUndoAction>(this, p, l, layerPos));

    fireRebuildLayerMenu();
    // Repaint is not needed here - the new layer is empty
}

std::string makeLayerName(std::size_t i, std::size_t j) {
    return "Span " + std::to_string(i) + "-" + std::to_string(j);
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

    if (l != nullptr) {
        if (l->getFirstPage() != l->getLastPage()) {
            l->decrementPageCount();
            if (selectedPage == l->getFirstPage()) {
                l->setFirstPage(l->getFirstPage() + 1);
            } else {
                l->setLastPage(l->getLastPage() - 1);
            }
            l->setName(makeLayerName(l->getFirstPage() + 1, l->getLastPage() + 1));
        }
    }

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

void LayerController::spanTo(std::size_t pageIndex) {
    Layer* currentLayer = getCurrentPage()->getSelectedLayer();

    std::size_t i, j;
    if (currentLayer->getName() == "") {
        i = (selectedPage < pageIndex ? selectedPage : pageIndex);
        j = (selectedPage < pageIndex ? pageIndex : selectedPage);
    } else {
        i = currentLayer->getFirstPage();
        j = currentLayer->getLastPage();

        if (i <= pageIndex and pageIndex <= j) {
            // this is a mistake: the user is trying to span a layer into a page
            // that already contains this layer
            return;
        }

        if (selectedPage < pageIndex) {
            ++j;
        } else {
            --i;
        }
    }

    currentLayer->incrementPageCount();
    currentLayer->setFirstPage(i);
    currentLayer->setLastPage(j);
    currentLayer->setName(makeLayerName(i + 1, j + 1));

    MainWindow* win = control->getWindow();
    if (win) {
        win->getXournal()->layerChanged(selectedPage < pageIndex ? j : i);
    }

    PageRef span_page = control->getDocument()->getPage(pageIndex);

    const Layer::Index idx = span_page->getLayerCount();
    span_page->addLayer(currentLayer);

    control->getUndoRedoHandler()->addUndoAction(
            std::make_unique<InsertLayerUndoAction>(this, span_page, currentLayer, idx));

    fireRebuildLayerMenu();
}

void LayerController::spanUp() { spanTo(selectedPage - 1); }

void LayerController::spanDown() { spanTo(selectedPage + 1); }

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
    Layer* layerBelow = page->getLayers()[layerBelowIndex];

    UndoActionPtr undo_redo_action =
            std::make_unique<MergeLayerDownUndoAction>(this, page, currentLayer, layerID - 1, layerBelow, pageID);
    undo_redo_action->redo(this->control);

    control->getUndoRedoHandler()->addUndoAction(std::move(undo_redo_action));

    fireRebuildLayerMenu();
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
    if (cloned->getName()[0] == 'S') {
        cloned->setName("");
        cloned->setFirstPage(selectedPage);
        cloned->setLastPage(selectedPage);
    }

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
    fireSelectedLayerChanged();

    if (hideShow) {
        for (Layer::Index i = 1; i <= p->getLayerCount(); i++) {
            p->setLayerVisible(i, i <= layerId);
        }

        fireLayerVisibilityChanged();
    }
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
