/*
 * Xournal++
 *
 * A Sidebar preview widget
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>  // for size_t
#include <string>   // for string

#include <gtk/gtk.h>  // for GtkWi...

#include "gui/sidebar/previews/base/SidebarPreviewBaseEntry.h"  // for Previ...
#include "model/Layer.h"                                        // for Layer
#include "model/PageRef.h"                                      // for PageRef

class SidebarPreviewLayers;

class SidebarPreviewLayerEntry: public SidebarPreviewBaseEntry {
public:
    SidebarPreviewLayerEntry(SidebarPreviewLayers* sidebar, const PageRef& page, Layer::Index layerId,
                             const std::string& layerName, size_t index, bool stacked);
    ~SidebarPreviewLayerEntry() override;

public:
    int getHeight() override;

    /**
     * @return What should be rendered
     * @override
     */
    PreviewRenderType getRenderType() override;

    /**
     * @return The layer to be rendered
     */
    Layer::Index getLayer() const;

    GtkWidget* getWidget() override;

    /**
     * Set the value of the visible checkbox
     */
    void setVisibleCheckbox(bool enabled);

protected:
    void mouseButtonPressCallback() override;
    void checkboxToggled();

    SidebarPreviewLayers* sidebar;

private:
    /**
     * Layer preview index
     */
    size_t index;

    /**
     * Layer to render
     */
    Layer::Index layerId;

    /**
     * Toolbar with controls
     */
    int toolbarHeight = 0;

    /**
     * Container box for the preview and the button
     */
    GtkWidget* box;

    /**
     * Visible checkbox
     */
    GtkWidget* cbVisible = nullptr;

    /**
     * Ignore events
     */
    bool inUpdate = false;

    /**
     * render as stacked
     */
    bool stacked = false;

    friend class PreviewJob;
};
