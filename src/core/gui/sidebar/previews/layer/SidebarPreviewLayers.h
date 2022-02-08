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

#include <string>
#include <vector>

#include "control/layer/LayerCtrlListener.h"
#include "gui/IconNameHelper.h"
#include "gui/sidebar/previews/base/SidebarPreviewBase.h"


class SidebarPreviewLayers: public SidebarPreviewBase, public LayerCtrlListener {
public:
    SidebarPreviewLayers(Control* control, GladeGui* gui, SidebarToolbar* toolbar, bool stacked);
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
    void layerSelected(size_t layerIndex);

    /**
     * A layer was hidden / showed
     */
    void layerVisibilityChanged(int layerIndex, bool enabled);

protected:
    void updateSelectedLayer();

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
