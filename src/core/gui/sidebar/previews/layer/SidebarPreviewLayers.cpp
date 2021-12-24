#include "SidebarPreviewLayers.h"

#include "control/Control.h"
#include "control/PdfCache.h"
#include "control/layer/LayerController.h"
#include "util/i18n.h"

#include "SidebarPreviewLayerEntry.h"

SidebarPreviewLayers::SidebarPreviewLayers(Control* control, GladeGui* gui, SidebarToolbar* toolbar, bool stacked):
        SidebarPreviewBase(control, gui, toolbar),
        lc(control->getLayerController()),
        stacked(stacked),
        iconNameHelper(control->getSettings()) {
    LayerCtrlListener::registerListener(lc);

    this->toolbar->setButtonEnabled(SIDEBAR_ACTION_NONE);
}

SidebarPreviewLayers::~SidebarPreviewLayers() {
    // clear old previews
    for (SidebarPreviewBaseEntry* p: this->previews) { delete p; }
    this->previews.clear();
}

/**
 * Called when an action is performed
 */
void SidebarPreviewLayers::actionPerformed(SidebarActions action) {
    switch (action) {
        case SIDEBAR_ACTION_MOVE_UP: {
            control->getLayerController()->moveCurrentLayer(true);
            break;
        }
        case SIDEBAR_ACTION_MOVE_DOWN: {
            control->getLayerController()->moveCurrentLayer(false);
            break;
        }
        case SIDEBAR_ACTION_COPY: {
            control->getLayerController()->copyCurrentLayer();
            break;
        }
        case SIDEBAR_ACTION_DELETE:
            control->getLayerController()->deleteCurrentLayer();
            break;
        default:
            break;
    }
}

void SidebarPreviewLayers::enableSidebar() {
    SidebarPreviewBase::enableSidebar();

    this->toolbar->setButtonTooltips(_("Swap the current layer with the one above"),
                                     _("Swap the current layer with the one below"),
                                     _("Insert a copy of the current layer below"), _("Delete this layer"));
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
    for (SidebarPreviewBaseEntry* p: this->previews) { p->repaint(); }
}

void SidebarPreviewLayers::updatePreviews() {
    if (!enabled) {
        return;
    }

    // clear old previews
    for (SidebarPreviewBaseEntry* p: this->previews) { delete p; }
    this->previews.clear();
    this->selectedEntry = npos;

    PageRef page = lc->getCurrentPage();
    if (!page) {
        return;
    }

    int layerCount = page->getLayerCount();

    size_t index = 0;
    for (int i = layerCount; i >= 0; i--) {
        std::string name = lc->getLayerNameById(i);
        SidebarPreviewBaseEntry* p = new SidebarPreviewLayerEntry(this, page, i - 1, name, index++, this->stacked);
        this->previews.push_back(p);
        gtk_layout_put(GTK_LAYOUT(this->iconViewPreview), p->getWidget(), 0, 0);
    }

    layout();
    updateSelectedLayer();
    layerVisibilityChanged();
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

    for (int i = 0; i < static_cast<int>(this->previews.size()); i++) {
        auto* sp = dynamic_cast<SidebarPreviewLayerEntry*>(this->previews[this->previews.size() - i - 1]);
        sp->setVisibleCheckbox(p->isLayerVisible(i));
    }
}

void SidebarPreviewLayers::updateSelectedLayer() {
    // Layers are in reverse order (top index: 0, but bottom preview is 0)
    size_t layerIndex = this->previews.size() - lc->getCurrentLayerId() - 1;

    if (this->selectedEntry == layerIndex) {
        return;
    }

    if (this->selectedEntry != npos && this->selectedEntry < this->previews.size()) {
        this->previews[this->selectedEntry]->setSelected(false);
    }

    this->selectedEntry = layerIndex;
    if (this->selectedEntry != npos && this->selectedEntry < this->previews.size()) {
        SidebarPreviewBaseEntry* p = this->previews[this->selectedEntry];
        p->setSelected(true);
        scrollToPreview(this);
    }

    int actions = 0;
    // Background and top layer cannot be moved up
    if (this->selectedEntry < (this->previews.size() - 1) && this->selectedEntry > 0) {
        actions |= SIDEBAR_ACTION_MOVE_UP;
    }

    // Background and first layer cannot be moved down
    if (this->selectedEntry < (this->previews.size() - 2)) {
        actions |= SIDEBAR_ACTION_MOVE_DOWN;
    }

    // Background cannot be copied
    if (this->selectedEntry < (this->previews.size() - 1)) {
        actions |= SIDEBAR_ACTION_COPY;
    }

    // Background cannot be deleted
    if (this->selectedEntry < (this->previews.size() - 1)) {
        actions |= SIDEBAR_ACTION_DELETE;
    }

    this->toolbar->setHidden(false);
    this->toolbar->setButtonEnabled(static_cast<SidebarActions>(actions));
}

void SidebarPreviewLayers::layerSelected(size_t layerIndex) {
    // Layers are in reverse order (top index: 0, but bottom preview is 0)
    lc->switchToLay(this->previews.size() - layerIndex - 1);
    updateSelectedLayer();
}

/**
 * A layer was hidden / showed
 */
void SidebarPreviewLayers::layerVisibilityChanged(int layerIndex, bool enabled) {
    lc->setLayerVisible(layerIndex, enabled);
}
