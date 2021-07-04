/*
 * Xournal++
 *
 * Previews of the layers in the current page
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "control/layer/LayerCtrlListener.h"
#include "gui/sidebar/previews/base/SidebarPreviewBase.h"
#include "util/IconNameHelper.h"

#include "XournalType.h"

class SidebarPreviewLayers: public SidebarPreviewBase, public LayerCtrlListener {
public:
    SidebarPreviewLayers(Control* control, GladeGui* gui, SidebarToolbar* toolbar, bool stacked);
    virtual ~SidebarPreviewLayers();

public:
    virtual void rebuildLayerMenu();
    virtual void layerVisibilityChanged();

public:
    /**
     * Called when an action is performed
     */
    void actionPerformed(SidebarActions action);

    void enableSidebar();

    /**
     * @overwrite
     */
    virtual string getName();

    /**
     * @overwrite
     */
    virtual string getIconName();

    /**
     * Update the preview images
     * @overwrite
     */
    virtual void updatePreviews();

    /**
     * Select a layer
     */
    void layerSelected(size_t layerIndex);

    /**
     * A layer was hidden / showed
     */
    void layerVisibilityChanged(int layerIndex, bool enabled);

    /**
     * Opens the layer preview context menu, at the current cursor position, for
     * the given layer.
     */
    void openPreviewContextMenu();

protected:
    void updateSelectedLayer();

public:
    // DocumentListener interface (only the part which is not handled by SidebarPreviewBase)
    virtual void pageSizeChanged(size_t page);
    virtual void pageChanged(size_t page);

private:
    /**
     * Layer Controller
     */
    LayerController* lc;

    /**
     * render as stacked
     */
    bool stacked;

    IconNameHelper iconNameHelper;


    /**
     * The context menu to display when a layer is right-clicked.
     */
    GtkWidget* const contextMenu = nullptr;

    /**
     * The data passed to the menu item callbacks.
     */
    struct ContextMenuData {
        SidebarToolbar* toolbar;
        SidebarActions actions;
    };

    /**
     * The signals connected to the context menu items. This must be kept track
     * of so the data can be deallocated safely.
     */
    std::vector<std::tuple<GtkWidget*, gulong, std::unique_ptr<ContextMenuData>>> contextMenuSignals;
};
