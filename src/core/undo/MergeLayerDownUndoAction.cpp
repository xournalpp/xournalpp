#include "MergeLayerDownUndoAction.h"

#include <memory>  // for __shared_ptr_access
#include <vector>  // for vector

#include "control/Control.h"                // for Control
#include "control/layer/LayerController.h"  // for LayerController
#include "gui/MainWindow.h"                 // for MainWindow
#include "gui/XournalView.h"                // for XournalView
#include "model/Layer.h"                    // for Layer, Layer::Index
#include "model/PageRef.h"                  // for PageRef
#include "model/XojPage.h"                  // for XojPage
#include "undo/UndoAction.h"                // for UndoAction
#include "util/i18n.h"                      // for _

class Element;

MergeLayerDownUndoAction::MergeLayerDownUndoAction(LayerController* layerController, const PageRef& page,
                                                   Layer* upperLayer, Layer* lowerLayer, Layer::Index upperLayerPos,
                                                   size_t selectedPage):
        UndoAction("MergeLayerDownUndoAction"),
        upperLayerPos(upperLayerPos),
        layerController(layerController),
        upperLayer(upperLayer),
        lowerLayer(lowerLayer),
        upperLayerID(upperLayerPos + 1),
        lowerLayerID(upperLayerPos),
        selectedPage(selectedPage) {
    this->page = page;
}

auto MergeLayerDownUndoAction::getText() -> std::string { return _("Merge layer down"); }

auto MergeLayerDownUndoAction::undo(Control* control) -> bool {
    // remove all elements present in the upper layer from the lower layer again
    const bool free_elems = false;  // don't free the elems, they're still used
    for (Element* elem: this->upperLayer->getElements()) { this->lowerLayer->removeElement(elem, free_elems); }
    // add the upper layer back at its old pos
    layerController->insertLayer(this->page, this->upperLayer, upperLayerPos);
    // set the selected layer back to the ID of the upper layer
    this->page->setSelectedLayerId(this->upperLayerID);

    this->undone = true;

    this->triggerUIUpdate(control);

    return true;
}

auto MergeLayerDownUndoAction::redo(Control* control) -> bool {
    // remove the upper layer
    layerController->removeLayer(this->page, this->upperLayer);
    // add all elements back to the lower layer
    for (Element* elem: this->upperLayer->getElements()) { this->lowerLayer->addElement(elem); }
    // set the selected layer back to the ID of the lower layer
    this->page->setSelectedLayerId(this->lowerLayerID);

    this->undone = false;

    this->triggerUIUpdate(control);

    return true;
}

void MergeLayerDownUndoAction::triggerUIUpdate(Control* control) {
    /*
     * NOTE: (call order relevant to avoid deadlock)
     *     When this implementation is called by the `UndoRedoHandler` the
     *     document is locked. Calling `layerChanged` adds a render job which
     *     can only be processed when the document is unlocked again, but might
     *     have already claimed `Scheduler::jobRunningMutex`.
     *     `fireRebuildLayerMenu` will wait for `jobRunningMutex` to be free,
     *     so calling `fireRebuildLayerMenu` AFTER `layerChanged` will likely
     *     result in a DEADLOCK.
     */

    /*
     * Rebuild the layer menu, so that the correct new layer selection is
     * properly displayed in the sidebar. Without this, the visually displayed
     * selection defaults to the top layer while the actual selection is the
     * newly merged layer; there would be a mismatch without rebuilding the
     * menu this way.
     */
    layerController->fireRebuildLayerMenu();

    /*
     * Under some circumstance (e.g. the layer below is not set to visible)
     * merging might change the appearance of the page, so `layerChanged` is
     * employed to rerender the page.
     */
    MainWindow* win = control->getWindow();
    if (win) {
        win->getXournal()->layerChanged(this->selectedPage);
    }
}
