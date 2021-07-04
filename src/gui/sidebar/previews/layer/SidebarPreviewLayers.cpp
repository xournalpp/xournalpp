#include "SidebarPreviewLayers.h"

#include "control/Control.h"
#include "control/PdfCache.h"
#include "control/layer/LayerController.h"

#include "SidebarPreviewLayerEntry.h"
#include "i18n.h"

// Since we have two layer sidebars, we need to make sure we don't
// double-register callbacks. Since we also need to make sure we only do the
// proper deallocation stuff on destuction of the last SidebarPreviewLayers
// object, we use a static counter which we count up in construction and down
// in destruction.
static int layerSidebarsActive = 0;

SidebarPreviewLayers::SidebarPreviewLayers(Control* control, GladeGui* gui, SidebarToolbar* toolbar, bool stacked):
        SidebarPreviewBase(control, gui, toolbar),
        contextMenu(gui->get("sidebarPreviewLayersContextMenu")),
        lc(control->getLayerController()),
        stacked(stacked),
        iconNameHelper(control->getSettings()) {
    LayerCtrlListener::registerListener(lc);

    this->toolbar->setButtonEnabled(SIDEBAR_ACTION_NONE);

    // Connect the context menu actions
    const std::map<std::string, SidebarActions> ctxMenuActions = {
            {"sidebarPreviewMergeDown", SIDEBAR_ACTION_MERGE_DOWN},
            {"sidebarPreviewLayerDuplicate", SIDEBAR_ACTION_COPY},
    };

    if (layerSidebarsActive == 0) {

        for (const auto& pair: ctxMenuActions) {
            GtkWidget* const entry = gui->get(pair.first);
            g_assert(entry != nullptr);

            // Unfortunately, we need a fairly complicated mechanism to keep track
            // of which action we want to execute.
            using Data = SidebarPreviewLayers::ContextMenuData;
            auto userdata = std::make_unique<Data>(Data{this->toolbar, pair.second});

            const auto callback =
                    G_CALLBACK(+[](GtkMenuItem* item, Data* data) { data->toolbar->runAction(data->actions); });
            const gulong signalId = g_signal_connect(entry, "activate", callback, userdata.get());
            g_object_ref(entry);
            this->contextMenuSignals.emplace_back(entry, signalId, std::move(userdata));

            if (pair.first == "sidebarPreviewMoveDown") {
                // TODO: correct these assert(0)s when we introduce moving to menu
                // I'm only creating a menu with the merging item right now,
                // because there are a lot of complications to the joint
                // functionalities of the multiple sidebars. Thus I'm ignoring the
                // move up and down actions which have some special treatment in
                // the class SidebarPreviewPages from which this is copied.
                g_assert(false);
            } else if (pair.first == "sidebarPreviewMoveUp") {
                g_assert(false);
            }
        }
    }
    layerSidebarsActive++;
}

SidebarPreviewLayers::~SidebarPreviewLayers() {
    // clear old previews
    for (SidebarPreviewBaseEntry* p: this->previews) {
        delete p;
    }
    this->previews.clear();

    if (layerSidebarsActive == 1) {
        for (const auto& signalTuple: this->contextMenuSignals) {
            GtkWidget* const widget = std::get<0>(signalTuple);
            const guint handlerId = std::get<1>(signalTuple);
            if (g_signal_handler_is_connected(widget, handlerId)) {
                g_signal_handler_disconnect(widget, handlerId);
            }
            g_object_unref(widget);
        }
    }
    layerSidebarsActive--;
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
        case SIDEBAR_ACTION_MERGE_DOWN: {
            control->getLayerController()->mergeCurrentLayerDown();
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
            // TODO: probably should warn in this case?
            break;
    }
}

void SidebarPreviewLayers::enableSidebar() {
    SidebarPreviewBase::enableSidebar();

    this->toolbar->setButtonTooltips(_("Swap the current layer with the one above"),
                                     _("Swap the current layer with the one below"),
                                     _("Merge the current layer with the one below"),
                                     _("Insert a copy of the current layer below"), _("Delete this layer"));
    rebuildLayerMenu();
}

auto SidebarPreviewLayers::getName() -> string { return stacked ? _("Layerstack Preview") : _("Layer Preview"); }

auto SidebarPreviewLayers::getIconName() -> string {
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
    for (SidebarPreviewBaseEntry* p: this->previews) {
        p->repaint();
    }
}

void SidebarPreviewLayers::updatePreviews() {
    if (!enabled) {
        return;
    }

    // clear old previews
    for (SidebarPreviewBaseEntry* p: this->previews) {
        delete p;
    }
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

    // Background and first layer cannot be merged down
    if (this->selectedEntry < (this->previews.size() - 2)) {
        actions |= SIDEBAR_ACTION_MERGE_DOWN;
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

void SidebarPreviewLayers::openPreviewContextMenu() {
    gtk_menu_popup(GTK_MENU(this->contextMenu), nullptr, nullptr, nullptr, nullptr, 3, gtk_get_current_event_time());
}
