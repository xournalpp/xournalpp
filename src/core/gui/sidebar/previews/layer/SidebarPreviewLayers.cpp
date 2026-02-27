#include "SidebarPreviewLayers.h"

#include <algorithm>  // for max
#include <vector>     // for vector

#include <gtk/gtk.h>  // for gtk...

#include "control/Control.h"                                    // for Con...
#include "control/layer/LayerController.h"                      // for Lay...
#include "gui/Builder.h"                                        // for Builder
#include "gui/sidebar/previews/base/SidebarPreviewBaseEntry.h"  // for Sid...
#include "model/PageRef.h"                                      // for Pag...
#include "model/XojPage.h"                                      // for Xoj...
#include "util/Util.h"                                          // for npos
#include "util/i18n.h"                                          // for _

#include "SidebarPreviewLayerEntry.h"  // for Sid...

constexpr auto MENU_ID = "PreviewLayersContextMenu";
constexpr auto TOOLBAR_ID = "PreviewLayersToolbar";

SidebarPreviewLayers::SidebarPreviewLayers(Control* control, bool stacked):
        SidebarPreviewBase(control, MENU_ID, TOOLBAR_ID),
        lc(control->getLayerController()),
        stacked(stacked),
        iconNameHelper(control->getSettings()) {
    LayerCtrlListener::registerListener(lc);
}

SidebarPreviewLayers::~SidebarPreviewLayers() = default;

void SidebarPreviewLayers::enableSidebar() {
    SidebarPreviewBase::enableSidebar();
    rebuildLayerMenu();
}

auto SidebarPreviewLayers::getName() -> std::string { return stacked ? _("Layerstack Preview") : _("Layer Preview"); }

auto SidebarPreviewLayers::getIconName() -> std::string {
    const char* icon = stacked ? "sidebar-layerstack" : "sidebar-layer";
    return this->iconNameHelper.iconName(icon);
}

void SidebarPreviewLayers::pageSizeChanged(size_t page) {
    if (page != this->lc->getCurrentPageId() || !enabled) {
        return;
    }

    updatePreviews();
}

void SidebarPreviewLayers::pageChanged(size_t page) {
    if (page != this->lc->getCurrentPageId() || !enabled) {
        return;
    }

    // Repaint all layer
    for (auto& p: this->previews) { p->repaint(); }
}

void SidebarPreviewLayers::updatePreviews() {
    if (!enabled) {
        return;
    }

    // clear old previews
    this->previews.clear();
    this->selectedEntry = npos;

    PageRef page = lc->getCurrentPage();
    if (!page) {
        return;
    }

    auto layerCount = page->getLayerCount();

    for (auto i = layerCount + 1; i != 0;) {
        --i;
        std::string name = lc->getLayerNameById(i);
        auto p = std::make_unique<SidebarPreviewLayerEntry>(this, page, i, name, this->stacked);
        gtk_fixed_put(this->miniaturesContainer.get(), p->getWidget(), 0, 0);
        this->previews.emplace_back(std::move(p));
    }

    layout();
    updateSelectedLayer();
    layerVisibilityChanged();
    ensureVisibleAreRendered();
}

void SidebarPreviewLayers::rebuildLayerMenu() {
    if (!enabled) {
        return;
    }

    updatePreviews();
}

void SidebarPreviewLayers::layerVisibilityChanged() {
    PageRef p = lc->getCurrentPage();
    if (!p) {
        return;
    }

    Layer::Index i = p->getLayerCount();
    for (auto& e: this->previews) {
        dynamic_cast<SidebarPreviewLayerEntry*>(e.get())->setVisibleCheckbox(p->isLayerVisible(i--));
    }
    updateSelectedLayer();
}

void SidebarPreviewLayers::updateSelectedLayer() {
    // Layers are in reverse order (top index: 0, but bottom preview is 0)
    size_t entryIndex = this->previews.size() - lc->getCurrentLayerId() - 1;

    if (this->selectedEntry == entryIndex) {
        return;
    }

    if (this->selectedEntry != npos && this->selectedEntry < this->previews.size()) {
        this->previews[this->selectedEntry]->setSelected(false);
    }

    this->selectedEntry = entryIndex;
    if (this->selectedEntry != npos && this->selectedEntry < this->previews.size()) {
        auto& p = this->previews[this->selectedEntry];
        p->setSelected(true);
        scrollToPreview(this);
    }
}

void SidebarPreviewLayers::layerSelected(Layer::Index layerId) {
    if (layerId != 0) {  // We can't select the background
        lc->switchToLay(layerId);
        updateSelectedLayer();
    }
}

/**
 * A layer was hidden / showed
 */
void SidebarPreviewLayers::layerVisibilityChanged(Layer::Index layerIndex, bool enabled) {
    lc->setLayerVisible(layerIndex, enabled);
}
