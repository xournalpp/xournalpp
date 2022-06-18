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

#include <cstddef>  // for size_t
#include <memory>   // for shared_ptr
#include <string>   // for string

#include "control/layer/LayerCtrlListener.h"               // for LayerCtrlL...
#include "gui/IconNameHelper.h"                            // for IconNameHe...
#include "gui/sidebar/previews/base/SidebarPreviewBase.h"  // for SidebarPre...
#include "gui/sidebar/previews/base/SidebarToolbar.h"      // for SidebarAct...
#include "model/Layer.h"                                   // for Layer, Lay...

class Control;
class GladeGui;
class LayerController;
class SidebarLayersContextMenu;


class SidebarPreviewLayers: public SidebarPreviewBase, public LayerCtrlListener {
public:
    SidebarPreviewLayers(Control* control, GladeGui* gui, SidebarToolbar* toolbar, bool stacked,
                         std::shared_ptr<SidebarLayersContextMenu> contextMenu);

    ~SidebarPreviewLayers() override;

public:
    void rebuildLayerMenu() override;
    void layerVisibilityChanged() override;

public:
    /**
     * Called when an action is performed
     */
    void actionPerformed(SidebarActions action) override;

    void enableSidebar() override;

    /**
     * @overwrite
     */
    std::string getName() override;

    /**
     * @overwrite
     */
    std::string getIconName() override;

    /**
     * Update the preview images
     * @overwrite
     */
    void updatePreviews() override;

    /**
     * Select a layer
     */
    void layerSelected(Layer::Index layerIndex);

    /**
     * A layer was hidden / showed
     */
    void layerVisibilityChanged(Layer::Index layerIndex, bool enabled);

    /**
     * Opens the layer preview context menu, at the current cursor position, for
     * the given layer.
     */
    void openPreviewContextMenu() override;

protected:
    void updateSelectedLayer();

public:
    // DocumentListener interface (only the part which is not handled by SidebarPreviewBase)
    void pageSizeChanged(size_t page) override;
    void pageChanged(size_t page) override;

private:
    /**
     * @return things that can reasonably be done to a given layer (e.g. merge down, copy, delete, etc.)
     */
    [[nodiscard]] static auto getViableActions(Layer::Index layerIndex, Layer::Index layerCount) -> SidebarActions;

    /**
     * Layer Controller
     */
    LayerController* lc;

    /**
     * render as stacked
     */
    bool stacked;

    IconNameHelper iconNameHelper;

    std::shared_ptr<SidebarLayersContextMenu> contextMenu;
};
