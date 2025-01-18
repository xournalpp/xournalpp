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
#include "model/Layer.h"                                   // for Layer, Lay...

class Control;
class GladeGui;
class LayerController;
class SidebarLayersContextMenu;


class SidebarPreviewLayers: public SidebarPreviewBase, public LayerCtrlListener {
public:
    SidebarPreviewLayers(Control* control, bool stacked);

    ~SidebarPreviewLayers() override;

public:
    void rebuildLayerMenu() override;
    void layerVisibilityChanged() override;
    void updateSelectedLayer() override;

public:
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

public:
    // DocumentListener interface (only the part which is not handled by SidebarPreviewBase)
    void pageSizeChanged(size_t page) override;
    void pageChanged(size_t page) override;

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
};
